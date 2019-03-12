#!/bin/bash

echo "Starting Travis job"

if [[ "$CPPLINT" ]]
then
  pip install -U cpplint
  cpplint --repository=. --recursive --linelength=200 --filter=-build/include  ./src
elif [[ "$TESTQUEUE" ]]
then
  pip install -U platformio
  platformio update
  mkdir build
  platformio ci --lib="./src" --project-conf="tests/platformio.ini" --build-dir=build --keep-build-dir tests/queue.cpp
else
  pip install -U platformio
  platformio update
  platformio lib -g install 416 # Blynk
  echo "should be building examples"
  # platformio ci --lib="." $PLATFORMIO_CI_EXTRA_ARGS
fi
