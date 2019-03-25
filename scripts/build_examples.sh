#!/bin/bash

pip install -U platformio
platformio update
pip install -U platformio
platformio update
platformio lib -g install 415  # Blynk
platformio lib -g install 305  # ESPAsyncTCP
platformio lib -g install 1826 # AsyncTCP
platformio lib -g install 306  # ESP Async Webserver
platformio lib -g install 1106 # Bounce2
platformio lib -g install https://github.com/homieiot/homie-esp8266.git#develop-v3  # Homie

retval = 0
find ./examples/ -maxdepth 1 -mindepth 1 -type d -print0 | while IFS= read -r -d $'\0' line; do
  if [[ -e "$line/platformio.ini" ]]
  then
    platformio ci --lib="." --project-conf="$line/platformio.ini" $line
    if [ $? -ne 0 ]; then
      retval = 1
    fi
  else
    platformio ci --lib="." --board=d1_mini $line
    if [ $? -ne 0 ]; then
      retval = 1
    fi
  fi
done
exit retval
