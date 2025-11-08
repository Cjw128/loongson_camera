#include "network_stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>

// 内部状态
static int server_fd = -1;
static int client_fds[MAX_CLIENTS];
static int client_count = 0;

/**
 * @brief 设置socket为非阻塞模式
 */
static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/**
 * @brief 获取当前时间戳（毫秒）
 */
static uint32_t get_timestamp_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

int network_stream_init(int port) {
    struct sockaddr_in server_addr;
    int opt = 1;

    // 初始化客户端列表
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_fds[i] = -1;
    }
    client_count = 0;

    // 创建socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket创建失败");
        return -1;
    }

    // 设置socket选项
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt失败");
        close(server_fd);
        return -1;
    }

    // 设置为非阻塞
    if (set_nonblocking(server_fd) < 0) {
        perror("设置非阻塞失败");
        close(server_fd);
        return -1;
    }

    // 绑定地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind失败");
        close(server_fd);
        return -1;
    }

    // 开始监听
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen失败");
        close(server_fd);
        return -1;
    }

    printf("网络流服务器已启动，端口: %d\n", port);
    return 0;
}

/**
 * @brief 接受新的客户端连接
 */
static void accept_clients() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int new_fd;

    while (client_count < MAX_CLIENTS) {
        new_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (new_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;  // 没有新连接
            }
            perror("accept失败");
            break;
        }

        // 设置为非阻塞
        set_nonblocking(new_fd);

        // 添加到客户端列表
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] == -1) {
                client_fds[i] = new_fd;
                client_count++;
                printf("新客户端连接: %s:%d [%d/%d]\n",
                       inet_ntoa(client_addr.sin_addr),
                       ntohs(client_addr.sin_port),
                       client_count, MAX_CLIENTS);
                break;
            }
        }
    }
}

/**
 * @brief 移除断开的客户端
 */
static void remove_client(int index) {
    if (client_fds[index] != -1) {
        close(client_fds[index]);
        client_fds[index] = -1;
        client_count--;
        printf("客户端断开连接 [%d/%d]\n", client_count, MAX_CLIENTS);
    }
}

int network_stream_send(const uint8_t *image, uint16_t width, uint16_t height) {
    if (server_fd < 0 || image == NULL) {
        return 0;
    }

    // 接受新连接
    accept_clients();

    if (client_count == 0) {
        return 0;  // 没有客户端连接
    }

    // 准备数据包头
    struct ImageHeader header;
    header.magic = 0x12345678;
    header.width = width;
    header.height = height;
    header.data_size = width * height;
    header.timestamp = get_timestamp_ms();

    int sent_count = 0;

    // 向所有客户端发送
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_fds[i] == -1) continue;

        // 发送头部
        ssize_t n = send(client_fds[i], &header, sizeof(header), MSG_NOSIGNAL);
        if (n != sizeof(header)) {
            remove_client(i);
            continue;
        }

        // 发送图像数据
        n = send(client_fds[i], image, header.data_size, MSG_NOSIGNAL);
        if (n != (ssize_t)header.data_size) {
            remove_client(i);
            continue;
        }

        sent_count++;
    }

    return sent_count;
}

int network_stream_get_clients() {
    return client_count;
}

void network_stream_close() {
    // 关闭所有客户端连接
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_fds[i] != -1) {
            close(client_fds[i]);
            client_fds[i] = -1;
        }
    }

    // 关闭服务器socket
    if (server_fd != -1) {
        close(server_fd);
        server_fd = -1;
    }

    client_count = 0;
    printf("网络流服务器已关闭\n");
}
