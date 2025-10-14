#!/bin/bash
set -e

cd $(dirname $0)/../..

mkdir -p build

clang++ -std=c++14 -o build/json_d -Iinclude -Wall -g \
  examples/json/main.cc \
  include/allocators/bump.cc \
  include/allocators/fake.cc \
