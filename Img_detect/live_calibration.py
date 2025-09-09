import cv2
import numpy as np

# --- 설정 ---
# 실제 바닥에서 측정한 기준 사각형의 꼭짓점 4개 좌표 (cm 또는 그리드 단위)
# 예: 가로 4칸(꼭짓점 5개), 세로 5칸(꼭짓점 6개) 그리드의 네 모서리
REAL_WORLD_COORDS = np.float32([
    [0, 6],  # 왼쪽 위
    [5, 6],  # 오른쪽 위
    [5, 0],  # 오른쪽 아래
    [0, 0]  # 왼쪽 아래
])
# --- 설정 끝 ---

# 전역 변수로 클릭한 좌표 저장
clicked_points = []


def mouse_callback(event, x, y, flags, param):
    """마우스 클릭 이벤트를 처리하여 좌표를 저장하는 함수"""
    if event == cv2.EVENT_LBUTTONDOWN:
        if len(clicked_points) < 4:
            clicked_points.append((x, y))
            print(f"포인트 {len(clicked_points)} 추가: ({x}, {y})")


def main():
    """실시간 웹캠 피드를 보며 보정을 수행합니다."""
    global clicked_points

    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("오류: 웹캠을 열 수 없습니다.")
        return

    captured_frame = None

    print("--- 실시간 보정 모드 ---")
    print("1. 바닥의 기준 사각형이 잘 보이도록 카메라를 조정하세요.")
    print("2. 마음에 드는 화면에서 '스페이스바'를 눌러 화면을 캡처(정지)합니다.")
    print("3. 'q' 키를 누르면 종료됩니다.")

    # 1. 화면 캡처 단계
    while True:
        success, frame = cap.read()
        if not success:
            break

        cv2.imshow("Calibration - Press SPACE to capture", frame)
        key = cv2.waitKey(1) & 0xFF

        if key == ord('q'):
            cap.release()
            cv2.destroyAllWindows()
            return
        elif key == ord(' '):  # 스페이스바를 누르면
            captured_frame = frame
            print("\n화면 캡처 완료! 이제 캡처된 화면에서 4점을 클릭하세요.")
            break

    # 2. 4점 클릭 단계
    if captured_frame is not None:
        window_name = "Click 4 corners (CW from top-left), then press 'q'"
        cv2.namedWindow(window_name)
        cv2.setMouseCallback(window_name, mouse_callback)

        while True:
            temp_img = captured_frame.copy()
            for point in clicked_points:
                cv2.circle(temp_img, point, 7, (0, 0, 255), -1)
            cv2.imshow(window_name, temp_img)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    cap.release()
    cv2.destroyAllWindows()

    # 3. 행렬 계산 및 저장
    if len(clicked_points) == 4:
        src_points = np.float32(clicked_points)
        matrix = cv2.getPerspectiveTransform(src_points, REAL_WORLD_COORDS)
        np.save('homography_matrix.npy', matrix)
        print("\n✅ 변환 행렬이 'homography_matrix.npy' 파일로 성공적으로 저장되었습니다.")
    else:
        print("\n❌ 4개의 포인트가 선택되지 않아 행렬을 저장하지 못했습니다.")


if __name__ == "__main__":
    main()