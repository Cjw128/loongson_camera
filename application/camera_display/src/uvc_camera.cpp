#include "uvc_camera.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/utility.hpp>
#include <iostream>

using namespace cv;

// �ڲ�����
static VideoCapture cap;
static Mat frame_rgb;
static Mat frame_gray;
static uint8_t *gray_image_ptr = nullptr;

int uvc_camera_init(const char *device_path) {
    // ������ͷ�豸
    cap.open(device_path, CAP_V4L2);

    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open camera device: " << device_path << std::endl;
        return -1;
    }

    std::cout << "Camera opened successfully: " << device_path << std::endl;

    // ��������ͷ����
    cap.set(CAP_PROP_FRAME_WIDTH, UVC_WIDTH);
    cap.set(CAP_PROP_FRAME_HEIGHT, UVC_HEIGHT);
    cap.set(CAP_PROP_FPS, UVC_FPS);

    // ��֤����
    int actual_width = cap.get(CAP_PROP_FRAME_WIDTH);
    int actual_height = cap.get(CAP_PROP_FRAME_HEIGHT);
    int actual_fps = cap.get(CAP_PROP_FPS);

    std::cout << "Camera settings: " << actual_width << "x" << actual_height
              << " @ " << actual_fps << " FPS" << std::endl;

    return 0;
}

int wait_image_refresh() {
    try {
        // ������ͷ��ȡһ֡
        cap >> frame_rgb;

        if (frame_rgb.empty()) {
            std::cerr << "Error: Failed to capture frame" << std::endl;
            return -1;
        }

        // ת��Ϊ�Ҷ�ͼ
        cv::cvtColor(frame_rgb, frame_gray, cv::COLOR_BGR2GRAY);

        // ��ȡ����ָ��
        gray_image_ptr = frame_gray.ptr<uint8_t>(0);

        return 0;

    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV Exception: " << e.what() << std::endl;
        return -1;
    }
}

uint8_t* get_gray_image() {
    return gray_image_ptr;
}

void uvc_camera_close() {
    if (cap.isOpened()) {
        cap.release();
        std::cout << "Camera closed" << std::endl;
    }
}