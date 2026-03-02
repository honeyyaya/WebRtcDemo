cmake_minimum_required(VERSION 3.16)

project(WebRtcDemo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# libmediastream 子项目
# ==============================================================================
add_subdirectory(libmediastream)

# ==============================================================================
# Qt 应用
# ==============================================================================
find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appWebRtcDemo
    main.cpp
    VideoFrameProvider.h
    VideoFrameProvider.cpp
)

qt_add_qml_module(appWebRtcDemo
    URI WebRtcDemo
    QML_FILES
        Main.qml
)

set_target_properties(appWebRtcDemo PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appWebRtcDemo
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appWebRtcDemo
    PRIVATE Qt6::Quick
    PRIVATE mediastream
)

include(GNUInstallDirs)
install(TARGETS appWebRtcDemo
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
