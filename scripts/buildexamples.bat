platformio ci --lib="." --board=d1_mini examples/basic/basic.ino
platformio ci --lib="." --board=lolin32 examples/basic-esp32/basic-esp32.ino
platformio ci --lib="." --board=d1_mini examples/datapoints/datapoints.ino
platformio ci --lib="." --board=d1_mini examples/rawOptolink/rawOptolink.ino
platformio ci --lib="." --board=d1_mini examples/readDatapoint/readDatapoint.ino
platformio ci --lib="." --project-option="lib_deps=415" --board=d1_mini examples/Blynk/Blynk.ino