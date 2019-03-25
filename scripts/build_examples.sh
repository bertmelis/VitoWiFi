#!/bin/bash

#pip install -U platformio
#platformio update
#pip install -U platformio
#platformio update
#platformio lib -g install 416  # Blynk
#platformio lib -g install 305  # ESPAsyncTCP
#platformio lib -g install 1826 # AsyncTCP
#platformio lib -g install 306  # ESP Async Webserver
#platformio lib -g install 1106 # Bounce2
#platformio lib -g install 555  # Homie

find ./examples/ -maxdepth 1 -mindepth 1 -type d -print0 | while IFS= read -r -d $'\0' line; do
  if [[ -e "$line/platformio.ini" ]]
  then
    platformio ci --lib="." --project-conf="$line/platformio.ini" $line
  else
    platformio ci --lib="." --board=d1_mini $line
  fi
done
