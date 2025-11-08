#ifndef UVC_CAMERA_H
#define UVC_CAMERA_H

#include <stdint.h>

// 摄像头分辨率配置
#define UVC_WIDTH   160
#define UVC_HEIGHT  120
#define UVC_FPS     110

/**
 * @brief 初始化 UVC 摄像头
 * @param device_path 设备路径，通常为 "/dev/video0"
 * @return 0: 成功, -1: 失败
 */
int uvc_camera_init(const char *device_path);

/**
 * @brief 等待并获取新的图像帧
 * @return 0: 成功, -1: 失败
 */
int wait_image_refresh();

/**
 * @brief 获取灰度图像数据指针
 * @return 灰度图像数据指针
 */
uint8_t* get_gray_image();

/**
 * @brief 关闭摄像头
 */
void uvc_camera_close();

#endif // UVC_CAMERA_H