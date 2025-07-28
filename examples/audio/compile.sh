#!/bin/bash
set -e

cd $(dirname $0)/../..

mkdir -p build

clang++ -std=c++14 -o build/audio_d -Iinclude -Wall -lpulse -g \
  examples/audio/main.cc                                       \
  include/riff.cc                                              \
  include/wav.cc
