#!/usr/bin/env python3
"""
龙芯LS2K0300摄像头图像接收客户端 - 无GUI版本
保存图像到文件而不是显示窗口
"""

import socket
import struct
import numpy as np
import cv2
import sys
import time
import os

# 网络配置
NETWORK_PORT = 8888
RECV_BUFFER_SIZE = 65536

# 图像包头结构体
HEADER_FORMAT = '<5I'
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)
MAGIC_NUMBER = 0x12345678

# 保存目录
SAVE_DIR = "captured_frames"


class CameraViewer:
    def __init__(self, board_ip):
        self.board_ip = board_ip
        self.socket = None
        self.connected = False
        self.frame_count = 0
        self.start_time = None

        # 创建保存目录
        if not os.path.exists(SAVE_DIR):
            os.makedirs(SAVE_DIR)
            print(f"创建保存目录: {SAVE_DIR}/")

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
            # 接收包头
            header_data = self.recv_exact(HEADER_SIZE)
            if not header_data:
                return None

            # 解析包头
            magic, width, height, data_size, timestamp = struct.unpack(HEADER_FORMAT, header_data)

            # 验证魔数
            if magic != MAGIC_NUMBER:
                print(f"警告：魔数错误 0x{magic:08X}")
                return None

            # 接收图像数据
            image_data = self.recv_exact(data_size)
            if not image_data:
                return None

            # 转换为numpy数组
            image = np.frombuffer(image_data, dtype=np.uint8)
            image = image.reshape((height, width))

            self.frame_count += 1
            return image

        except Exception as e:
            print(f"接收帧失败: {e}")
            return None

    def run(self, max_frames=100):
        """主循环：接收并保存图像"""
        if not self.connect():
            return

        print("\n开始接收图像...")
        print(f"图像将保存到: {SAVE_DIR}/")
        print(f"最多保存 {max_frames} 帧")
        print("按 Ctrl+C 提前退出\n")

        try:
            while self.frame_count < max_frames:
                # 接收图像帧
                frame = self.receive_frame()
                if frame is None:
                    break

                # 每10帧保存一次
                if self.frame_count % 10 == 0:
                    filename = f"{SAVE_DIR}/frame_{self.frame_count:04d}.png"
                    cv2.imwrite(filename, frame)
                    print(f"✓ 已保存: {filename}")

                # 显示帧率
                if self.frame_count % 30 == 0:
                    elapsed = time.time() - self.start_time
                    fps = self.frame_count / elapsed
                    print(f"帧率: {fps:.1f} FPS, 总帧数: {self.frame_count}")

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

        if self.start_time:
            elapsed = time.time() - self.start_time
            if elapsed > 0:
                avg_fps = self.frame_count / elapsed
                print(f"统计：接收 {self.frame_count} 帧，平均帧率 {avg_fps:.1f} FPS")
                print(f"图像已保存到: {SAVE_DIR}/")


def main():
    if len(sys.argv) < 2:
        print("用法: python3 camera_saver.py <板卡IP地址> [最大帧数]")
        print("示例: python3 camera_saver.py 192.168.110.250 100")
        sys.exit(1)

    board_ip = sys.argv[1]
    max_frames = int(sys.argv[2]) if len(sys.argv) > 2 else 100

    print("=" * 60)
    print("  LS2K0300 摄像头图像接收客户端 [保存版本]")
    print("=" * 60)
    print(f"板卡IP: {board_ip}")
    print(f"端口:   {NETWORK_PORT}")
    print(f"保存目录: {SAVE_DIR}/")
    print("=" * 60)

    viewer = CameraViewer(board_ip)
    viewer.run(max_frames)


if __name__ == "__main__":
    main()
