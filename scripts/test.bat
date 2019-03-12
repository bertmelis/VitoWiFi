@echo off
echo Cleaning up previous test
if exist .\build rmdir /s /q .\build
echo Compiling
mkdir build
platformio ci --lib="./src" --project-conf=tests/platformio.ini --build-dir=build --keep-build-dir tests/queue.cpp
@echo Starting tests
if exist .\build\.pioenvs\test_queue\program.exe .\build\.pioenvs\test_queue\program.exe
@echo Cleaning up
if exist .\build rmdir /s /q .\build
if exist .\-p rmdir /s /q .\-p
@echo on
