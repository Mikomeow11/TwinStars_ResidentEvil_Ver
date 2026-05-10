cmake_minimum_required(VERSION 3.16)

project(TwinStars_ResidentEvil_ver VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        MyWindow.cpp
        MyWindow.h
        mywindow.ui
        Player.h
        Player.cpp
        GameScene.h
        GameScene.cpp
        Platform.h
        Platform.cpp
        Ground.h
        Ground.cpp
        Spike.h
        Spike.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TwinStars_ResidentEvil_ver
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        resources.qrc
    )
# 如需移植到 Android，可在这里补充 Qt 6 的 Android 工程目录配置。
else()
    if(ANDROID)
        add_library(TwinStars_ResidentEvil_ver SHARED
            ${PROJECT_SOURCES}
        )
# 如需使用 Qt 5 构建 Android 版本，可在这里设置 Android 工程目录。
    else()
        add_executable(TwinStars_ResidentEvil_ver
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(TwinStars_ResidentEvil_ver PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# iOS/macOS 打包时建议设置固定的应用标识符。
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TwinStars_ResidentEvil_ver)
endif()
set_target_properties(TwinStars_ResidentEvil_ver PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS TwinStars_ResidentEvil_ver
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TwinStars_ResidentEvil_ver)
endif()
