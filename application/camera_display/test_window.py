#!/usr/bin/env python3
"""
简单测试：验证窗口是否能显示
"""

import cv2
import numpy as np
import time

print("创建测试窗口...")
window_name = "测试窗口 - 如果能看到这个窗口说明OpenCV正常"
cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
cv2.resizeWindow(window_name, 800, 600)
cv2.moveWindow(window_name, 100, 100)

print("显示彩色测试图案...")
# 创建彩色测试图像
test_image = np.zeros((600, 800, 3), dtype=np.uint8)
test_image[:200, :] = [255, 0, 0]    # 蓝色
test_image[200:400, :] = [0, 255, 0]  # 绿色
test_image[400:, :] = [0, 0, 255]    # 红色

# 添加文字
cv2.putText(test_image, "OpenCV Test Window", (200, 300),
            cv2.FONT_HERSHEY_SIMPLEX, 2, (255, 255, 255), 3)

print("\n" + "="*60)
print("如果能看到窗口，说明OpenCV显示功能正常！")
print("窗口标题：测试窗口 - 如果能看到这个窗口说明OpenCV正常")
print("窗口内容：三色条纹（蓝、绿、红）和文字")
print("\n请尝试：")
print("1. 按 Alt+Tab 切换窗口")
print("2. 查看任务栏是否有新窗口")
print("3. 最小化其他窗口")
print("\n按任意键关闭测试窗口，或等待10秒自动关闭")
print("="*60 + "\n")

cv2.imshow(window_name, test_image)

# 等待10秒或按键
key = cv2.waitKey(10000)
if key != -1:
    print(f"检测到按键: {key}")
else:
    print("10秒超时")

cv2.destroyAllWindows()
print("测试完成")
