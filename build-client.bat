@echo off
echo ========================================
echo Test Client Build Script
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

echo Client derleme basladi...
echo.

REM Eğer TestClient.exe çalışıyorsa kapat
taskkill /F /IM TestClient.exe >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo TestClient.exe kapatildi.
    timeout /t 1 /nobreak >nul
)

REM Test client'ı derle
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
    src\TestClient.cpp ^
    net\Socket.cpp ^
    -o TestClient.exe ^
    -lws2_32

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo Client derleme BASARILI!
    echo ========================================
    echo.
    echo Calistirmak icin: TestClient.exe
    echo Veya: TestClient.exe 127.0.0.1 7777
    echo.
) else (
    echo.
    echo ========================================
    echo Client derleme BASARISIZ!
    echo ========================================
    echo.
)

pause

