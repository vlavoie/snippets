/*
Common header for Pulse Audio startup and shutdown code for examples
Copyright (C) 2025  Vincent Lavoie

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <common.hh>

#include <pulse/pulseaudio.h>
#include <pulse/stream.h>
#include <pulse/error.h>

#define AUDIOSTREAM_SAMPLE_RATE 44100

struct pulse_audio
{
  pa_mainloop *MainLoop;
  pa_mainloop_api *MainLoopAPI;
  pa_context *Context;
  pa_stream *Stream;
};

bool32 InitializePulseAudio(pulse_audio *PulseAudio);
void ShutdownPulseAudio(pulse_audio *PulseAudio);
