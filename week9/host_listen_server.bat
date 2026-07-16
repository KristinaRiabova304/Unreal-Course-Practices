@echo off
REM ============================================================
REM  Week 09 - Path B (Listen Server)
REM  Launches Multi as a LISTEN-SERVER host on UDP port 7777.
REM  IMPORTANT: run this from the folder that has Multi.exe
REM  (Packaged\Windows\). Copy this .bat there first.
REM ============================================================
REM  Arguments:
REM    /Game/ThirdPerson/Lvl_ThirdPerson  - map to open
REM    ?listen                            - become a listen server (renders + hosts)
REM    ?port=7777                         - UDP port to listen on
REM    -log                               - console log window
REM    -nosteam                           - direct IP networking (Path B), skip Steam
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
"%~dp0Multi.exe" /Game/ThirdPerson/Lvl_ThirdPerson?listen?port=7777 -log -nosteam
