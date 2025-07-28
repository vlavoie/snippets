#!/bin/bash
set -e

cd $(dirname $0)/../..

mkdir -p build

clang++ -std=c++14 -o build/cartridge_d -Iinclude -Wall -g \
  examples/cartridge/main.cc                               \
  include/cartridge.cc
