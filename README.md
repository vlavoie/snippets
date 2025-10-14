# vl/snippets

This repo contains some extracted code snippets that I use when programming. It is mostly for personal use and my own portfolio.

A lot of inspiration was drawn from [nothings/stb](https://github.com/nothings/stb), [Handmade Hero](https://hero.handmade.network/) and other sources.

## Examples

The examples folder contains implementation examples with their build scripts. I use these small CLI programs to test changes in a non-automated way for now.

* examples/audio: CLI tool to playback all WAV file passed as arguments. It will mix them and output to pulseaudio.
* examples/cartridge: CLI tool to pack files passed as arguments into an archive blob.
* examples/image: CLI tool that takes TGA files passed as arguments and places them into a texture atlas which is then rendered to an x11 window.
* examples/json: Code example to parse JSON via recursive descent and pack all data into a queryable contiguous block of memory.

## Requirements

* Clang
* C++14 or later
* X11 and OpenGL (For image example)
* Pulse Audio (For audio example)

All examples were written and built on Ubuntu 24.04.2 LTS, compabitility is not guaranteed on other platforms.
