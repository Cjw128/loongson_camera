#include "uvc_camera.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/utility.hpp>
#include <iostream>

using namespace cv;

// 内部变量
static VideoCapture cap;
static Mat frame_rgb;
static Mat frame_gray;
static uint8_t *gray_image_ptr = nullptr;

int uvc_camera_init(const char *device_path) {
    // 打开摄像头设备（参考逐飞LS2K0300开源库优化方案）
    cap.open(device_path, CAP_V4L2);

    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open camera device: " << device_path << std::endl;
        return -1;
    }

    std::cout << "Camera opened successfully: " << device_path << std::endl;

    // === 高帧率优化方案（参考逐飞LS2K0300开源库）===

    // 1. 设置 MJPEG 格式（支持高帧率的关键！）
    cap.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M', 'J', 'P', 'G'));
    std::cout << "✓ Set MJPEG format for high frame rate support" << std::endl;

    // 2. 设置分辨率
    cap.set(CAP_PROP_FRAME_WIDTH, UVC_WIDTH);
    cap.set(CAP_PROP_FRAME_HEIGHT, UVC_HEIGHT);

    // 3. 设置帧率（龙邱110fps摄像头）
    cap.set(CAP_PROP_FPS, UVC_FPS);

    // ⚠️ 移除以下设置（可能限制性能）：
    // - CAP_PROP_BUFFERSIZE：可能不被V4L2支持
    // - CAP_PROP_AUTO_EXPOSURE / CAP_PROP_EXPOSURE：增加处理开销

    // 验证配置
    int actual_width = cap.get(CAP_PROP_FRAME_WIDTH);
    int actual_height = cap.get(CAP_PROP_FRAME_HEIGHT);
    int actual_fps = cap.get(CAP_PROP_FPS);

    std::cout << "Camera settings: " << actual_width << "x" << actual_height
              << " @ " << actual_fps << " FPS" << std::endl;

    if (actual_fps < UVC_FPS) {
        std::cout << "⚠️  Warning: Requested " << UVC_FPS << " fps, actual "
                  << actual_fps << " fps" << std::endl;
        std::cout << "   提示: 请确认使用龙邱110fps摄像头并安装了正确的UVC驱动" << std::endl;
    } else {
        std::cout << "✅ 摄像头初始化成功: 已达到目标帧率 " << UVC_FPS << " fps" << std::endl;
    }

    return 0;
}

int wait_image_refresh() {
    // 读取一帧
    bool ret = cap.read(frame_rgb);
    if (!ret || frame_rgb.empty()) {
        std::cerr << "Error: Failed to read frame from camera" << std::endl;
        return -1;
    }

    // 转换为灰度图
    cvtColor(frame_rgb, frame_gray, COLOR_BGR2GRAY);

    // 更新指针
    gray_image_ptr = frame_gray.data;

    return 0;
}

uint8_t* get_gray_image() {
    return gray_image_ptr;
}

void uvc_camera_close() {
    if (cap.isOpened()) {
        cap.release();
        std::cout << "Camera closed" << std::endl;
    }
    gray_image_ptr = nullptr;
}
