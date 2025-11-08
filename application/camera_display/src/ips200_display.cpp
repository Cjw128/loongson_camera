#include "ips200_display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

// 屏幕参数
#define IPS200_WIDTH    240
#define IPS200_HEIGHT   320

// 默认颜色配置
#define IPS200_DEFAULT_PENCOLOR  RGB565_RED
#define IPS200_DEFAULT_BGCOLOR   RGB565_WHITE

// 内部变量
static int ips200_width = IPS200_WIDTH;
static int ips200_height = IPS200_HEIGHT;
static unsigned short *screen_base = NULL;
static int fb_fd = -1;
static unsigned int screen_size = 0;

// 8x16 ASCII 字体（简化版本，仅支持部分常用字符）
// 实际使用时建议引入完整字体库
static const uint8_t ascii_font_8x16[][16] = {
    // 空格 (32)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 更多字符需要完整字体库...
};

/**
 * @brief 初始化 IPS200 屏幕显示
 */
int ips200_display_init(const char *fb_device) {
    struct fb_fix_screeninfo fb_fix;
    struct fb_var_screeninfo fb_var;

    // 打开 Framebuffer 设备
    fb_fd = open(fb_device, O_RDWR);
    if (fb_fd < 0) {
        perror("Failed to open framebuffer device");
        return -1;
    }

    // 获取屏幕固定参数
    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &fb_fix) < 0) {
        perror("Failed to get fixed screen info");
        close(fb_fd);
        fb_fd = -1;
        return -1;
    }

    // 获取屏幕可变参数
    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &fb_var) < 0) {
        perror("Failed to get variable screen info");
        close(fb_fd);
        fb_fd = -1;
        return -1;
    }

    // 计算屏幕尺寸
    screen_size = fb_fix.line_length * fb_var.yres;
    ips200_width = fb_var.xres;
    ips200_height = fb_var.yres;

    printf("IPS200 Screen Info: %dx%d, bpp=%d\n",
           ips200_width, ips200_height, fb_var.bits_per_pixel);

    // 映射显存到用户空间
    screen_base = (unsigned short *)mmap(NULL, screen_size,
                                          PROT_READ | PROT_WRITE,
                                          MAP_SHARED, fb_fd, 0);
    if (screen_base == MAP_FAILED) {
        perror("Failed to mmap framebuffer");
        close(fb_fd);
        fb_fd = -1;
        return -1;
    }

    // 清屏
    ips200_clear();

    printf("IPS200 display initialized successfully\n");
    return 0;
}

/**
 * @brief 清空屏幕（填充默认背景色）
 */
void ips200_clear(void) {
    ips200_full(IPS200_DEFAULT_BGCOLOR);
}

/**
 * @brief 屏幕填充指定颜色
 */
void ips200_full(const uint16_t color) {
    if (screen_base == NULL) {
        fprintf(stderr, "Screen not initialized\n");
        return;
    }

    for (int i = 0; i < ips200_height; i++) {
        for (int j = 0; j < ips200_width; j++) {
            ips200_draw_point(j, i, color);
        }
    }
}

/**
 * @brief 画点
 */
void ips200_draw_point(uint16_t x, uint16_t y, const uint16_t color) {
    if (screen_base == NULL) {
        return;
    }

    if (x >= ips200_width || y >= ips200_height) {
        return;
    }

    screen_base[y * ips200_width + x] = color;
}

/**
 * @brief 显示灰度图像
 */
void ips200_show_gray_image(uint16_t x, uint16_t y, const uint8_t *image,
                             uint16_t width, uint16_t height) {
    if (screen_base == NULL || image == NULL) {
        fprintf(stderr, "Screen not initialized or invalid image\n");
        return;
    }

    for (uint16_t y_offset = 0; y_offset < height; y_offset++) {
        for (uint16_t x_offset = 0; x_offset < width; x_offset++) {
            uint16_t screen_x = x + x_offset;
            uint16_t screen_y = y + y_offset;

            // 边界检查
            if (screen_x >= ips200_width || screen_y >= ips200_height) {
                continue;
            }

            // 获取灰度值
            uint8_t gray_value = image[y_offset * width + x_offset];

            // 灰度转 RGB565
            // RGB565 格式: RRRRR GGGGGG BBBBB
            uint16_t r = (gray_value >> 3) & 0x1F;  // 5 bits
            uint16_t g = (gray_value >> 2) & 0x3F;  // 6 bits
            uint16_t b = (gray_value >> 3) & 0x1F;  // 5 bits
            uint16_t color = (r << 11) | (g << 5) | b;

            ips200_draw_point(screen_x, screen_y, color);
        }
    }
}

/**
 * @brief 显示字符（简化版本）
 */
static void ips200_show_char(uint16_t x, uint16_t y, char ch) {
    // 简化实现：仅绘制一个简单的方块表示字符
    // 实际使用时需要完整的字体库
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            ips200_draw_point(x + i, y + j, IPS200_DEFAULT_PENCOLOR);
        }
    }
}

/**
 * @brief 显示字符串
 */
void ips200_show_string(uint16_t x, uint16_t y, const char *str) {
    if (str == NULL || screen_base == NULL) {
        return;
    }

    uint16_t offset = 0;
    while (*str != '\0') {
        ips200_show_char(x + offset, y, *str);
        offset += 8;  // 字符宽度 8 像素
        str++;
    }
}

/**
 * @brief 关闭 IPS200 显示
 */
void ips200_display_close(void) {
    if (screen_base != NULL && screen_base != MAP_FAILED) {
        munmap(screen_base, screen_size);
        screen_base = NULL;
    }

    if (fb_fd >= 0) {
        close(fb_fd);
        fb_fd = -1;
    }

    printf("IPS200 display closed\n");
}
