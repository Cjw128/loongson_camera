/*********************************************************************************************************************
* LS2K0300 USB摄像头采集与IPS200屏幕显示Demo
*
* 硬件连接说明：
* - IPS200屏幕通过SPI接口连接（设备树配置）
* - USB摄像头插入USB接口（设备路径 /dev/video0）
* - Framebuffer设备路径：/dev/fb0
*
* 功能说明：
* 1. 初始化IPS200屏幕（基于逐飞开源库实现）
* 2. 初始化USB摄像头（OpenCV V4L2）
* 3. 初始化网络流服务器（TCP端口8888）
* 4. 循环采集摄像头图像并显示到IPS200屏幕，同时传输到电脑
*
* 编译说明：
* ./build_simple.sh
*
* 运行说明：
* LD_LIBRARY_PATH=/home/root/opencv/lib ./camera_display_ips200
*********************************************************************************************************************/

#include "uvc_camera.h"
#include "ips200_display.h"
#include "network_stream.h"
#include <iostream>
#include <signal.h>
#include <unistd.h>

// 全局标志，用于安全退出
static volatile bool running = true;

/**
 * @brief 信号处理函数（Ctrl+C）
 */
void sigint_handler(int signum) {
    std::cout << "\n收到退出信号，程序即将退出..." << std::endl;
    running = false;
}

/**
 * @brief 清理函数
 */
void cleanup() {
    std::cout << "执行清理操作..." << std::endl;

    // 关闭网络流服务器
    network_stream_close();

    // 关闭摄像头
    uvc_camera_close();

    // 关闭屏幕
    ips200_display_close();
}

int main(int argc, char** argv) {
    std::cout << "========================================" << std::endl;
    std::cout << "  USB摄像头 + IPS200屏幕 + 网络显示" << std::endl;
    std::cout << "  基于逐飞IPS200开源库实现" << std::endl;
    std::cout << "========================================" << std::endl;

    // 注册清理函数
    atexit(cleanup);

    // 注册信号处理函数
    signal(SIGINT, sigint_handler);

    // 1. 初始化 IPS200 屏幕
    std::cout << "\n[1/3] 正在初始化 IPS200 屏幕..." << std::endl;
    if (ips200_display_init("/dev/fb0") < 0) {
        std::cerr << "错误：IPS200屏幕初始化失败！" << std::endl;
        std::cerr << "请检查：" << std::endl;
        std::cerr << "  1. 设备树是否正确配置（st7789v节点）" << std::endl;
        std::cerr << "  2. /dev/fb0 设备是否存在" << std::endl;
        std::cerr << "  3. 屏幕硬件连接是否正常" << std::endl;
        return -1;
    }

    // 清屏并显示提示信息
    ips200_clear();
    std::cout << "IPS200屏幕初始化成功！" << std::endl;

    // 2. 初始化 UVC 摄像头
    std::cout << "\n[2/3] 正在初始化 USB 摄像头..." << std::endl;
    if (uvc_camera_init("/dev/video0") < 0) {
        std::cerr << "错误：USB摄像头初始化失败！" << std::endl;
        std::cerr << "请检查：" << std::endl;
        std::cerr << "  1. USB摄像头是否已插入" << std::endl;
        std::cerr << "  2. /dev/video0 设备是否存在" << std::endl;
        std::cerr << "  3. 摄像头驱动是否加载（lsmod | grep uvc）" << std::endl;
        return -1;
    }
    std::cout << "USB摄像头初始化成功！" << std::endl;

    // 3. 初始化网络流服务器
    std::cout << "\n[3/3] 正在初始化网络流服务器..." << std::endl;
    if (network_stream_init(NETWORK_PORT) < 0) {
        std::cerr << "错误：网络流服务器初始化失败！" << std::endl;
        std::cerr << "请检查：" << std::endl;
        std::cerr << "  1. 端口 " << NETWORK_PORT << " 是否被占用" << std::endl;
        std::cerr << "  2. 网络配置是否正确" << std::endl;
        return -1;
    }
    std::cout << "网络流服务器启动成功，端口: " << NETWORK_PORT << std::endl;
    std::cout << "等待电脑客户端连接..." << std::endl;

    // 4. 主循环：采集图像并显示
    std::cout << "\n开始采集并显示图像..." << std::endl;
    std::cout << "按 Ctrl+C 退出程序\n" << std::endl;

    int frame_count = 0;
    time_t start_time = time(NULL);

    while (running) {
        // 等待并采集新的图像帧
        if (wait_image_refresh() < 0) {
            std::cerr << "警告：图像采集失败！" << std::endl;

            // 如果连续失败，则退出
            static int fail_count = 0;
            if (++fail_count > 10) {
                std::cerr << "错误：摄像头采集连续失败，程序退出" << std::endl;
                break;
            }

            usleep(100000);  // 等待100ms后重试
            continue;
        }

        // 获取灰度图像数据
        uint8_t* gray_image = get_gray_image();
        if (gray_image == NULL) {
            std::cerr << "警告：无法获取灰度图像数据" << std::endl;
            continue;
        }

        // 显示图像到 IPS200 屏幕
        // 图像居中显示：(240-160)/2=40, (320-120)/2=100
        ips200_show_gray_image(40, 100, gray_image, UVC_WIDTH, UVC_HEIGHT);

        // 发送图像到网络客户端
        int clients = network_stream_send(gray_image, UVC_WIDTH, UVC_HEIGHT);
        if (clients > 0 && frame_count % 30 == 0) {
            // 每30帧提示一次客户端连接数
            std::cout << "网络客户端数: " << clients << std::endl;
        }

        // 统计帧率
        frame_count++;
        if (frame_count % 30 == 0) {
            time_t current_time = time(NULL);
            double elapsed = difftime(current_time, start_time);
            if (elapsed > 0) {
                double fps = frame_count / elapsed;
                std::cout << "实时帧率: " << fps << " FPS, 总帧数: "
                         << frame_count << std::endl;
            }
        }
    }

    std::cout << "\n程序正常退出，总共处理 " << frame_count << " 帧图像" << std::endl;
    return 0;
}
