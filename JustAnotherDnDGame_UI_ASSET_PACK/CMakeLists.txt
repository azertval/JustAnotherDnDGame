cmake_minimum_required(VERSION 3.21)
project(JADG_UI_ASSET_PREVIEW LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_add_executable(JADG_UI_ASSET_PREVIEW
    src/main.cpp
    JADG_UI_PREVIEW.qrc
)

target_link_libraries(JADG_UI_ASSET_PREVIEW PRIVATE
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2
)

if (WIN32)
    set_target_properties(JADG_UI_ASSET_PREVIEW PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
