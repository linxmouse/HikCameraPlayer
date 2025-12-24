cmake_minimum_required(VERSION 3.16)
project(HikCameraPlayer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Multimedia)
qt_standard_project_setup(REQUIRES 6.10)
set(QT_QML_GENERATE_QMLLS_INI ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

# 使用ffmpeg将png转换为ico
# ffmpeg -i app_icon.png -vf "scale=256:256" app_icon.ico
qt_add_executable(HikCameraPlayer WIN32
    main.cpp
    app.rc
)

qt_add_qml_module(HikCameraPlayer
    URI HikCameraPlayer
    VERSION 1.0
    QML_FILES 
        Main.qml
    RESOURCES
        app_icon.png
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
