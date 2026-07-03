cmake_minimum_required(VERSION 3.16)

# Toolchain: clang-cl with MSVC ABI
set(LLVM_DIR "D:/_EDWARD/Tools/clang+llvm-22.1.8-x86_64-pc-windows-msvc")
set(MSVC_DIR "D:/_EDWARD/Tools/msvc")
set(MSVC_VER "14.51.36231")
set(SDK_VER  "10.0.28000.0")

set(CMAKE_C_COMPILER   "${LLVM_DIR}/bin/clang-cl.exe")
set(CMAKE_CXX_COMPILER "${LLVM_DIR}/bin/clang-cl.exe")
set(CMAKE_LINKER        "${LLVM_DIR}/bin/lld-link.exe")
set(CMAKE_RC_COMPILER   "${LLVM_DIR}/bin/llvm-rc.exe")
set(CMAKE_AR            "${LLVM_DIR}/bin/llvm-lib.exe")
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

set(MSVC_INCLUDE "${MSVC_DIR}/VC/Tools/MSVC/${MSVC_VER}/include")
set(SDK_UCRT     "${MSVC_DIR}/Windows Kits/10/Include/${SDK_VER}/ucrt")
set(SDK_UM       "${MSVC_DIR}/Windows Kits/10/Include/${SDK_VER}/um")
set(SDK_SHARED   "${MSVC_DIR}/Windows Kits/10/Include/${SDK_VER}/shared")
set(SDK_CPPWINRT "${MSVC_DIR}/Windows Kits/10/Include/${SDK_VER}/cppwinrt")
set(MSVC_LIB     "${MSVC_DIR}/VC/Tools/MSVC/${MSVC_VER}/lib/x64")
set(SDK_UCRT_LIB "${MSVC_DIR}/Windows Kits/10/Lib/${SDK_VER}/ucrt/x64")
set(SDK_UM_LIB   "${MSVC_DIR}/Windows Kits/10/Lib/${SDK_VER}/um/x64")

set(ENV{LIB}     "${MSVC_LIB};${SDK_UCRT_LIB};${SDK_UM_LIB}")
set(ENV{INCLUDE} "${MSVC_INCLUDE};${SDK_UCRT};${SDK_UM};${SDK_SHARED}")

string(CONCAT _CLANG_FLAGS
    " -imsvc\"${MSVC_INCLUDE}\""
    " -imsvc\"${SDK_UCRT}\""
    " -imsvc\"${SDK_UM}\""
    " -imsvc\"${SDK_SHARED}\""
    " -imsvc\"${SDK_CPPWINRT}\""
)
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${_CLANG_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_CLANG_FLAGS} /EHsc")

string(CONCAT _LINK_FLAGS
    " /LIBPATH:\"${MSVC_LIB}\""
    " /LIBPATH:\"${SDK_UCRT_LIB}\""
    " /LIBPATH:\"${SDK_UM_LIB}\""
)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${_LINK_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} ${_LINK_FLAGS}")

project(eduhacksss LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Package manager
include(cmake/CPM.cmake)

# Dependencies
set(MINHOOK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/minhook)
set(MINHOOK_SOURCES
    ${MINHOOK_DIR}/src/buffer.c
    ${MINHOOK_DIR}/src/hook.c
    ${MINHOOK_DIR}/src/trampoline.c
    ${MINHOOK_DIR}/src/hde/hde64.c
)

# MinHook as a proper CMake target (required by kiero)
add_library(minhook STATIC
    ${MINHOOK_DIR}/src/buffer.c
    ${MINHOOK_DIR}/src/hook.c
    ${MINHOOK_DIR}/src/trampoline.c
    ${MINHOOK_DIR}/src/hde/hde64.c
)
target_include_directories(minhook PUBLIC ${MINHOOK_DIR}/include)
add_library(minhook::minhook ALIAS minhook)

CPMAddPackage("gh:BasedInc/libhat@0.9.0")
CPMAddPackage("gh:skypjack/entt#f931687ff04d435871ac9664bb299f71f2a8fafc")

# ImGui (vendored sources)
CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    VERSION 1.91.8
    DOWNLOAD_ONLY YES
)

if(imgui_ADDED)
    set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    )
endif()

# Kiero (swap chain discovery)
CPMAddPackage(
    NAME kiero
    GITHUB_REPOSITORY BasedInc/kiero
    GIT_TAG 269860311ac04b0ee89e68f68bca55abd02a3390
    OPTIONS
        "KIERO_INCLUDE_D3D11 ON"
        "KIERO_INCLUDE_D3D12 ON"
        "KIERO_BUILD_MINHOOK ON"
        "KIERO_MINHOOK_EXTERNAL ON"
)

# Our sources
set(EDU_SOURCES
    src/Core/Entry.cpp
    src/Client/ClientStore.cpp
    src/Features/Hooks/ClientInstanceUpdate.cpp
    src/Features/Fly.cpp
    src/Features/CoordsDisplay.cpp
    src/Rendering/SwapChainHook.cpp
    src/GUI/ClickGUI.cpp
    src/GUI/Notifications.cpp
)

add_library(eduhacksss SHARED
    ${EDU_SOURCES}
    ${IMGUI_SOURCES}
)

target_include_directories(eduhacksss PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${MINHOOK_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(eduhacksss PRIVATE
    libhat::libhat
    EnTT::EnTT
    minhook
    d3d11.lib
    dxgi.lib
)

if(TARGET kiero::kiero-minhook)
    target_link_libraries(eduhacksss PRIVATE kiero::kiero-minhook)
elseif(TARGET kiero::kiero)
    target_link_libraries(eduhacksss PRIVATE kiero::kiero)
endif()

target_compile_options(eduhacksss PRIVATE
    /W3
    /EHsc
    -Wno-unused-parameter
    -fms-compatibility
    -fms-compatibility-version=19.51
)

set_target_properties(eduhacksss PROPERTIES
    PREFIX ""
    OUTPUT_NAME "eduhacksss"
)
