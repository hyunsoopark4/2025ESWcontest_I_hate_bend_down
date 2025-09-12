import cv2
import numpy as np
from ultralytics import YOLO
import os
import time
import requests
import threading
import math

# 설정
WEBHOOK_URL = "https://discord.com/api/webhooks/1407691826638487572/vq_BWv6GeRGTlbWNFD1HOanbL40kI4-RIZo48RYTVqAbkRFvThfLjTHb3SpkyFEBOIhO"
YOLO_MODEL_PATH = 'runs/detect/train6/weights/best.pt'
CONF_THRESHOLD = 0.8

# 그리드 설정
GRID_DIMS = (5, 6)

# 시간 및 상태 관리 설정
DETECTION_INTERVAL = 5
CONFIRMATION_TIME = 15
POST_NOTIFICATION_COOLDOWN = 60
POSITION_THRESHOLD = 50


def send_discord_notification(filename, conf, vtx_info):
    print(f"디스코드로 정보 전송 시도")
    try:
        with open(filename, "rb") as f:
            message = f"신발이 탐지되었습니다! (신뢰도: {conf * 100:.1f}%)\n{vtx_info}"
            payload = {"content": message}
            files = {'file': (filename, f, 'image/jpeg')}
            response = requests.post(WEBHOOK_URL, data=payload, files=files)
        if 200 <= response.status_code < 300:
            print("디스코드 전송 성공!")
    except Exception as e:
        print(f"전송 중 오류 발생: {e}")


def get_grid_points_in_pixels(grid_dims, homography_matrix):
    grid_w, grid_h = grid_dims
    real_points = []
    for i in range(grid_h):
        for j in range(grid_w): real_points.append([j, i])
    real_points = np.float32(real_points).reshape(-1, 1, 2)
    try:
        inv_matrix = np.linalg.inv(homography_matrix)
        pixel_points = cv2.perspectiveTransform(real_points, inv_matrix)
        return pixel_points, real_points.reshape(-1, 2)
    except np.linalg.LinAlgError:
        return None, None


def find_nearest_vertex(point, vertices_pixels, vertices_real):
    distances = np.linalg.norm(vertices_pixels - point, axis=1)
    nearest_idx = np.argmin(distances)
    return vertices_pixels[nearest_idx], vertices_real[nearest_idx]


def get_angle_direction(p1, p2):
    angle_rad = math.atan2(p1[1] - p2[1], p2[0] - p1[0])
    angle_deg = math.degrees(angle_rad)
    if -22.5 <= angle_deg < 22.5: return "E"
    if 22.5 <= angle_deg < 67.5: return "NE"
    if 67.5 <= angle_deg < 112.5: return "N"
    if 112.5 <= angle_deg < 157.5: return "NW"
    if 157.5 <= angle_deg or angle_deg < -157.5: return "W"
    if -157.5 <= angle_deg < -112.5: return "SW"
    if -112.5 <= angle_deg < -67.5: return "S"
    if -67.5 <= angle_deg < -22.5: return "SE"
    return ""

# 메인 코드
model = YOLO(YOLO_MODEL_PATH)
try:
    matrix = np.load('homography_matrix.npy')
except FileNotFoundError:
    print(" 'homography_matrix.npy' 파일이 없습니다. 'live_calibration.py'를 먼저 실행하세요.")
    exit()

all_vertices_pixels, all_vertices_real = get_grid_points_in_pixels(GRID_DIMS, matrix)
if all_vertices_pixels is None: exit()
all_vertices_pixels = all_vertices_pixels.reshape(-1, 2)

cap = cv2.VideoCapture(1)
last_check_time = 0
first_seen_time = None
last_seen_position = None
last_notification_time = -POST_NOTIFICATION_COOLDOWN

print("실시간 추적 및 그리드 감지 시스템 실행 중... (종료: 'q')")

