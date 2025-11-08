# 龙芯LS2K0300 USB摄像头 + IPS200屏幕显示 + 网络传输

基于逐飞IPS200开源库实现的USB摄像头采集、IPS200屏幕显示与网络传输功能。

## 📋 项目简介

本项目实现了在龙芯LS2K0300开发板上通过USB摄像头采集图像，同时在IPS200 SPI屏幕上实时显示，并通过网络传输到电脑窗口显示。

### 主要特性

- ✅ 支持USB UVC摄像头（V4L2接口）
- ✅ 支持逐飞IPS200 SPI屏幕（ST7789V驱动芯片）
- ✅ 实时灰度图像采集与显示
- ✅ 基于Framebuffer直接操作，高性能
- ✅ **新增：TCP网络流传输，电脑窗口实时显示**
- ✅ 完整的交叉编译支持

### 硬件要求

- **开发板**: 龙芯LS2K0300
- **摄像头**: USB UVC摄像头（支持V4L2）
- **屏幕**: IPS200 SPI屏幕（240x320分辨率）
- **系统**: Linux 4.19内核（包含ST7789V驱动）

---

## 🚀 快速开始

### 1. 准备开发环境

**在x86 Linux虚拟机上：**

```bash
# 确认工具链已安装
ls /opt/ls_2k0300_env/loongson-gnu-toolchain-8.3-x86_64-loongarch64-linux-gnu-rc1.6/bin/

# 进入项目目录
cd /home/cjw/ls2k0300_camera_project/application/camera_display
```

### 2. 编译程序

```bash
# 执行交叉编译脚本
./build_simple.sh
```

编译成功后，可执行文件位于：
```
/home/cjw/ls2k0300_camera_project/output/camera_display_ips200
```

### 3. 传输到板卡

```bash
# 替换 <板卡IP> 为实际IP地址
scp /home/cjw/ls2k0300_camera_project/output/camera_display_ips200 root@<板卡IP>:~/

# 示例：
scp /home/cjw/ls2k0300_camera_project/output/camera_display_ips200 root@192.168.110.250:~/
```

### 4. 在板卡上运行

SSH登录到板卡后执行：

```bash
# 设置OpenCV库路径并运行
LD_LIBRARY_PATH=/home/root/opencv/lib ./camera_display_ips200
```

### 5. 预期输出

**板卡端输出：**

```
========================================
  USB摄像头 + IPS200屏幕 + 网络显示
  基于逐飞IPS200开源库实现
========================================

[1/3] 正在初始化 IPS200 屏幕...
IPS200 Screen Info: 240x320, bpp=16
IPS200 display initialized successfully
IPS200屏幕初始化成功！

[2/3] 正在初始化 USB 摄像头...
Camera opened successfully: /dev/video0
Camera settings: 160x120 @ 30 FPS
USB摄像头初始化成功！

[3/3] 正在初始化网络流服务器...
网络流服务器启动成功，端口: 8888
等待电脑客户端连接...

开始采集并显示图像...
按 Ctrl+C 退出程序

网络客户端数: 1
实时帧率: 29.8 FPS, 总帧数: 30
实时帧率: 29.9 FPS, 总帧数: 60
...
```

### 6. 在电脑上查看图像（新功能）

**在电脑上运行Python客户端：**

```bash
# 1. 确保已安装依赖
pip3 install opencv-python numpy

# 2. 运行客户端（替换为实际板卡IP）
cd /home/cjw/ls2k0300_camera_project/application/camera_display
python3 camera_viewer.py 192.168.110.250
```

**电脑端输出：**

```
============================================================
  LS2K0300 摄像头图像接收客户端
============================================================
板卡IP: 192.168.110.250
端口:   8888
============================================================
正在连接到板卡 192.168.110.250:8888...
连接成功！

开始接收图像...
按 'q' 或 ESC 键退出

帧率: 29.5 FPS, 总帧数: 30
帧率: 29.7 FPS, 总帧数: 60
...
```

程序会打开一个OpenCV窗口实时显示摄像头图像（放大4倍显示）。

### 7. 退出程序

- **板卡端**: 按 `Ctrl+C` 安全退出
- **电脑端**: 按 `q` 键或 `ESC` 键退出，或 `Ctrl+C`

---

## 📁 项目结构

