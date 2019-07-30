#!/bin/bash

pip3 install -U platformio
platformio update
platformio lib -g install 415  # Blynk
platformio lib -g install https://github.com/homieiot/homie-esp8266.git#develop-v3  # Homie

lines=$(find ./examples/ -maxdepth 1 -mindepth 1 -type d)
retval=0
while read line; do
  if [[ -e "$line/platformio.ini" ]]
  then
    platformio ci --lib="." --project-conf="$line/platformio.ini" $line
    if [ $? -ne 0 ]; then
      retval=1
    fi
  else
    platformio ci --lib="." --board=d1_mini $line
    if [ $? -ne 0 ]; then
      retval=1
    fi
  fi
done <<< "$lines"
exit $retval
