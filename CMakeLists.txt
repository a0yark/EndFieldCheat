cmake_minimum_required(VERSION 3.14)
project(EndfieldCheat)

set(CMAKE_CXX_STANDARD 17)

# MSVC UTF-8 source/execution charset
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")

# --- Dear ImGui (local) ---
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

# Build ImGui as a static library
add_library(imgui_lib STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
)
target_include_directories(imgui_lib PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# --- MinHook (local) ---
set(MINHOOK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/minhook)

add_library(minhook STATIC
    ${MINHOOK_DIR}/src/buffer.c
    ${MINHOOK_DIR}/src/hook.c
    ${MINHOOK_DIR}/src/trampoline.c
    ${MINHOOK_DIR}/src/hde/hde32.c
    ${MINHOOK_DIR}/src/hde/hde64.c
)
target_include_directories(minhook PUBLIC
    ${MINHOOK_DIR}/include
)

# --- Main DLL ---
add_library(EndfieldCheat SHARED
    DllMain.cpp
    Cheat.cpp
    Hook.cpp
    GameStructs.h
    Hook.h
)

target_link_libraries(EndfieldCheat PRIVATE
    imgui_lib
    minhook
    d3d11
    dxgi
    user32
    kernel32
)
