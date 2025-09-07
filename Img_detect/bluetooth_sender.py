import cv2
from ultralytics import YOLO
import os
import time
import subprocess  # Windows 기능을 실행하기 위한 라이브러리

# --- 설정 ---
YOLO_MODEL_PATH = 'yolov8n.pt'
CONF_THRESHOLD = 0.6
# --- 설정 끝 ---

# YOLO 모델 로드
model = YOLO(YOLO_MODEL_PATH)

# 웹캠 열기
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("오류: 웹캠을 열 수 없습니다.")
    exit()

print("웹캠 실행 중... 신발을 탐지합니다. (종료하려면 'q' 키를 누르세요)")

while True:
    success, frame = cap.read()
    if not success:
        break

    original_frame = frame.copy()
    results = model.predict(source=frame, verbose=False)

    found_and_sent = False
    for r in results:
        for box in r.boxes:
            conf = box.conf[0]
            if conf > CONF_THRESHOLD:
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                print(f"✅ 신발 탐지! (신뢰도: {conf:.2f})")

                # 1. 탐지된 신발 이미지 자르기 및 저장
                cropped_shoe = original_frame[y1:y2, x1:x2]
                filename = "detected_shoe.jpg"
                cv2.imwrite(filename, cropped_shoe)
                print(f"📸 '{filename}'으로 이미지 저장 완료.")

                # 2. Windows 블루투스 파일 전송 기능 실행
                print("📱 Windows 블루투스 파일 전송 창을 엽니다...")
                try:
                    # fsquirt.exe를 이용해 파일 전송 창을 띄움
                    subprocess.run(['fsquirt.exe', '-send', filename])
                    print("✅ 전송 창을 열었습니다. 기기를 선택하고 전송을 완료하세요.")
                except FileNotFoundError:
                    print("❌ fsquirt.exe를 찾을 수 없습니다. Windows 시스템이 맞는지 확인하세요.")

                found_and_sent = True
                break
        if found_and_sent:
            break

    cv2.imshow("Laptop Shoe Detection", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

    if found_and_sent:
        print("\n전송이 완료되면 터미널을 닫고 프로그램을 다시 실행하세요.")
        break  # 팝업 창이 뜬 후에는 루프를 중단하여 사용자가 선택할 시간을 줌

# 자원 정리
cap.release()
cv2.destroyAllWindows()