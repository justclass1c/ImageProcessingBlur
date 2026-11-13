@echo off
rem Builds the project with the CUDA implementation.
rem
rem On Windows nvcc produces MSVC objects, and MinGW's gcc cannot link those
rem (unresolved __security_cookie / __GSHandlerCheck / _Init_thread_*), so the
rem CUDA build uses MSVC (cl.exe) + nvcc for every file.
rem
rem Usage: run "build_cuda.bat" from a normal command prompt.
setlocal EnableExtensions

rem nvcc on Windows needs cl.exe from Visual Studio.
set "VS_VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VS_VCVARS%" (
    echo ERROR: vcvars64.bat not found.
    echo Install the "Desktop development with C++" workload in Visual Studio.
    exit /b 1
)
call "%VS_VCVARS%" >nul
if errorlevel 1 exit /b 1

rem Pick the newest installed CUDA toolkit (v13.5, v12.9, ...).
set "CUDA_ROOT="
for /f "delims=" %%D in ('dir /b /ad /o-n "%ProgramFiles%\NVIDIA GPU Computing Toolkit\CUDA\v*" 2^>nul') do (
    if not defined CUDA_ROOT set "CUDA_ROOT=%ProgramFiles%\NVIDIA GPU Computing Toolkit\CUDA\%%D"
)
if not defined CUDA_ROOT (
    echo ERROR: CUDA toolkit not found in "%ProgramFiles%\NVIDIA GPU Computing Toolkit\CUDA".
    exit /b 1
)
set "CUDA_PATH=%CUDA_ROOT%"
set "PATH=%CUDA_PATH%\bin;%PATH%"

pushd "%~dp0"

if exist obj rmdir /s /q obj
mkdir obj

rem /openmp builds the OpenMP implementation with MSVC's runtime (vcomp)
set "CFLAGS=/nologo /std:c11 /utf-8 /O2 /W3 /openmp /D_CRT_SECURE_NO_WARNINGS /DUSE_CUDA /I. /Iheaders"

echo [1/3] Compiling C sources with MSVC...
cl %CFLAGS% /c main.c /Foobj\main.obj
if errorlevel 1 goto :fail
cl %CFLAGS% /c src\image.c /Foobj\image.obj
if errorlevel 1 goto :fail
cl %CFLAGS% /c src\sequential_blur.c /Foobj\sequential_blur.obj
if errorlevel 1 goto :fail
cl %CFLAGS% /c src\blur.c /Foobj\blur.obj
if errorlevel 1 goto :fail
cl %CFLAGS% /c src\tui.c /Foobj\tui.obj
if errorlevel 1 goto :fail
cl %CFLAGS% /c src\openmp_blur.c /Foobj\openmp_blur.obj
if errorlevel 1 goto :fail
cl %CFLAGS% /c src\metrics.c /Foobj\metrics.obj
if errorlevel 1 goto :fail

echo [2/3] Compiling CUDA kernel with nvcc...
nvcc -O2 -arch=native -Xcompiler "/wd4819" -I. -Iheaders -c src\cuda_blur.cu -o obj\cuda_blur.obj
if errorlevel 1 goto :fail

echo [3/3] Linking main.exe...

rem Windows keeps main.exe locked while it is running, so link.exe fails with
rem LNK1104. Check that first, then retry once in case antivirus locked it.
tasklist /FI "IMAGENAME eq main.exe" 2>nul | find /I "main.exe" >nul
if not errorlevel 1 (
    echo ERROR: main.exe is still running.
    echo Close it ^(or stop the debugger^) and run this script again.
    goto :fail
)

call :link
if errorlevel 1 (
    echo Link failed, retrying in 2 seconds...
    timeout /t 2 /nobreak >nul
    call :link
)
if errorlevel 1 goto :fail

echo.
echo Build finished: main.exe  (choose implementation 4 = CUDA)
popd
exit /b 0

:link
rem -cudart=static embeds the CUDA runtime, so main.exe runs without extra DLLs
nvcc -O2 -cudart=static obj\main.obj obj\image.obj obj\sequential_blur.obj obj\blur.obj obj\tui.obj obj\openmp_blur.obj obj\metrics.obj obj\cuda_blur.obj vcomp.lib -o main.exe
exit /b %errorlevel%

:fail
popd
echo.
echo Build failed.
exit /b 1
