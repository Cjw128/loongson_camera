#!/bin/bash
########################################
# 简化版交叉编译 - 只编译 IPS200 版本
# 使用运行时库路径解决ABI不兼容问题
########################################

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build_simple"
OUTPUT_DIR="${PROJECT_DIR}/../../output"

# 工具链
TOOLCHAIN="/opt/ls_2k0300_env/loongson-gnu-toolchain-8.3-x86_64-loongarch64-linux-gnu-rc1.6"
CC="${TOOLCHAIN}/bin/loongarch64-linux-gnu-gcc"
CXX="${TOOLCHAIN}/bin/loongarch64-linux-gnu-g++"

# Sysroot
SYSROOT="${PROJECT_DIR}/../../rootfs/rootfs_loongos"

# OpenCV 包含路径（使用 sysroot 中的）
OPENCV_INCLUDE="-I${SYSROOT}/usr/include/opencv4"

# 板卡上 OpenCV 的运行时路径
OPENCV_RPATH="/home/root/opencv/lib"

echo "========================================"
echo " 简化版龙芯交叉编译"
echo "========================================"

# 创建构建目录
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
mkdir -p "${OUTPUT_DIR}"

cd "${BUILD_DIR}"

# 编译源文件
echo "[1/3] 编译源文件..."

${CXX} -c ../src/main.cpp \
    --sysroot=${SYSROOT} \
    -march=loongarch64 -mabi=lp64d \
    -I../include \
    ${OPENCV_INCLUDE} \
    -O2 -Wall -std=c++11

${CXX} -c ../src/uvc_camera.cpp \
    --sysroot=${SYSROOT} \
    -march=loongarch64 -mabi=lp64d \
    -I../include \
    ${OPENCV_INCLUDE} \
    -O2 -Wall -std=c++11

${CXX} -c ../src/ips200_display.cpp \
    --sysroot=${SYSROOT} \
    -march=loongarch64 -mabi=lp64d \
    -I../include \
    ${OPENCV_INCLUDE} \
    -O2 -Wall -std=c++11

${CXX} -c ../src/network_stream.cpp \
    --sysroot=${SYSROOT} \
    -march=loongarch64 -mabi=lp64d \
    -I../include \
    ${OPENCV_INCLUDE} \
    -O2 -Wall -std=c++11

# 链接
echo "[2/3] 链接可执行文件..."

${CXX} main.o uvc_camera.o ips200_display.o network_stream.o \
    --sysroot=${SYSROOT} \
    -march=loongarch64 -mabi=lp64d \
    -L${SYSROOT}/usr/lib64 \
    -Wl,-rpath,${OPENCV_RPATH} \
    -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_core \
    -lpthread \
    -o camera_display_ips200

# 复制到输出目录
echo "[3/3] 复制到输出目录..."
cp camera_display_ips200 "${OUTPUT_DIR}/"

echo ""
echo "========================================"
echo " 编译完成！"
echo "========================================"
echo "可执行文件: ${OUTPUT_DIR}/camera_display_ips200"
echo ""
echo "传输到板卡："
echo "  scp ${OUTPUT_DIR}/camera_display_ips200 root@192.168.110.250:~/"
echo ""
echo "在板卡上运行："
echo "  LD_LIBRARY_PATH=/home/root/opencv/lib ./camera_display_ips200"
echo "========================================"
