cmake_minimum_required(VERSION 3.16)
project(HikRealPlayer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)
qt_standard_project_setup(REQUIRES 6.10)

# 设置二进制输出路径到 bin 目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

qt_add_executable(HikRealPlayer WIN32
    main.cpp
)

qt_add_qml_module(HikRealPlayer
    URI HikRealPlayer
    VERSION 1.0
    QML_FILES Main.qml
    SOURCES
        src/AppController.h
        src/AppController.cpp
)

target_include_directories(HikRealPlayer PRIVATE 
    src
    sdk
)

target_link_libraries(HikRealPlayer
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sdk/HCNetSDK.lib
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sdk/PlayCtrl.lib
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sdk/HCCore.lib
)

# 获取 SDK 目录下的所有 DLL 文件
file(GLOB SDK_DLLS "${CMAKE_CURRENT_SOURCE_DIR}/sdk/*.dll")
# 复制 SDK DLL 文件到 bin 目录
add_custom_command(TARGET HikRealPlayer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDK_DLLS}
        "$<TARGET_FILE_DIR:HikRealPlayer>"
    COMMENT "Copying SDK DLLs to bin directory..."
)
# 复制 HCNetSDKCom 文件夹到 bin 目录
add_custom_command(TARGET HikRealPlayer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/sdk/HCNetSDKCom"
        "$<TARGET_FILE_DIR:HikRealPlayer>/HCNetSDKCom"
    COMMENT "Copying HCNetSDKCom folder to bin directory..."
)
