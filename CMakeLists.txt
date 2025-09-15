cmake_minimum_required(VERSION 3.15)
project(ImGuiDX11App)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)
set(LIBRARIES_DIR ${CMAKE_SOURCE_DIR}/libraries)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_impl_win32.cpp
    ${IMGUI_DIR}/imgui_impl_dx11.cpp
)

set(LIBRARIES_SOURCES
    ${LIBRARIES_DIR}/bind.cpp
    ${LIBRARIES_DIR}/general.cpp
    ${LIBRARIES_DIR}/imgui_lib.cpp
    ${LIBRARIES_DIR}/init.cpp
    ${LIBRARIES_DIR}/tasks.cpp
    ${LIBRARIES_DIR}/wndproc.cpp
)

set(APP_SOURCES
    main.cpp
    ${LIBRARIES_SOURCES}
    ${IMGUI_SOURCES}
)

add_executable(ImGuiDX11App ${APP_SOURCES})

target_include_directories(ImGuiDX11App PRIVATE
    ${IMGUI_DIR}
    ${LIBRARIES_DIR}
)

target_link_libraries(ImGuiDX11App
    d3d11
    d3dcompiler
    dxgi
    user32
    gdi32
    kernel32
    winspool
    shell32
    ole32
    oleaut32
    uuid
    comdlg32
    advapi32
)

set_target_properties(ImGuiDX11App PROPERTIES
    WIN32_EXECUTABLE TRUE
)