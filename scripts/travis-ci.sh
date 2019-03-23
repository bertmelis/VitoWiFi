#!/bin/bash

function install_pio()
{
  pip install -U platformio
  platformio update
}

function build_tests()
{
  rm -rf build
  mkdir build
  platformio ci --lib="./src" --project-conf="scripts/platformio.ini" --build-dir=build --keep-build-dir tests
}

case $1 in
  cpplint)
    pip install -U cpplint
    cpplint --repository=. --recursive --linelength=200 --filter=-build/include  ./src
    ;;
  cppcheck)
    cppcheck --error-exitcode=1 --enable=warning,style,performance,portability -DARDUINO_ARCH_ESP8266=1 ./src
    ;;
  test)
    install_pio
    build_tests
    ./build/.pioenvs/catch2/program
    ;;
  memcheck)
    install_pio
    build_tests
    valgrind --leak-check=full ./build/.pioenvs/catch2/program
    ;;
  buildexamples)
    find ./examples -type d -print0 | while IFS= read -r -d $'\0' line; do
      echo "$line";
    done
    ;;
  *)
    echo "Should not end up here"
    ;;
esac