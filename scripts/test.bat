@echo off
if exist .\build rmdir /s /q .\build
mkdir build
platformio ci --lib="./src" --project-conf=scripts/platformio.ini --build-dir=build --keep-build-dir tests/queue.cpp
if exist .\build\.pioenvs\test_queue\program.exe .\build\.pioenvs\test_queue\program.exe
if exist .\build rmdir /s /q .\build
if exist .\-p rmdir /s /q .\-p
@echo on

@echo off
if exist .\build rmdir /s /q .\build
mkdir build
platformio ci --lib="./src" --project-conf=scripts/platformio.ini --build-dir=build --keep-build-dir tests/datapoints.cpp
if exist .\build\.pioenvs\test_queue\program.exe .\build\.pioenvs\test_queue\program.exe
if exist .\build rmdir /s /q .\build
if exist .\-p rmdir /s /q .\-p
@echo on
