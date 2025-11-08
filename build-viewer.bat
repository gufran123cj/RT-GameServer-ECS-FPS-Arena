@echo off
echo ========================================
echo Mini Game Viewer Build Script
echo ========================================
echo.

REM MinGW path kontrol
set MINGW_PATH=D:\MinGW\bin
if not exist "%MINGW_PATH%\g++.exe" (
    echo HATA: g++ bulunamadi!
    echo MinGW path'ini kontrol edin: %MINGW_PATH%
    pause
    exit /b 1
)

echo Viewer derleme basladi...
echo.

REM Eğer MiniGameViewer.exe çalışıyorsa kapat
taskkill /F /IM MiniGameViewer.exe >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo MiniGameViewer.exe kapatildi.
    timeout /t 1 /nobreak >nul
)

REM Mini Game Viewer'ı derle
"%MINGW_PATH%\g++.exe" ^
    -std=c++17 ^
    -O3 ^
    -Wall ^
    -Wextra ^
    -D_MINGW_EXTENSION=__extension__ ^
    -D__USE_MINGW_ANSI_STDIO=1 ^
    -U__STRICT_ANSI__ ^
    -fpermissive ^
    -Wno-unknown-pragmas ^
    -I. ^
    -Iinclude ^
    -Iecs ^
    -Inet ^
    -Iphysics ^
    -Imatchmaker ^
    -Ianti-cheat-lite ^
    -Icomponents ^
    src\MiniGameViewer.cpp ^
    net\Socket.cpp ^
    -o MiniGameViewer.exe ^
    -lws2_32

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Viewer derleme BASARILI!
    echo ========================================
    echo.
    echo Calistirmak icin: MiniGameViewer.exe
    echo Veya: MiniGameViewer.exe 127.0.0.1 7777
    echo.
) else (
    echo.
    echo ========================================
    echo Viewer derleme BASARISIZ!
    echo ========================================
    echo.
)

pause

