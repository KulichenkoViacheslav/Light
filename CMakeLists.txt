cmake_minimum_required(VERSION 3.5)

project(Light VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES Light_uk_UA.ts)

set(PROJECT_SOURCES
    main.cpp
    light.cpp
    light.h
    light.ui
    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Light
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        configs.h
        configs.cpp
        configs.ui
        installer.qs # Додайте шлях до вашого скрипту інсталятора Qt тут
    )
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(Light SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(Light
            ${PROJECT_SOURCES}
        )
    endif()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(Light PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(Light PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Light
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(Light)

# Налаштування Qt Installer Framework
find_program(WINDEPLOYQT windeployqt HINTS "${_qt_bin_dir}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/deploy-qt-windows.cmake.in" "${CMAKE_CURRENT_SOURCE_DIR}/deploy-qt-windows.cmake" @ONLY)

set(CPACK_PRE_BUILD_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/deploy-qt-windows.cmake)

set(CPACK_PACKAGE_NAME "Light")
set(CPACK_PACKAGE_VENDOR "LBME NTU 'KhPI'")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Light Installation")
set(CPACK_PACKAGE_EXECUTABLES "Light" "Light")

set(CPACK_GENERATOR "IFW")
set(CPACK_IFW_ROOT "C:/Qt/Tools/QtInstallerFramework/4.7")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_IFW_PACKAGE_TITLE "Light Installer")
#set(CPACK_IFW_PACKAGE_ICON "icon.ico")
#set(CPACK_IFW_PACKAGE_MAINTAINER "LBME NTU 'KhPI'")
#set(CPACK_IFW_PACKAGE_SCRIPT "path/to/installer.qs")

include(CPack)
include(CPackIFW)
