/*********************************************************************************************************************
* LS2K0300 USB摄像头采集与网络传输Demo
*
* 硬件连接说明：
* - IPS200屏幕通过SPI接口连接（设备树配置）- 可选
* - USB摄像头插入USB接口（设备路径 /dev/video0）
* - Framebuffer设备路径：/dev/fb0
*
* 功能说明：
* 1. 初始化USB摄像头（OpenCV V4L2）
* 2. 初始化网络流服务器（TCP端口8888）
* 3. 可选：初始化IPS200屏幕显示
* 4. 循环采集摄像头图像并传输到电脑，可选显示到屏幕
*
* 编译说明：
* ./build_simple.sh
*
* 运行说明：
* # 仅网络传输（性能最佳，推荐）
* LD_LIBRARY_PATH=/home/root/opencv/lib ./camera_display_ips200
*
* # 同时启用屏幕显示
* LD_LIBRARY_PATH=/home/root/opencv/lib ./camera_display_ips200 --enable-display
*********************************************************************************************************************/

#include "uvc_camera.h"
#include "ips200_display.h"
#include "network_stream.h"
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <string.h>

// 全局标志，用于安全退出
static volatile bool running = true;

// 配置选项：是否启用IPS200屏幕显示
static bool enable_display = false;
static bool display_initialized = false;

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

    // 关闭屏幕（如果已初始化）
    if (display_initialized) {
        ips200_display_close();
    }
}

/**
 * @brief 显示使用帮助
 */
void show_usage(const char* program_name) {
    std::cout << "用法: " << program_name << " [选项]" << std::endl;
    std::cout << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  --enable-display     启用IPS200屏幕显示（默认：禁用）" << std::endl;
    std::cout << "  --disable-display    禁用IPS200屏幕显示（默认）" << std::endl;
    std::cout << "  -h, --help           显示此帮助信息" << std::endl;
    std::cout << std::endl;
    std::cout << "示例:" << std::endl;
    std::cout << "  " << program_name << "                    # 仅网络传输（推荐，性能最佳）" << std::endl;
    std::cout << "  " << program_name << " --enable-display  # 同时显示到IPS200屏幕" << std::endl;
    std::cout << std::endl;
    std::cout << "说明:" << std::endl;
    std::cout << "  禁用屏幕显示可以节省约30%的CPU资源，提高网络传输帧率。" << std::endl;
    std::cout << "  推荐使用电脑端查看图像，获得更好的显示效果和性能。" << std::endl;
}

int main(int argc, char** argv) {
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--enable-display") == 0) {
            enable_display = true;
        } else if (strcmp(argv[i], "--disable-display") == 0) {
            enable_display = false;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            show_usage(argv[0]);
            return 0;
        } else {
            std::cerr << "错误：未知选项 '" << argv[i] << "'" << std::endl;
            std::cerr << "使用 --help 查看帮助信息" << std::endl;
            return 1;
        }
    }

    std::cout << "========================================" << std::endl;
    std::cout << "  USB摄像头高帧率图像传输系统" << std::endl;
    std::cout << "  基于逐飞LS2K0300开源库优化" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "配置：" << std::endl;
    std::cout << "  - 网络传输: 启用" << std::endl;
    std::cout << "  - IPS200显示: " << (enable_display ? "启用" : "禁用（节省资源）") << std::endl;
    std::cout << "========================================" << std::endl;

    // 注册清理函数
    atexit(cleanup);

    // 注册信号处理函数
    signal(SIGINT, sigint_handler);

    int step = 1;

    // 1. 初始化 IPS200 屏幕（可选）
    if (enable_display) {
        std::cout << "\n[" << step++ << "/3] 正在初始化 IPS200 屏幕..." << std::endl;
        if (ips200_display_init("/dev/fb0") < 0) {
            std::cerr << "警告：IPS200屏幕初始化失败！" << std::endl;
            std::cerr << "将继续运行，但屏幕显示功能不可用。" << std::endl;
            std::cerr << "如需屏幕显示，请检查：" << std::endl;
            std::cerr << "  1. 设备树是否正确配置（st7789v节点）" << std::endl;
            std::cerr << "  2. /dev/fb0 设备是否存在" << std::endl;
            std::cerr << "  3. 屏幕硬件连接是否正常" << std::endl;
            enable_display = false;  // 禁用显示功能
        } else {
            // 清屏并显示提示信息
            ips200_clear();
            display_initialized = true;
            std::cout << "IPS200屏幕初始化成功！" << std::endl;
        }
    } else {
        std::cout << "\n提示：IPS200显示已禁用，节省CPU资源以提高网络传输性能。" << std::endl;
        std::cout << "      如需启用屏幕显示，请使用参数：--enable-display" << std::endl;
    }

    // 2. 初始化 UVC 摄像头
    std::cout << "\n[" << step++ << "/3] 正在初始化 USB 摄像头..." << std::endl;
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
    std::cout << "\n[" << step++ << "/3] 正在初始化网络流服务器..." << std::endl;
    if (network_stream_init(NETWORK_PORT) < 0) {
        std::cerr << "错误：网络流服务器初始化失败！" << std::endl;
        std::cerr << "请检查：" << std::endl;
        std::cerr << "  1. 端口 " << NETWORK_PORT << " 是否被占用" << std::endl;
        std::cerr << "  2. 网络配置是否正确" << std::endl;
        return -1;
    }
    std::cout << "网络流服务器启动成功，端口: " << NETWORK_PORT << std::endl;
    std::cout << "等待电脑客户端连接..." << std::endl;

    // 4. 主循环：采集图像并传输/显示
    std::cout << "\n开始采集并传输图像..." << std::endl;
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

        // 显示图像到 IPS200 屏幕（如果启用）
        if (enable_display && display_initialized) {
            // 图像居中显示：(240-160)/2=40, (320-120)/2=100
            ips200_show_gray_image(40, 100, gray_image, UVC_WIDTH, UVC_HEIGHT);
        }

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
                         << frame_count;
                if (enable_display) {
                    std::cout << " (含屏幕显示)";
                } else {
                    std::cout << " (仅网络传输)";
                }
                std::cout << std::endl;
            }
        }
    }

    std::cout << "\n程序正常退出，总共处理 " << frame_count << " 帧图像" << std::endl;
    return 0;
}
