cmake_minimum_required(VERSION 3.10)
project(camera_display)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2")

# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 查找 OpenCV
# 优先尝试 OpenCV 4.x，如果找不到则尝试 OpenCV 3.x
find_package(OpenCV QUIET)
if(NOT OpenCV_FOUND)
    # 如果pkg-config可用，尝试使用它查找OpenCV
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(OpenCV opencv4 QUIET)
        if(NOT OpenCV_FOUND)
            pkg_check_modules(OpenCV opencv QUIET)
        endif()
    endif()
endif()

if(OpenCV_FOUND)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
    include_directories(${OpenCV_INCLUDE_DIRS})
else()
    message(WARNING "OpenCV not found via find_package or pkg-config")
    message(STATUS "Trying to use system OpenCV paths...")

    # 尝试常见的系统路径
    set(OpenCV_INCLUDE_DIRS "/usr/include/opencv4" "/usr/include")
    set(OpenCV_LIBS opencv_core opencv_imgproc opencv_highgui opencv_videoio)
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()

# 源文件 - IPS200版本（需要逐飞内核）
set(SOURCES_IPS200
    src/main.cpp
    src/uvc_camera.cpp
    src/ips200_display.cpp
)

# 源文件 - 通用版本（使用OpenCV窗口显示）
set(SOURCES_GENERIC
    src/main_generic.cpp
    src/uvc_camera.cpp
    src/screen_display.cpp
)

# 生成可执行文件 - IPS200版本
add_executable(camera_display_ips200 ${SOURCES_IPS200})
target_link_libraries(camera_display_ips200
    ${OpenCV_LIBS}
    pthread
)

# 生成可执行文件 - 通用版本（推荐）
add_executable(camera_display ${SOURCES_GENERIC})
target_link_libraries(camera_display
    ${OpenCV_LIBS}
    pthread
)

# 安装规则（可选）
install(TARGETS camera_display camera_display_ips200 DESTINATION bin)

# 打印信息
message(STATUS "========================================")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libs: ${OpenCV_LIBS}")
message(STATUS "")
message(STATUS "Build targets:")
message(STATUS "  1. camera_display        - 通用版本（OpenCV窗口，推荐）")
message(STATUS "  2. camera_display_ips200 - IPS200版本（需要逐飞内核）")
message(STATUS "========================================")
