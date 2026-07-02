@echo off
REM ---------------------------------------------------------------------------
REM  eduhacksss build script - MinGW g++ (no admin, no CMake required)
REM
REM  Builds the injected DLL, static-linking libgcc/libstdc++ and MinHook so
REM  the artifact has zero external runtime dependencies.
REM ---------------------------------------------------------------------------
setlocal enabledelayedexpansion

set ROOT=%~dp0
set SRC=%ROOT%src
set MH=%ROOT%third_party\minhook
set OUT=%ROOT%build
if not exist %OUT% mkdir %OUT%

set CXX=g++
set WARN=-Wall -Wextra -Wno-unused-parameter
set CXXFLAGS=-std=c++17 -mthreads -DUNICODE -D_UNICODE -O2 %WARN%
set CFLAGS=-std=c11 -mthreads -O2 %WARN%
set INCLUDES=-I%SRC% -I%MH%\include
set LDFLAGS=-mthreads -static -static-libgcc -static-libstdc++ -shared

REM obj(name, srcfile) macro: compile src -> obj, append to OBJS
set OBJS=

echo [1/3] Compiling eduhacksss sources...
call :cxx_src Core_Entry           "%SRC%\Core\Entry.cpp"                     || goto :err
call :cxx_src Memory_SigScanner    "%SRC%\Memory\SigScanner.cpp"             || goto :err
call :cxx_src Client_ClientStore   "%SRC%\Client\ClientStore.cpp"            || goto :err
call :cxx_src Hooks_ClientUpdate   "%SRC%\Features\Hooks\ClientInstanceUpdate.cpp" || goto :err

echo [2/3] Compiling MinHook sources...
call :c_src   MH_buffer            "%MH%\src\buffer.c"                       || goto :err
call :c_src   MH_hook              "%MH%\src\hook.c"                         || goto :err
call :c_src   MH_trampoline        "%MH%\src\trampoline.c"                   || goto :err
call :c_src   MH_hde64             "%MH%\src\hde\hde64.c"                    || goto :err

echo [3/3] Linking eduhacksss.dll...
%CXX% %LDFLAGS% %OBJS% -o "%OUT%\eduhacksss.dll" || goto :err

echo.
echo BUILD OK -^> %OUT%\eduhacksss.dll
exit /b 0

REM --- helpers ---------------------------------------------------------------
:cxx_src
echo   ^> %~n2
%CXX% %CXXFLAGS% %INCLUDES% -c "%~2" -o "%OUT%\%~1.o" || exit /b 1
set OBJS=!OBJS! "%OUT%\%~1.o"
exit /b 0

:c_src
echo   ^> %~n2
%CXX% %CFLAGS% %INCLUDES% -c "%~2" -o "%OUT%\%~1.o" || exit /b 1
set OBJS=!OBJS! "%OUT%\%~1.o"
exit /b 0

:err
echo.
echo BUILD FAILED.
exit /b 1