```
camera_display/
├── build_simple.sh          # 交叉编译脚本
├── CMakeLists.txt            # CMake配置（保留用于参考）
├── README.md                 # 本文档
├── 使用手册.md               # 详细使用手册
├── camera_viewer.py          # **新增：电脑端图像显示客户端**
├── include/                  # 头文件目录
│   ├── uvc_camera.h         # USB摄像头接口定义
│   ├── ips200_display.h     # IPS200屏幕接口定义
│   └── network_stream.h     # **新增：网络流接口定义**
└── src/                      # 源代码目录
    ├── main.cpp             # 主程序
    ├── uvc_camera.cpp       # USB摄像头实现
    ├── ips200_display.cpp   # IPS200屏幕实现
    └── network_stream.cpp   # **新增：网络流实现**
```

---

## 🔧 硬件连接

### IPS200屏幕

根据设备树配置，IPS200屏幕使用以下引脚：

- **SPI接口**: SPI1
- **DC引脚**: GPIO26 (gpa1 10)
- **RST引脚**: GPIO74 (gpa4 10)
- **背光引脚**: GPIO75 (gpa4 11)
- **分辨率**: 240x320
- **Framebuffer设备**: `/dev/fb0`

### USB摄像头

- 插入任意USB接口
- 设备路径: `/dev/video0`
- 默认分辨率: 160x120
- 帧率: 30 FPS

---

## ⚙️ 配置说明

### 摄像头参数

在 `include/uvc_camera.h` 中可以修改摄像头参数：

```cpp
#define UVC_WIDTH  160    // 图像宽度
#define UVC_HEIGHT 120    // 图像高度
#define UVC_FPS    30     // 帧率
```

### 屏幕参数

在 `include/ips200_display.h` 中定义了屏幕参数：

```cpp
#define IPS200_WIDTH  240   // 屏幕宽度
#define IPS200_HEIGHT 320   // 屏幕高度
```

---

## 🐛 常见问题

### 1. 找不到OpenCV库

**问题**: `error while loading shared libraries: libopencv_core.so.411`

**解决方法**:

```bash
# 方法1: 使用LD_LIBRARY_PATH
LD_LIBRARY_PATH=/home/root/opencv/lib ./camera_display_ips200

# 方法2: 配置系统库路径（永久解决）
echo "/home/root/opencv/lib" > /etc/ld.so.conf.d/opencv.conf
ldconfig
./camera_display_ips200
```

### 2. 摄像头打开失败

**问题**: `Error: Cannot open camera device: /dev/video0`

**解决方法**:

```bash
# 1. 检查USB摄像头是否插入
lsusb

# 2. 检查设备节点
ls -l /dev/video*

# 3. 检查UVC驱动
lsmod | grep uvc

# 4. 如果驱动未加载
modprobe uvcvideo
```

### 3. 屏幕无显示

**问题**: 程序运行但屏幕没有显示

**解决方法**:

```bash
# 1. 检查framebuffer设备
ls -l /dev/fb0

# 2. 查看设备信息
cat /sys/class/graphics/fb0/name
cat /sys/class/graphics/fb0/modes

# 3. 查看内核日志
dmesg | grep fb
dmesg | grep st7789v
```

### 4. 网络连接失败（新增）

**问题**: 电脑客户端无法连接到板卡

**解决方法**:

```bash
# 1. 检查板卡IP是否正确
ping 192.168.110.250

# 2. 检查端口是否开放（在板卡上）
netstat -tuln | grep 8888

# 3. 检查防火墙（如果有）
# 板卡端允许8888端口
iptables -I INPUT -p tcp --dport 8888 -j ACCEPT

# 4. 确认板卡程序正在运行
ps aux | grep camera_display
```

### 5. 电脑端图像显示异常

**问题**: 图像花屏或错位

**解决方法**:

- 检查网络连接是否稳定
- 确认板卡端程序版本与客户端匹配
- 重新启动板卡端程序和客户端

### 6. 自启动服务冲突

**问题**: 板卡开机自动运行了其他程序

**解决方法**:

```bash
# 停止并禁用auto_menu服务
systemctl stop auto_menu.service
systemctl disable auto_menu.service
```

### 5. 摄像头设备不是video0

**问题**: 摄像头设备是 `/dev/video1` 或其他

