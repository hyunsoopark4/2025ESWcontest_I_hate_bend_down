import cv2
from ultralytics import YOLO
import os
import time
import requests
import threading
import math

# --- 설정 ---
WEBHOOK_URL = "https://discord.com/api/webhooks/1407691826638487572/vq_BWv6GeRGTlbWNFD1HOanbL40kI4-RIZo48RYTVqAbkRFvThfLjTHb3SpkyFEBOIhO"
YOLO_MODEL_PATH = 'runs/detect/train5/weights/best.pt'
CONF_THRESHOLD = 0.75

# --- 새로운 시간 및 상태 관리 설정 ---
DETECTION_INTERVAL = 5  # 평소 탐지 간격 (5초)
CONFIRMATION_TIME = 15  # 신발 인식 후 알림까지 대기 시간 (15초)
POST_NOTIFICATION_COOLDOWN = 60  # 알림 전송 후 탐지 비활성화 시간 (1분)
POSITION_THRESHOLD = 50  # 같은 위치로 판단할 픽셀 거리 (50픽셀)


# --- 설정 끝 ---

def send_discord_notification(filename, conf):
    """(백그라운드 작업) 이미지 파일을 디스코드 웹훅으로 전송합니다."""
    print(f"🚀 (백그라운드) 디스코드로 이미지 전송 시도...")
    try:
        with open(filename, "rb") as f:
            payload = {"content": f"신발이 탐지되었습니다! (신뢰도: {conf * 100:.1f}%)"}
            files = {'file': (filename, f, 'image/jpeg')}
            response = requests.post(WEBHOOK_URL, data=payload, files=files)

        if 200 <= response.status_code < 300:
            print("✅ (백그라운드) 디스코드 전송 성공!")
        else:
            print(f"❌ (백그라운드) 디스코드 전송 실패: {response.status_code}")
    except Exception as e:
        print(f"❌ (백그라운드) 전송 중 오류 발생: {e}")


# --- 메인 코드 ---
model = YOLO(YOLO_MODEL_PATH)
cap = cv2.VideoCapture(1)

# --- 상태 변수 초기화 ---
last_loop_time = 0
first_seen_time = None
last_seen_position = None
last_notification_time = -POST_NOTIFICATION_COOLDOWN  # 프로그램 시작 시 바로 탐지 가능하도록 설정

print("스마트 감지 시스템 실행 중... (종료: 'q')")

while True:
    success, frame = cap.read()
    if not success: break

    current_time = time.time()

    # --- 5. 1분 휴식 모드 ---
    if current_time - last_notification_time < POST_NOTIFICATION_COOLDOWN:
        cv2.putText(frame,
                    f"Cooldown: {int(POST_NOTIFICATION_COOLDOWN - (current_time - last_notification_time))}s left",
                    (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
        cv2.imshow("Smart Shoe Detection", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'): break
        continue

    # --- 1. 5초마다 탐지 ---
    if current_time - last_loop_time < DETECTION_INTERVAL:
        # 5초가 안 지났으면 화면만 보여주고 다음 프레임으로
        if first_seen_time:  # 타이머가 동작 중일 때는 남은 시간 표시
            remaining = CONFIRMATION_TIME - (current_time - first_seen_time)
            cv2.putText(frame, f"Confirming shoe... {int(remaining)}s", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1,
                        (0, 255, 255), 2)
        else:
            cv2.putText(frame, "Status: Idle", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        cv2.imshow("Smart Shoe Detection", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'): break
        continue

    last_loop_time = current_time  # 현재 탐지 시간 기록
    original_frame = frame.copy()
    results = model.predict(source=frame, verbose=False)

    detected_shoe_box = None
    # 가장 신뢰도 높은 신발 하나만 찾기
    max_conf = 0
    for r in results:
        for box in r.boxes:
            conf = box.conf[0]
            if conf > CONF_THRESHOLD and conf > max_conf:
                detected_shoe_box = box
                max_conf = conf

    # --- 2 & 3. 인지 및 확인 모드 ---
    if detected_shoe_box:
        x1, y1, x2, y2 = map(int, detected_shoe_box.xyxy[0])
        current_position = (int((x1 + x2) / 2), int(y2))  # 하단 중앙점

        if first_seen_time is None:
            # 신발 첫 발견
            first_seen_time = current_time
            last_seen_position = current_position
            print(f"[{time.strftime('%H:%M:%S')}] 신발 첫 발견! 15초 카운트다운 시작.")
        else:
            # 이전에 발견한 신발과 같은 위치인지 확인
            distance = math.sqrt(
                (current_position[0] - last_seen_position[0]) ** 2 + (current_position[1] - last_seen_position[1]) ** 2)
            if distance < POSITION_THRESHOLD:
                # 같은 위치에 계속 있음
                if current_time - first_seen_time >= CONFIRMATION_TIME:
                    # --- 4. 알림 모드 ---
                    print(f"[{time.strftime('%H:%M:%S')}] 15초 확인 완료. 디스코드 알림 전송!")
                    cropped_shoe = original_frame[y1:y2, x1:x2]
                    filename = "confirmed_shoe.jpg"
                    cv2.imwrite(filename, cropped_shoe)

                    notification_thread = threading.Thread(target=send_discord_notification, args=(filename, max_conf))
                    notification_thread.start()

                    last_notification_time = current_time  # 알림 보낸 시간 기록
                    first_seen_time = None  # 상태 초기화
                    last_seen_position = None
            else:
                # 다른 위치에서 새 신발 발견, 타이머 리셋
                print(f"[{time.strftime('%H:%M:%S')}] 다른 위치에서 신발 발견. 타이머 리셋.")
                first_seen_time = current_time
                last_seen_position = current_position
    else:
        # 신발이 사라짐
        if first_seen_time is not None:
            print(f"[{time.strftime('%H:%M:%S')}] 신발 사라짐. 카운트다운 취소.")
            first_seen_time = None  # 상태 초기화
            last_seen_position = None

    # ... (imshow 및 종료키 부분은 루프 상단으로 이동) ...

cap.release()
cv2.destroyAllWindows()