#!/bin/bash
set -e

cd $(dirname $0)/../..

mkdir -p build

clang++ -std=c++14 -o build/mixer_d -Iinclude -Iexamples/common -Wall -lpulse -g \
  examples/mixer/main.cc                                                         \
  examples/common/pulseaudio.cc                                                  \
  include/riff.cc                                                                \
  include/wav.cc
