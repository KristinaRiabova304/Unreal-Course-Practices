@echo off
REM ============================================================
REM  Week 09 - Path B (Listen Server)
REM  Joins a listen-server host by IP:port (direct connection).
REM  IMPORTANT: run this from the folder that has Multi.exe
REM  (Packaged\Windows\). Copy this .bat there first.
REM ============================================================
REM  Usage:
REM    join_client.bat 192.168.1.50:7777    (LAN example)
REM    join_client.bat 203.0.113.10:7777    (public IP for cross-network bonus)
REM    join_client.bat                       (defaults to 127.0.0.1:7777)
REM  Alternative: run Multi.exe, press ~ and type:  open 192.168.x.x:7777
REM ============================================================
if not exist "%~dp0Multi.exe" (
  echo.
  echo  ERROR: Multi.exe was not found next to this script.
  echo  Copy this .bat into the packaged folder that contains Multi.exe, e.g.:
  echo    C:\Users\User\Documents\Unreal Projects\Multi\Packaged\Windows\
  echo  and run it from THERE.
  echo.
  pause
  exit /b 1
)
set TARGET=%1
if "%TARGET%"=="" set TARGET=127.0.0.1:7777
"%~dp0Multi.exe" %TARGET% -log -nosteam
