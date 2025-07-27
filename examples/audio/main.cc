/*
Example program to use wav.hh
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

#include <common.hh>
#include <wav.hh>

// glibc
#include <stdio.h>
#include <stdlib.h>

// pulse audio
#include <pulse/error.h>
#include <pulse/pulseaudio.h>
#include <pulse/stream.h>

#define AUDIOSTREAM_COUNT 1

// pulse audio variables
struct pulse_audio
{
  pa_mainloop *MainLoop;
  pa_mainloop_api *MainLoopAPI;
  pa_context *Context;
  pa_stream *Streams[AUDIOSTREAM_COUNT];
};

static pulse_audio PulseAudio;

bool32 InitializePulseAudio()
{
  fprintf(stdout, "Initializing PulseAudio.\n");

  PulseAudio.MainLoop = pa_mainloop_new();
  if (PulseAudio.MainLoop)
  {
    fprintf(stdout, "\tSuccessfully allocated PulseAudio mainloop.\n");
  }
  else
  {
    fprintf(stderr, "\tFailed to allocate PulseAudio mainloop.\n");
    return false;
  }

  PulseAudio.MainLoopAPI = pa_mainloop_get_api(PulseAudio.MainLoop);
  PulseAudio.Context = pa_context_new(PulseAudio.MainLoopAPI, "Sharktooth");
  if (PulseAudio.Context)
  {
    fprintf(stdout, "\tSuccessfully allocated PulseAudio context.\n");
  }
  else
  {
    fprintf(stderr, "\tFailed to allocate PulseAudio context.\n");
    return false;
  }

  i32 ContextError = pa_context_connect(PulseAudio.Context, NULL, PA_CONTEXT_NOFLAGS, NULL);
  if (ContextError < 0)
  {
    fprintf(stderr, "\tFailed to connect PulseAudio context: %s\n", pa_strerror(ContextError));
    return false;
  }

  pa_context_state_t PreviousContextState;
  pa_context_state_t ContextState = pa_context_get_state(PulseAudio.Context);
  while ((ContextState = pa_context_get_state(PulseAudio.Context)) != PA_CONTEXT_READY)
  {
    if (PreviousContextState != ContextState)
    {
      switch (ContextState)
      {
      case PA_CONTEXT_UNCONNECTED:
      case PA_CONTEXT_CONNECTING:
        fprintf(stdout, "\tConnecting PulseAudio context...\n");
        break;
      case PA_CONTEXT_AUTHORIZING:
        fprintf(stdout, "\tAuthorizing PulseAudio context...\n");
        break;
      case PA_CONTEXT_SETTING_NAME:
        fprintf(stdout, "\tSetting name of PulseAudio context...\n");
        break;
      case PA_CONTEXT_FAILED:
      case PA_CONTEXT_TERMINATED:
        fprintf(stderr, "\tFailed connecting PulseAudio context.\n");
        return false;
      default:
        break;
      }
    }
    i32 MainLoopValue;
    pa_mainloop_iterate(PulseAudio.MainLoop, 0, &MainLoopValue);
    PreviousContextState = ContextState;
  }
  fprintf(stdout, "\tSuccessfully connected PulseAudio context.\n");

  pa_sample_spec SampleSpec;
  SampleSpec.format = PA_SAMPLE_S16NE;
  SampleSpec.channels = 1;
  SampleSpec.rate = 44100;

  pa_channel_map ChannelMap;
  pa_channel_map_init_mono(&ChannelMap);

  for (key StreamIndex = 0; StreamIndex < AUDIOSTREAM_COUNT; StreamIndex++)
  {
    PulseAudio.Streams[StreamIndex] =
        pa_stream_new(PulseAudio.Context, "Music", &SampleSpec, &ChannelMap);

    if (PulseAudio.Streams[StreamIndex])
    {
      fprintf(stdout, "\tSuccessfully allocated PulseAudio stream %zu.\n", StreamIndex);
    }
    else
    {
      fprintf(stderr, "\tFailed to allocate PulseAudio stream %zu\n", StreamIndex);
      return false;
    }

    pa_buffer_attr BufferAttributes;
    BufferAttributes.maxlength = (uint32_t)-1;
    BufferAttributes.tlength = (uint32_t)-1;
    BufferAttributes.prebuf = (uint32_t)-1;
    BufferAttributes.minreq = (uint32_t)-1;
    BufferAttributes.fragsize = (uint32_t)-1;

    i32 StreamError = pa_stream_connect_playback(PulseAudio.Streams[StreamIndex], NULL,
                                                 &BufferAttributes, PA_STREAM_NOFLAGS, NULL, NULL);

    if (StreamError)
    {
      fprintf(stderr, "\tFailed to connect PulseAudio stream to playback: %s\n",
              pa_strerror(StreamError));
      return false;
    }

    pa_stream_state_t PreviousStreamState;
    pa_stream_state_t StreamState;
    while ((StreamState = pa_stream_get_state(PulseAudio.Streams[StreamIndex])) != PA_STREAM_READY)
    {
      if (PreviousStreamState != StreamState)
      {
        switch (StreamState)
        {
        case PA_STREAM_UNCONNECTED:
        case PA_STREAM_CREATING:
          fprintf(stdout, "\tConnecting PulseAudio stream %zu...\n", StreamIndex);
          break;
        case PA_STREAM_FAILED:
        case PA_STREAM_TERMINATED:
          fprintf(stderr, "\tFailed connecting PulseAudio stream %zu.\n", StreamIndex);
          return false;
        default:
          break;
        }
      }
      i32 MainLoopValue;
      pa_mainloop_iterate(PulseAudio.MainLoop, 0, &MainLoopValue);
      PreviousStreamState = StreamState;
    }
    fprintf(stdout, "\tSuccessfully connected PulseAudio stream %zu.\n", StreamIndex);
  }

  return true;
}

void ShutdownPulseAudio()
{
  fprintf(stdout, "Shutting down PulseAudio.\n");
  // clear leftover PulseAudio playback
  fprintf(stdout, "\tFlushing PulseAudio streams.\n");
  for (key i = 0; i < AUDIOSTREAM_COUNT; i++)
  {
    fprintf(stdout, "\tFlushing PulseAudio stream %zu.\n", i);
    pa_stream_flush(PulseAudio.Streams[i], NULL, NULL);
    pa_stream_disconnect(PulseAudio.Streams[i]);
  }
  int r;
  pa_mainloop_iterate(PulseAudio.MainLoop, 0, &r);

  fprintf(stdout, "\tFreeing PulseAudio mainloop.\n");
  pa_context_disconnect(PulseAudio.Context);
  pa_mainloop_quit(PulseAudio.MainLoop, 0);
  pa_mainloop_free(PulseAudio.MainLoop);
}

i32 main(i32 Argc, char *Argv[])
{
  if (Argc < 2)
  {
    fprintf(stderr, "WAV file argument is required to play audio.");
    return 1;
  }

  InitializePulseAudio();

  ShutdownPulseAudio();
}
