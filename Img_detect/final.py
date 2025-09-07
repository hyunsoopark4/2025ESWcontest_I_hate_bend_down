import cv2
import numpy as np
from ultralytics import YOLO
import math
import time
import serial
import serial.tools.list_ports

# --- 설정 (사용자 지정 값으로 고정) ---
WEBHOOK_URL = "https://discord.com/api/webhooks/1407691826638487572/vq_BWv6GeRGTlbWNFD1HOanbL40kI4-RIZo48RYTVqAbkRFvThfLjTHb3SpkyFEBOIhO"
YOLO_MODEL_PATH = 'runs/detect/train5/weights/best.pt'
CONF_THRESHOLD = 0.75

# --- 블루투스 및 그리드 설정 ---
# ❗️ Windows 제어판에서 확인한 HC-06의 가상 COM 포트 번호를 입력하세요
BT_SERIAL_PORT = "COM4"
BAUD_RATE = 9600
GRID_DIMS = (4, 4)  #(꼭짓점 4개)

# --- 시간 설정 ---
SEND_COOLDOWN = 5  # 다음 전송까지의 최소 시간 (초)


# --- Helper 함수들 ---

def get_grid_points_in_pixels(grid_dims, homography_matrix):
    """실제 그리드 좌표를 이미지의 픽셀 좌표로 변환합니다."""
    grid_w, grid_h = grid_dims
    real_points = []
    for i in range(grid_h):
        for j in range(grid_w):
            real_points.append([j, i])

    real_points = np.float32(real_points).reshape(-1, 1, 2)

    try:
        inv_matrix = np.linalg.inv(homography_matrix)
        pixel_points = cv2.perspectiveTransform(real_points, inv_matrix)
        return pixel_points, real_points.reshape(-1, 2)
    except np.linalg.LinAlgError:
        print("❌ 보정 행렬이 잘못되었습니다. live_calibration.py를 다시 실행하세요.")
        return None, None


def find_nearest_vertex(point, vertices_pixels, vertices_real):
    """주어진 점에서 가장 가까운 꼭짓점을 찾습니다."""
    distances = np.linalg.norm(vertices_pixels - point, axis=1)
    nearest_idx = np.argmin(distances)
    return vertices_pixels[nearest_idx], vertices_real[nearest_idx]


def get_angle_direction(p1, p2):
    """두 점 사이의 각도를 계산하여 8방위로 반환합니다."""
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


# --- 메인 프로그램 시작 ---

def main():
    # --- 초기화 ---
    # 블루투스 시리얼 포트 연결
    ser = None
    try:
        ser = serial.Serial(BT_SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"✅ HC-06 모듈에 '{BT_SERIAL_PORT}'로 연결되었습니다.")
    except serial.SerialException as e:
        print(f"❌ 시리얼 포트 연결 실패: {e}. COM 포트 설정을 확인하세요.")

    # YOLO 모델 로드
    model = YOLO(YOLO_MODEL_PATH)

    # 변환 행렬 로드
    try:
        matrix = np.load('homography_matrix.npy')
    except FileNotFoundError:
        print("❌ 'homography_matrix.npy' 파일이 없습니다. 먼저 'live_calibration.py'를 실행하세요.")
        return

    # 그리드 꼭짓점 픽셀 좌표 계산
    all_vertices_pixels, all_vertices_real = get_grid_points_in_pixels(GRID_DIMS, matrix)
    if all_vertices_pixels is None:
        return
    all_vertices_pixels = all_vertices_pixels.reshape(-1, 2)

    # 웹캠 열기
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("❌ 오류: 웹캠을 열 수 없습니다.")
        return

    last_send_time = 0

    # --- 메인 루프 ---
    while True:
        success, frame = cap.read()
        if not success:
            break

        results = model.predict(source=frame, verbose=False)

        current_time = time.time()

        # 쿨다운 시간이 지났을 때만 탐지 및 전송 로직 실행
        if (current_time - last_send_time) > SEND_COOLDOWN:
            best_box = None
            max_conf = 0
            for r in results:
                for box in r.boxes:
                    conf = box.conf[0]
                    if conf > max_conf:
                        max_conf = conf
                        best_box = box

            if best_box and max_conf > CONF_THRESHOLD:
                x1, y1, x2, y2 = map(int, best_box.xyxy[0])

                # 신발 뒷부분 좌표 계산
                shoe_rear_point = np.array([int((x1 + x2) / 2), y2])

                # 가장 가까운 꼭짓점 및 방향 계산
                nearest_vtx_pixel, nearest_vtx_real = find_nearest_vertex(shoe_rear_point, all_vertices_pixels,
                                                                          all_vertices_real)
                direction = get_angle_direction(nearest_vtx_pixel, shoe_rear_point)
                vtx_x = int(nearest_vtx_real[0])
                vtx_y = int(nearest_vtx_real[1])

                # 블루투스로 데이터 전송
                if ser:
                    command = f"<{vtx_x},{vtx_y},{direction}>"
                    ser.write(command.encode('utf-8'))
                    print(f"🚀 HC-06으로 제어 신호 전송: {command}")
                    last_send_time = current_time  # 전송 시간 기록

                # 시각화
                cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
                cv2.circle(frame, tuple(shoe_rear_point), 5, (0, 0, 255), -1)
                cv2.circle(frame, tuple(map(int, nearest_vtx_pixel)), 7, (255, 0, 0), -1)
                cv2.line(frame, tuple(map(int, nearest_vtx_pixel)), tuple(shoe_rear_point), (255, 255, 0), 2)
                label = f"Vtx:({vtx_x},{vtx_y}) Dir:{direction}"
                cv2.putText(frame, label, (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)

        # 모든 그리드 꼭짓점 시각화
        for p in all_vertices_pixels:
            cv2.circle(frame, tuple(map(int, p)), 3, (0, 255, 255), -1)

        cv2.imshow("Windows BT Controller", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # --- 자원 정리 ---
    if ser:
        ser.close()
    cap.release()
    cv2.destroyAllWindows()
    print("프로그램을 종료합니다.")


if __name__ == "__main__":
    main()