while True:
    success, frame = cap.read()
    if not success: break

    current_time = time.time()
    original_frame = frame.copy()

    # 1. 1분 휴식 모드 체크
    if current_time - last_notification_time < POST_NOTIFICATION_COOLDOWN:
        status_text = f"Cooldown: {int(POST_NOTIFICATION_COOLDOWN - (current_time - last_notification_time))}s"
        cv2.putText(frame, status_text, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
        cv2.imshow("Real-time Grid Detector", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
        continue

    # 2. (매 프레임) YOLO 추론 및 최고 신뢰도 신발 찾기
    results = model.predict(source=frame, verbose=False)
    best_box_in_frame = None
    max_conf = 0
    for r in results:
        for box in r.boxes:
            conf = box.conf[0]
            if conf > CONF_THRESHOLD:
                if conf > max_conf:
                    max_conf = conf
                    best_box_in_frame = box

    # 3. (매 프레임) 최고 신뢰도 신발에 대한 실시간 시각화
    if best_box_in_frame:
        x1, y1, x2, y2 = map(int, best_box_in_frame.xyxy[0])
        shoe_rear_point = np.array([int((x1 + x2) / 2), int((y1 + y2) / 2)])
        nearest_vtx_pixel, nearest_vtx_real = find_nearest_vertex(shoe_rear_point, all_vertices_pixels,
                                                                  all_vertices_real)
        direction = get_angle_direction(nearest_vtx_pixel, shoe_rear_point)
        vtx_x, vtx_y = int(nearest_vtx_real[0]), int(nearest_vtx_real[1])
        vtx_info = f"Vtx:({vtx_x},{vtx_y}) Dir:{direction}"

        cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
        cv2.putText(frame, f"Shoe: {max_conf:.2f}", (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
        cv2.circle(frame, tuple(shoe_rear_point.astype(int)), 5, (0, 0, 255), -1)
        cv2.circle(frame, tuple(nearest_vtx_pixel.astype(int)), 7, (255, 0, 0), -1)
        cv2.line(frame, tuple(shoe_rear_point.astype(int)), tuple(nearest_vtx_pixel.astype(int)), (255, 255, 0), 2)
        cv2.putText(frame, vtx_info, (x1, y2 + 25), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
    else:
        vtx_info = ""

    # 4. (5초마다) 상태 관리 및 알림 로직 실행
    if current_time - last_check_time >= DETECTION_INTERVAL:
        last_check_time = current_time
        if best_box_in_frame:
            current_position = (int((best_box_in_frame.xyxy[0][0] + best_box_in_frame.xyxy[0][2]) / 2),
                                int(best_box_in_frame.xyxy[0][3]))
            if first_seen_time is None:
                first_seen_time = current_time
                last_seen_position = current_position
                print(f"[{time.strftime('%H:%M:%S')}] 신발 첫 발견! {CONFIRMATION_TIME}초 카운트다운 시작.")
            else:
                distance = math.sqrt((current_position[0] - last_seen_position[0]) ** 2 + (
                            current_position[1] - last_seen_position[1]) ** 2)
                if distance < POSITION_THRESHOLD:
                    if current_time - first_seen_time >= CONFIRMATION_TIME:
                        print(f"[{time.strftime('%H:%M:%S')}] {CONFIRMATION_TIME}초 확인 완료. 알림 전송!")
                        x1_b, y1_b, x2_b, y2_b = map(int, best_box_in_frame.xyxy[0])
                        cropped_shoe = original_frame[y1_b:y2_b, x1_b:x2_b]
                        filename = "confirmed_shoe.jpg"
                        cv2.imwrite(filename, cropped_shoe)

                        notification_thread = threading.Thread(target=send_discord_notification,
                                                               args=(filename, max_conf, vtx_info))
                        notification_thread.start()

                        last_notification_time = current_time
                        first_seen_time = None
                else:
                    print(f"[{time.strftime('%H:%M:%S')}] 다른 위치에서 신발 발견. 타이머 리셋.")
                    first_seen_time = current_time
                    last_seen_position = current_position
        else:
            if first_seen_time is not None:
                print(f"[{time.strftime('%H:%M:%S')}] 신발 사라짐. 카운트다운 취소.")
            first_seen_time = None

    # 5. (매 프레임) 상태 텍스트 및 그리드 시각화
    if first_seen_time:
        remaining = CONFIRMATION_TIME - (current_time - first_seen_time)
        status_text = f"Confirming... {int(remaining)}s"
        color = (0, 255, 255)
    else:
        status_text = "Status: Idle"
        color = (0, 255, 0)
    cv2.putText(frame, status_text, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, color, 2)

    for p in all_vertices_pixels:
        cv2.circle(frame, tuple(map(int, p)), 3, (0, 255, 255), -1)

    cv2.imshow("Real-time Grid Detector", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()

cv2.destroyAllWindows()