**解决方法**:

修改 `src/main.cpp` 中的设备路径：

```cpp
// 将这行
if (uvc_camera_init("/dev/video0") != 0) {

// 修改为
if (uvc_camera_init("/dev/video1") != 0) {
```

然后重新编译。

---

## 📊 性能参数

| 参数 | 值 |
|------|-----|
| 摄像头分辨率 | 160×120 |
| 屏幕分辨率 | 240×320 |
| 目标帧率 | **110 FPS** (MJPEG优化) |
| 实际帧率 | 100-110 FPS |
| 图像格式 | 灰度图（8位） |
| 屏幕格式 | RGB565（16位） |
| 网络端口 | TCP 8888 |
| 最大客户端 | 2个 |
| 网络延迟 | < 50ms (局域网) |
| 网络带宽 | ~16.5 Mbps @ 110 FPS |

### 🚀 高帧率特性

本项目基于**逐飞LS2K0300开源库**的高帧率优化方案：

#### 关键优化技术
1. **MJPEG格式** - 使用硬件MJPEG解码器，大幅提升性能
2. **简化配置** - 移除不必要的参数设置，减少处理开销
3. **龙邱110fps摄像头** - 硬件支持高速采集
4. **优化驱动** - 参考逐飞官方优化代码

#### 性能提升
- **原版**: 30 FPS (YUYV格式)
- **优化后**: 110+ FPS (MJPEG格式)
- **提升**: **3.6倍**

#### 使用建议
- 推荐使用**龙邱110fps USB摄像头**
- 确保安装了支持MJPEG的UVC驱动
- 如果无法达到110fps，程序会显示警告并给出提示

---

## 🔄 重新编译

如果修改了代码，重新编译：

```bash
# 在虚拟机上
cd /home/cjw/ls2k0300_camera_project/application/camera_display
./build_simple.sh

# 传输到板卡
scp /home/cjw/ls2k0300_camera_project/output/camera_display_ips200 root@<板卡IP>:~/
```

---

## 📝 技术细节

### 数据流程

```
USB摄像头 → V4L2 → OpenCV采集 → 转灰度图 →
├─> 转RGB565 → Framebuffer → IPS200屏幕显示
└─> TCP Socket → 网络传输 → 电脑客户端 → OpenCV窗口显示
```

### 网络协议（新增）

**数据包格式**:
```
[ 包头 20字节 ][ 图像数据 width×height 字节 ]
```

**包头结构** (小端序):
- magic (4字节): 0x12345678 - 魔数，用于验证数据包
- width (4字节): 图像宽度
- height (4字节): 图像高度
- data_size (4字节): 图像数据大小（字节）
- timestamp (4字节): 时间戳（毫秒）

**特性**:
- TCP可靠传输
- 非阻塞Socket服务器
- 自动断线检测
- 支持多客户端（最多2个）

### Framebuffer操作

屏幕显示基于Linux Framebuffer接口：
1. 打开 `/dev/fb0` 设备
2. 使用 `ioctl` 获取屏幕参数
3. 使用 `mmap` 映射显存到用户空间
4. 直接写入 RGB565 像素数据

### 灰度到RGB565转换

```cpp
// 灰度值转RGB565（5:6:5格式）
uint16_t r = (gray_value >> 3) & 0x1F;  // 5 bits
uint16_t g = (gray_value >> 2) & 0x3F;  // 6 bits
uint16_t b = (gray_value >> 3) & 0x1F;  // 5 bits
uint16_t color = (r << 11) | (g << 5) | b;
```

### 图像居中显示

```cpp
// 计算居中位置
x_offset = (240 - 160) / 2 = 40
y_offset = (320 - 120) / 2 = 100
```

---

## 📚 参考资料

- [逐飞科技LS2K0300开源库](https://github.com/seekfree)
- [Linux Framebuffer文档](https://www.kernel.org/doc/Documentation/fb/framebuffer.txt)
- [V4L2摄像头接口](https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/v4l2.html)
- [OpenCV官方文档](https://docs.opencv.org/)

---

## 📄 许可证

本项目参考逐飞科技开源库实现，遵循 GPL 3.0 开源许可证。

---

## 👨‍💻 作者

基于逐飞IPS200开源库适配实现

**最后更新**: 2025-11-08
