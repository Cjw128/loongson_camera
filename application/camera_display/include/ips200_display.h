#ifndef IPS200_DISPLAY_H
#define IPS200_DISPLAY_H

#include <stdint.h>

/**
 * @brief 初始化 IPS200 屏幕显示（基于逐飞开源库）
 * @param fb_device Framebuffer 设备路径，通常为 "/dev/fb0"
 * @return 0: 成功, -1: 失败
 */
int ips200_display_init(const char *fb_device);

/**
 * @brief 清空屏幕
 */
void ips200_clear(void);

/**
 * @brief 屏幕填充指定颜色
 * @param color RGB565 颜色值
 */
void ips200_full(const uint16_t color);

/**
 * @brief 画点
 * @param x X 坐标 [0-239]
 * @param y Y 坐标 [0-319]
 * @param color RGB565 颜色值
 */
void ips200_draw_point(uint16_t x, uint16_t y, const uint16_t color);

/**
 * @brief 显示灰度图像到 IPS200 屏幕
 * @param x 起始 X 坐标
 * @param y 起始 Y 坐标
 * @param image 灰度图像数据指针（8位灰度值）
 * @param width 图像宽度
 * @param height 图像高度
 */
void ips200_show_gray_image(uint16_t x, uint16_t y, const uint8_t *image,
                             uint16_t width, uint16_t height);

/**
 * @brief 在屏幕上显示字符串
 * @param x X 坐标
 * @param y Y 坐标
 * @param str 字符串
 */
void ips200_show_string(uint16_t x, uint16_t y, const char *str);

/**
 * @brief 关闭 IPS200 显示
 */
void ips200_display_close(void);

// RGB565 颜色定义
#define RGB565_BLACK   0x0000
#define RGB565_WHITE   0xFFFF
#define RGB565_RED     0xF800
#define RGB565_GREEN   0x07E0
#define RGB565_BLUE    0x001F
#define RGB565_YELLOW  0xFFE0
#define RGB565_CYAN    0x07FF
#define RGB565_PURPLE  0xF81F

#endif // IPS200_DISPLAY_H
