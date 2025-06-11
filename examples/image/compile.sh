#!/bin/bash
set -e

cd $(dirname $0)/../..

mkdir -p build

clang++ $FLAGS -std=c++14 -o build/image_d -Iinclude -Wall -lX11 -lGL -g \
  examples/image/main.cc                                                       \
  include/image/tga.cc
