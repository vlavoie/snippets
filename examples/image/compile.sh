#!/bin/bash
set -e

cd $(dirname $0)/../..

mkdir -p build

clang++ $FLAGS -std=c++14 -Iinclude -Wall -lGL `sdl2-config --cflags --libs` -g \
  examples/image/main.cc                                                        \
  include/image/tga.cc                                                          \
  -o build/image_d 
