cmake_minimum_required(VERSION 3.16)

# Toolchain: clang-cl with MSVC ABI (required for entt ABI compatibility with the game)
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
)
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${_CLANG_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_CLANG_FLAGS}")

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

# MSVC include/lib paths
set(MSVC_INCLUDE "${MSVC_DIR}/VC/Tools/MSVC/${MSVC_VER}/include")
set(SDK_UCRT     "${MSVC_DIR}/Windows Kits/10/Include/${SDK_VER}/ucrt")
set(SDK_UM       "${MSVC_DIR}/Windows Kits/10/Include/${SDK_VER}/um")
set(SDK_SHARED   "${MSVC_DIR}/Windows Kits/10/Include/${SDK_VER}/shared")

set(MSVC_LIB     "${MSVC_DIR}/VC/Tools/MSVC/${MSVC_VER}/lib/x64")
set(SDK_UCRT_LIB "${MSVC_DIR}/Windows Kits/10/Lib/${SDK_VER}/ucrt/x64")
set(SDK_UM_LIB   "${MSVC_DIR}/Windows Kits/10/Lib/${SDK_VER}/um/x64")

# CPM package manager
include(cmake/CPM.cmake)

set(MINHOOK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/minhook)
set(MINHOOK_SOURCES
    ${MINHOOK_DIR}/src/buffer.c
    ${MINHOOK_DIR}/src/hook.c
    ${MINHOOK_DIR}/src/trampoline.c
    ${MINHOOK_DIR}/src/hde/hde64.c
)

CPMAddPackage("gh:BasedInc/libhat@0.9.0")
CPMAddPackage("gh:skypjack/entt#f931687ff04d435871ac9664bb299f71f2a8fafc")

set(EDU_SOURCES
    src/Core/Entry.cpp
    src/Client/ClientStore.cpp
    src/Features/Hooks/ClientInstanceUpdate.cpp
    src/Features/Fly.cpp
)

add_library(eduhacksss SHARED
    ${EDU_SOURCES}
    ${MINHOOK_SOURCES}
)

target_include_directories(eduhacksss PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${MINHOOK_DIR}/include
)


target_link_libraries(eduhacksss PRIVATE libhat::libhat EnTT::EnTT)

target_compile_options(eduhacksss PRIVATE
    /W3
    /EHsc
    -Wno-unused-parameter
    -fms-compatibility
    -fms-compatibility-version=19.51
)

target_link_options(eduhacksss PRIVATE
    /DLL
)

set_target_properties(eduhacksss PROPERTIES
    PREFIX ""
    OUTPUT_NAME "eduhacksss"
)
