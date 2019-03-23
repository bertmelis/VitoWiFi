platformio ci --lib="." --board=d1_mini examples/basic/basic.ino
platformio ci --lib="." --board=d1_mini examples/rawOptolink/rawOptolink.ino
platformio ci --lib="." --project-option="lib_deps=415" --board=d1_mini examples/Blynk/Blynk.ino