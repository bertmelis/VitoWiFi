#!/bin/bash

echo "Starting Travis job"

if [[ "$CPPLINT" ]]
then
  cpplint --repository=. --recursive --linelength=200 --filter=-build/include  ./src
elif [[ "$TESTQUEUE" ]]
then
  platformio ci --lib="." --project-conf=platformio.ini --build-dir=build --keep-build-dir ../tests/queue.cpp
else
  echo "should be building examples"
  # platformio ci --lib="." $PLATFORMIO_CI_EXTRA_ARGS
fi
