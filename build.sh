#!/bin/sh

command -v cmake > /dev/null || { echo >&2 "To build kostak we need CMake. Aborting"; exit 1; }
command -v gcc > /dev/null || { echo >&2 "You don't even have compiler. Please install gcc"; exit 1; }

EXTRA_DEFS=""

function do_build() {
  mkdir -p build/{bin,lib} && cd build
  if command -v ninja > /dev/null; then
    echo >&2 "Ninja found, using ninja build tools"
    cmake ${EXTRA_DEFS} -G "Ninja" .. && ninja 
  else
    command -v make > /dev/null || { echo >&2 "Please install make. Aborting"; exit 1; }
    cmake ${EXTRA_DEFS} -G "Unix Makefiles" .. && make
  fi
  cd ..
}

case $1 in
  clean)
    rm -rf build/*
    ;;
  static)
    EXTRA_DEFS="-DKOSTAK_STATIC=1"
    do_build
    ;;
  *)
    do_build
esac
