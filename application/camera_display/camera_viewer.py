#!/usr/bin/env python3
"""
龙芯LS2K0300摄像头图像接收与显示客户端

功能：
1. 连接到板卡的TCP服务器（端口8888）
2. 接收图像数据包（头部+灰度图像数据）
3. 在电脑窗口中实时显示图像

使用方法：
python3 camera_viewer.py <板卡IP地址>
例如：python3 camera_viewer.py 192.168.110.250

依赖：
pip3 install opencv-python numpy
"""

import socket
import struct
import numpy as np
import cv2
import sys
import time

# 网络配置
NETWORK_PORT = 8888
RECV_BUFFER_SIZE = 65536

# 图像包头结构体：uint32_t magic, width, height, data_size, timestamp
HEADER_FORMAT = '<5I'  # Little-endian, 5个unsigned int (4字节)
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)
MAGIC_NUMBER = 0x12345678


class CameraViewer:
    def __init__(self, board_ip):
        self.board_ip = board_ip
        self.socket = None
        self.connected = False
        self.frame_count = 0
        self.start_time = None

    def connect(self):
        """连接到板卡服务器"""
        try:
            print(f"正在连接到板卡 {self.board_ip}:{NETWORK_PORT}...")
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.board_ip, NETWORK_PORT))
            self.connected = True
            print("连接成功！")
            self.start_time = time.time()
            return True
        except Exception as e:
            print(f"连接失败: {e}")
            return False

    def recv_exact(self, size):
        """接收指定大小的数据"""
        data = b''
        while len(data) < size:
            packet = self.socket.recv(size - len(data))
            if not packet:
                return None
            data += packet
        return data

    def receive_frame(self):
        """接收一帧图像"""
        try:
            # 1. 接收包头
            header_data = self.recv_exact(HEADER_SIZE)
            if not header_data:
                print("连接断开")
                return None

            # 2. 解析包头
            magic, width, height, data_size, timestamp = struct.unpack(HEADER_FORMAT, header_data)

            # 3. 验证魔数
            if magic != MAGIC_NUMBER:
                print(f"警告：魔数错误 0x{magic:08X}，期望 0x{MAGIC_NUMBER:08X}")
                return None

            # 4. 接收图像数据
            image_data = self.recv_exact(data_size)
            if not image_data:
                print("图像数据接收失败")
                return None

            # 5. 转换为numpy数组
            image = np.frombuffer(image_data, dtype=np.uint8)
            image = image.reshape((height, width))

            self.frame_count += 1
            return image

        except Exception as e:
            print(f"接收帧失败: {e}")
            return None

    def run(self):
        """主循环：接收并显示图像"""
        if not self.connect():
            return

        window_name = f"LS2K0300 Camera - {self.board_ip}"
        cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
        cv2.resizeWindow(window_name, 800, 600)  # 设置固定窗口大小
        cv2.moveWindow(window_name, 100, 100)     # 移动窗口到屏幕可见位置

        print("\n开始接收图像...")
        print("按 'q' 或 ESC 键退出\n")

        try:
            while True:
                # 接收图像帧
                frame = self.receive_frame()
                if frame is None:
                    break

                # 显示图像（放大2倍以便观看）
                display_frame = cv2.resize(frame, None, fx=4, fy=4, interpolation=cv2.INTER_NEAREST)
                cv2.imshow(window_name, display_frame)

                # 显示帧率
                if self.frame_count % 30 == 0:
                    elapsed = time.time() - self.start_time
                    fps = self.frame_count / elapsed
                    print(f"帧率: {fps:.1f} FPS, 总帧数: {self.frame_count}")

                # 处理按键
                key = cv2.waitKey(1) & 0xFF
                if key == ord('q') or key == 27:  # 'q' 或 ESC
                    print("\n用户退出")
                    break

        except KeyboardInterrupt:
            print("\n用户中断（Ctrl+C）")
        except Exception as e:
            print(f"\n错误: {e}")
        finally:
            self.cleanup()

    def cleanup(self):
        """清理资源"""
        print("\n正在关闭...")
        if self.socket:
            self.socket.close()
        cv2.destroyAllWindows()

        if self.start_time:
            elapsed = time.time() - self.start_time
            if elapsed > 0:
                avg_fps = self.frame_count / elapsed
                print(f"统计：接收 {self.frame_count} 帧，平均帧率 {avg_fps:.1f} FPS")


def main():
    if len(sys.argv) != 2:
        print("用法: python3 camera_viewer.py <板卡IP地址>")
        print("示例: python3 camera_viewer.py 192.168.110.250")
        sys.exit(1)

    board_ip = sys.argv[1]

    print("=" * 60)
    print("  LS2K0300 摄像头图像接收客户端")
    print("=" * 60)
    print(f"板卡IP: {board_ip}")
    print(f"端口:   {NETWORK_PORT}")
    print("=" * 60)

    viewer = CameraViewer(board_ip)
    viewer.run()


if __name__ == "__main__":
    main()
