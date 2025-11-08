#!/usr/bin/env python3
"""
龙芯LS2K0300摄像头图像接收与显示客户端 - 调试版本

添加了详细的调试信息来诊断问题
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
            print("✓ 连接成功！")
            self.start_time = time.time()
            return True
        except Exception as e:
            print(f"✗ 连接失败: {e}")
            return False

    def recv_exact(self, size):
        """接收指定大小的数据"""
        data = b''
        try:
            while len(data) < size:
                remaining = size - len(data)
                packet = self.socket.recv(remaining)
                if not packet:
                    print(f"\n[错误] Socket连接断开，已接收 {len(data)}/{size} 字节")
                    return None
                data += packet
            return data
        except socket.timeout:
            print(f"\n[错误] Socket超时，已接收 {len(data)}/{size} 字节")
            return None
        except Exception as e:
            print(f"\n[错误] 接收数据异常: {e}，已接收 {len(data)}/{size} 字节")
            return None

    def receive_frame(self, verbose=True):
        """接收一帧图像"""
        try:
            # 1. 接收包头
            if verbose:
                print(f"[调试] 正在接收包头 ({HEADER_SIZE} 字节)...", end='', flush=True)
            header_data = self.recv_exact(HEADER_SIZE)
            if not header_data:
                if verbose:
                    print(" ✗ 连接断开")
                return None
            if verbose:
                print(" ✓")

            # 2. 解析包头
            magic, width, height, data_size, timestamp = struct.unpack(HEADER_FORMAT, header_data)

            if verbose:
                print(f"[调试] 包头信息:")
                print(f"  - 魔数: 0x{magic:08X} (期望: 0x{MAGIC_NUMBER:08X})")
                print(f"  - 图像尺寸: {width}x{height}")
                print(f"  - 数据大小: {data_size} 字节")
                print(f"  - 时间戳: {timestamp} ms")

            # 3. 验证魔数
            if magic != MAGIC_NUMBER:
                print(f"✗ 警告：魔数错误！")
                return None

            # 4. 接收图像数据
            if verbose:
                print(f"[调试] 正在接收图像数据 ({data_size} 字节)...", end='', flush=True)
            image_data = self.recv_exact(data_size)
            if not image_data:
                if verbose:
                    print(" ✗ 图像数据接收失败")
                return None
            if verbose:
                print(" ✓")

            # 5. 转换为numpy数组
            if verbose:
                print(f"[调试] 转换为图像数组...", end='', flush=True)
            image = np.frombuffer(image_data, dtype=np.uint8)
            image = image.reshape((height, width))
            if verbose:
                print(f" ✓ (形状: {image.shape})")

            self.frame_count += 1
            if verbose:
                print(f"[调试] 第 {self.frame_count} 帧接收完成\n")
            return image

        except Exception as e:
            print(f"\n✗ 接收帧失败: {e}")
            import traceback
            traceback.print_exc()
            return None

    def run(self):
        """主循环：接收并显示图像"""
        if not self.connect():
            return

        window_name = f"LS2K0300 Camera - {self.board_ip}"

        print(f"\n[调试] 创建OpenCV窗口: '{window_name}'")
        try:
            cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
            print("✓ OpenCV窗口创建成功")
        except Exception as e:
            print(f"✗ OpenCV窗口创建失败: {e}")
            print("  可能原因：")
            print("  1. 没有图形界面（SSH远程登录）")
            print("  2. DISPLAY环境变量未设置")
            print("  3. OpenCV安装不完整")
            return

        print("\n" + "="*60)
        print("开始接收图像...")
        print("按 'q' 或 ESC 键退出")
        print("="*60 + "\n")

        try:
            frame_num = 0
            while True:
                frame_num += 1

                # 前3帧显示详细信息，之后简化显示
                if frame_num <= 3:
                    print(f"{'='*60}")
                    print(f"准备接收第 {frame_num} 帧")
                    print(f"{'='*60}")

                # 接收图像帧
                frame = self.receive_frame(verbose=(frame_num <= 3))
                if frame is None:
                    print("✗ 接收失败，退出")
                    break

                # 显示图像（放大4倍以便观看）
                if frame_num <= 3:
                    print(f"[调试] 缩放图像 (4倍)...", end='', flush=True)
                display_frame = cv2.resize(frame, None, fx=4, fy=4, interpolation=cv2.INTER_NEAREST)
                if frame_num <= 3:
                    print(f" ✓ (新尺寸: {display_frame.shape})")

                if frame_num <= 3:
                    print(f"[调试] 显示图像到窗口...", end='', flush=True)
                cv2.imshow(window_name, display_frame)
                if frame_num <= 3:
                    print(" ✓")

                # 第3帧后提示切换到简化模式
                if frame_num == 3:
                    print(f"\n{'='*60}")
                    print("✓ 前3帧调试信息显示完成")
                    print("✓ 窗口显示正常，切换到简化模式...")
                    print("✓ 现在持续接收并显示图像")
                    print("  （每30帧显示一次帧率统计）")
                    print(f"{'='*60}\n")

                # 显示帧率
                if self.frame_count % 30 == 0:
                    elapsed = time.time() - self.start_time
                    fps = self.frame_count / elapsed
                    print(f"\n📊 帧率统计: {fps:.1f} FPS, 总帧数: {self.frame_count}")

                # 处理按键
                key = cv2.waitKey(1) & 0xFF

                if key == ord('q') or key == 27:  # 'q' 或 ESC
                    print("\n✓ 用户按键退出")
                    break

        except KeyboardInterrupt:
            print("\n✓ 用户中断（Ctrl+C）")
        except Exception as e:
            print(f"\n✗ 错误: {e}")
            import traceback
            traceback.print_exc()
        finally:
            self.cleanup()

    def cleanup(self):
        """清理资源"""
        print("\n正在关闭...")
        if self.socket:
            self.socket.close()
            print("✓ Socket已关闭")
        cv2.destroyAllWindows()
        print("✓ 窗口已销毁")

        if self.start_time:
            elapsed = time.time() - self.start_time
            if elapsed > 0:
                avg_fps = self.frame_count / elapsed
                print(f"\n📊 统计：接收 {self.frame_count} 帧，平均帧率 {avg_fps:.1f} FPS")


def main():
    if len(sys.argv) != 2:
        print("用法: python3 camera_viewer_debug.py <板卡IP地址>")
        print("示例: python3 camera_viewer_debug.py 192.168.110.250")
        sys.exit(1)

    board_ip = sys.argv[1]

    print("=" * 60)
    print("  LS2K0300 摄像头图像接收客户端 [调试版本]")
    print("=" * 60)
    print(f"板卡IP: {board_ip}")
    print(f"端口:   {NETWORK_PORT}")
    print("=" * 60)

    # 检查OpenCV环境
    print("\n[环境检查]")
    print(f"OpenCV版本: {cv2.__version__}")
    print(f"NumPy版本: {np.__version__}")

    # 检查DISPLAY环境变量
    import os
    display = os.environ.get('DISPLAY', '未设置')
    print(f"DISPLAY环境变量: {display}")
    if display == '未设置':
        print("⚠️  警告: DISPLAY未设置，可能无法显示窗口")
        print("   如果是SSH登录，需要使用 ssh -X 启用X11转发")

    print("=" * 60 + "\n")

    viewer = CameraViewer(board_ip)
    viewer.run()


if __name__ == "__main__":
    main()
