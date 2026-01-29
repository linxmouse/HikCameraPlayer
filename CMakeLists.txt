cmake_minimum_required(VERSION 3.16)
project(HikCameraPlayer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Multimedia)
qt_standard_project_setup(REQUIRES 6.8)
set(QT_QML_GENERATE_QMLLS_INI ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

# Debug模式支持QML调试
string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
if (BUILD_TYPE_UPPER STREQUAL "DEBUG")
    add_definitions(-DQT_QML_DEBUG)
endif()
# 使用ffmpeg将png转换为ico
# ffmpeg -i app_icon.png -vf "scale=256:256" app_icon.ico
qt_add_executable(HikCameraPlayer WIN32
    main.cpp
    app.rc
)

qt_add_qml_module(HikCameraPlayer
    # 这里定义了模块标识
    URI work.dbugs.hikcamera
    VERSION 1.0
    QML_FILES 
        Main.qml
    RESOURCES
        app_icon.png
    # 自动扫描 QML_ELEMENT
    SOURCES
        src/AppController.h
        src/AppController.cpp
)

target_include_directories(HikCameraPlayer PRIVATE 
    src
    sdk
)

target_link_libraries(HikCameraPlayer
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
    PRIVATE Qt6::Multimedia
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sdk/HCNetSDK.lib
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sdk/PlayCtrl.lib
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sdk/HCCore.lib
)

file(GLOB SDK_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/sdk/*.dll")
add_custom_command(TARGET HikCameraPlayer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDK_DLLS}
        "$<TARGET_FILE_DIR:HikCameraPlayer>"
    COMMENT "Copying SDK DLLs to bin directory..."
)
add_custom_command(TARGET HikCameraPlayer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/sdk/HCNetSDKCom"
        "$<TARGET_FILE_DIR:HikCameraPlayer>/HCNetSDKCom"
    COMMENT "Copying HCNetSDKCom folder to bin directory..."
)

if(WINDEPLOYQT_EXECUTABLE AND NOT WINDEPLOYQT_EXECUTABLE MATCHES "Qt/6")
    unset(WINDEPLOYQT_EXECUTABLE CACHE)
endif()
get_target_property(QT_BIN_DIR Qt6::Core LOCATION)
get_filename_component(QT_BIN_DIR "${QT_BIN_DIR}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}" NO_DEFAULT_PATH)
if(WINDEPLOYQT_EXECUTABLE)
    add_custom_command(TARGET HikCameraPlayer POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}"
            --qmldir "${CMAKE_CURRENT_SOURCE_DIR}"
            "$<TARGET_FILE:HikCameraPlayer>"
        COMMENT "Running windeployqt..."
    )
endif()

# -S . 表示当前目录是源码
# -B build 表示把构建文件放在 build 文件夹
# -DCMAKE_BUILD_TYPE=Release 指定构建类型为 Release
# -DCMAKE_PREFIX_PATH 指定 Qt 的安装路径
# cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="D:/Qt/6.10.1/msvc2022_64"

# --build build：告诉 CMake 去 build 文件夹里找图纸并开始编译
# --clean-first：先删除之前编译产生的旧文件, 防止因为旧缓存导致莫名其妙的报错
# --config Release：明确我们要 Release 版本
# cmake --build build --clean-first --config Release
