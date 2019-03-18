@echo off
if exist .\build rmdir /s /q .\build
mkdir build
platformio ci --lib="./src" --project-conf=scripts/platformio.ini --build-dir=build --keep-build-dir tests
if exist .\build\.pioenvs\catch2\program.exe .\build\.pioenvs\catch2\program.exe
if exist .\build rmdir /s /q .\build
if exist .\-p rmdir /s /q .\-p
@echo on
