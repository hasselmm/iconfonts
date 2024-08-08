cmake_minimum_required(VERSION 3.19)

project(IconFonts VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ICONFONTS_ENABLE_TESTING YES)

set(ICONFONTS_BINARY_DIR            "${CMAKE_CURRENT_BINARY_DIR}")
set(ICONFONTS_SOURCE_DIR            "${CMAKE_CURRENT_SOURCE_DIR}")

set(ICONFONTS_MODULE_DIR            "${ICONFONTS_SOURCE_DIR}/cmake")

list(APPEND CMAKE_MODULE_PATH       "${ICONFONTS_MODULE_DIR}")
set(ICONFONTS_TOOL_EXECUTABLE       "${ICONFONTS_MODULE_DIR}/iconfontstool.py")

add_compile_definitions(
    QT_NO_CAST_FROM_ASCII=1
)

enable_testing()

find_package(
    PythonModules
    REQUIRED COMPONENTS fontTools fontTools.ttx brotli)

find_package(
    Qt6
    REQUIRED COMPONENTS Test Widgets
    OPTIONAL_COMPONENTS QuickTest QuickWidgets
)

qt_policy(SET QTP0001 NEW)
qt_standard_project_setup()

option(ICONFONTS_ENABLE_ALL_FONTS "Enable all known fonts" OFF)

include(IconFonts)

add_subdirectory(iconfonts)
add_subdirectory(iconfontsviewer)
add_subdirectory(namedoptions)
add_subdirectory(tests)

add_custom_target(iconfontsdocs SOURCES LICENSE README.md)
