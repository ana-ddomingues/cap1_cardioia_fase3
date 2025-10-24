@echo off
setlocal enabledelayedexpansion
REM Start CardioIA dev services (compile firmware + Node-RED) on Windows

REM 1) Go to repo root
pushd "%~dp0"

REM 2) Compile firmware for Wokwi (if Arduino CLI portable is present)
if exist "tools\arduino-cli\arduino-cli.exe" (
  if not exist "tmp_sketch" mkdir "tmp_sketch" >nul 2>&1
  copy /Y "src\sketch.ino" "tmp_sketch\tmp_sketch.ino" >nul 2>&1
  echo [1/2] Compiling ESP32 firmware with Arduino CLI...
  "tools\arduino-cli\arduino-cli.exe" compile --fqbn esp32:esp32:esp32doit-devkit-v1 "%CD%\tmp_sketch" --build-path "%CD%\build" --warnings all
) else (
  echo [skip] Arduino CLI not found at tools\arduino-cli\arduino-cli.exe
  echo       Skipping firmware compile. The last build in .\build will be used by Wokwi.
)

REM 3) Start Node-RED in a new window
echo [2/2] Starting Node-RED server...
start "Node-RED" node-red

REM 4) Open Node-RED Dashboard
start "Node-RED Dashboard" "http://127.0.0.1:1880/ui"

REM 5) Instructions for Wokwi
echo.
echo Next step: In VS Code press Ctrl+Shift+P, then run:  Wokwi: Start Simulator

echo Done.

popd
endlocal
