#ifndef _NETWORK_STREAM_H_
#define _NETWORK_STREAM_H_

#include <stdint.h>

// 网络传输配置
#define NETWORK_PORT 8888           // TCP端口
#define MAX_CLIENTS  2              // 最大客户端连接数

// 图像数据包头
struct ImageHeader {
    uint32_t magic;                 // 魔数：0x12345678
    uint32_t width;                 // 图像宽度
    uint32_t height;                // 图像高度
    uint32_t data_size;             // 数据大小（字节）
    uint32_t timestamp;             // 时间戳（毫秒）
};

/**
 * @brief 初始化网络流服务器
 * @param port TCP端口号
 * @return 0:成功 -1:失败
 */
int network_stream_init(int port);

/**
 * @brief 发送图像数据到所有连接的客户端
 * @param image 图像数据指针（灰度图）
 * @param width 图像宽度
 * @param height 图像高度
 * @return 发送的客户端数量
 */
int network_stream_send(const uint8_t *image, uint16_t width, uint16_t height);

/**
 * @brief 获取当前连接的客户端数量
 * @return 客户端数量
 */
int network_stream_get_clients();

/**
 * @brief 关闭网络流服务器
 */
void network_stream_close();

#endif // _NETWORK_STREAM_H_
