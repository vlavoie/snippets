#!/bin/bash
set -e

cd $(dirname $0)/../..

mkdir -p build

clang++ $FLAGS -std=c++14 -o build/model_d -Iinclude -Wall -lX11 -lGL -g \
  examples/model/main.cc                                                 \
  include/model/wavefront.cc
