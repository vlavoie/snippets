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
#include <mixer.hh>
#include <wav.hh>

// glibc
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// pulseaudio
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

static pulse_audio PulseAudio;

bool32 InitializePulseAudio(pulse_audio *PulseAudio)
{
  fprintf(stdout, "Initializing PulseAudio->\n");

  PulseAudio->MainLoop = pa_mainloop_new();
  if (PulseAudio->MainLoop)
  {
    fprintf(stdout, "Successfully allocated PulseAudio mainloop.\n");
  }
  else
  {
    fprintf(stdout, "Failed to allocate PulseAudio mainloop.\n");
    return false;
  }

  PulseAudio->MainLoopAPI = pa_mainloop_get_api(PulseAudio->MainLoop);
  PulseAudio->Context = pa_context_new(PulseAudio->MainLoopAPI, "Sharktooth");
  if (PulseAudio->Context)
  {
    fprintf(stdout, "Successfully allocated PulseAudio context.\n");
  }
  else
  {
    fprintf(stdout, "Failed to allocate PulseAudio context.\n");
    return false;
  }

  i32 ContextError = pa_context_connect(PulseAudio->Context, NULL, PA_CONTEXT_NOFLAGS, NULL);
  if (ContextError < 0)
  {
    fprintf(stdout, "Failed to connect PulseAudio context: %s\n", pa_strerror(ContextError));
    return false;
  }

  pa_context_state_t PreviousContextState;
  pa_context_state_t ContextState = pa_context_get_state(PulseAudio->Context);
  while ((ContextState = pa_context_get_state(PulseAudio->Context)) != PA_CONTEXT_READY)
  {
    if (PreviousContextState != ContextState)
    {
      switch (ContextState)
      {
      case PA_CONTEXT_UNCONNECTED:
      case PA_CONTEXT_CONNECTING:
        fprintf(stdout, "Connecting PulseAudio context...\n");
        break;
      case PA_CONTEXT_AUTHORIZING:
        fprintf(stdout, "Authorizing PulseAudio context...\n");
        break;
      case PA_CONTEXT_SETTING_NAME:
        fprintf(stdout, "Setting name of PulseAudio context...\n");
        break;
      case PA_CONTEXT_FAILED:
      case PA_CONTEXT_TERMINATED:
        fprintf(stdout, "Failed connecting PulseAudio context.\n");
        return false;
      default:
        break;
      }
    }
    i32 MainLoopValue;
    pa_mainloop_iterate(PulseAudio->MainLoop, 0, &MainLoopValue);
    PreviousContextState = ContextState;
  }
  fprintf(stdout, "Successfully connected PulseAudio context.\n");

  pa_sample_spec SampleSpec;
  SampleSpec.format = PA_SAMPLE_S16NE;
  SampleSpec.channels = 1;
  SampleSpec.rate = AUDIOSTREAM_SAMPLE_RATE;

  pa_channel_map ChannelMap;
  pa_channel_map_init_mono(&ChannelMap);

  PulseAudio->Stream = pa_stream_new(PulseAudio->Context, "Music", &SampleSpec, &ChannelMap);

  if (PulseAudio->Stream)
  {
    fprintf(stdout, "Successfully allocated PulseAudio stream.\n");
  }
  else
  {
    fprintf(stdout, "Failed to allocate PulseAudio stream\n");
    return false;
  }

  pa_buffer_attr BufferAttributes;
  BufferAttributes.maxlength = (uint32_t)-1;
  BufferAttributes.tlength = (uint32_t)-1;
  BufferAttributes.prebuf = (uint32_t)-1;
  BufferAttributes.minreq = (uint32_t)-1;
  BufferAttributes.fragsize = (uint32_t)-1;

  i32 StreamError = pa_stream_connect_playback(PulseAudio->Stream, NULL, &BufferAttributes,
                                               PA_STREAM_NOFLAGS, NULL, NULL);

  if (StreamError)
  {
    fprintf(stdout, "Failed to connect PulseAudio stream to playback: %s\n",
            pa_strerror(StreamError));
    return false;
  }

  pa_stream_state_t PreviousStreamState;
  pa_stream_state_t StreamState;
  while ((StreamState = pa_stream_get_state(PulseAudio->Stream)) != PA_STREAM_READY)
  {
    if (PreviousStreamState != StreamState)
    {
      switch (StreamState)
      {
      case PA_STREAM_UNCONNECTED:
      case PA_STREAM_CREATING:
        fprintf(stdout, "Connecting PulseAudio stream...\n");
        break;
      case PA_STREAM_FAILED:
      case PA_STREAM_TERMINATED:
        fprintf(stdout, "Failed connecting PulseAudio stream.\n");
        return false;
      default:
        break;
      }
    }
    i32 MainLoopValue;
    pa_mainloop_iterate(PulseAudio->MainLoop, 0, &MainLoopValue);
    PreviousStreamState = StreamState;
  }
  fprintf(stdout, "Successfully connected PulseAudio stream.\n");

  return true;
}

void ShutdownPulseAudio(pulse_audio *PulseAudio)
{
  fprintf(stdout, "Shutting down PulseAudio.\n");
  // clear leftover PulseAudio playback
  fprintf(stdout, "Flushing PulseAudio streams.\n");
  fprintf(stdout, "Flushing PulseAudio stream.\n");
  pa_stream_flush(PulseAudio->Stream, NULL, NULL);
  pa_stream_disconnect(PulseAudio->Stream);

  i32 ReturnCode;
  pa_mainloop_iterate(PulseAudio->MainLoop, 0, &ReturnCode);

  fprintf(stdout, "Freeing PulseAudio mainloop.\n");
  pa_context_disconnect(PulseAudio->Context);
  pa_mainloop_quit(PulseAudio->MainLoop, 0);
  pa_mainloop_free(PulseAudio->MainLoop);
}

i32 main(i32 Argc, char *Argv[])
{
  if (Argc < 2)
  {
    fprintf(stdout, "At least one WAV file argument is required to output audio.\n");
    return 1;
  }

  key AudioCount = Argc - 1;
  wav::audio *Audios = SysAllocate(wav::audio, AudioCount);
  key LongestAudio = 0;

  for (key AudioIndex = 0; AudioIndex < AudioCount; AudioIndex++)
  {
    FILE *File = fopen(Argv[AudioIndex + 1], "r");
    key WavLength = 0;
    void *WavData = 0x0;

    if (File)
    {
      fseek(File, 0, SEEK_END);
      WavLength = ftell(File);
      WavData = SysAllocate(byte, WavLength);
      rewind(File);
      fread(WavData, 1, WavLength, File);

      fclose(File);
    }
    else
    {
      fprintf(stdout, "Failed to read WAV file.\n");
      return 1;
    }

    Audios[AudioIndex] = wav::GetAudio(WavLength, WavData);

    if (Audios[AudioIndex].SampleCount == 0)
    {
      fprintf(stdout, "Failed to parse WAV file '%s'.\n", Argv[AudioIndex + 1]);
      return 1;
    }

    LongestAudio = Audios[AudioIndex].SampleCount > LongestAudio ? Audios[AudioIndex].SampleCount
                                                                 : LongestAudio;
  }

  if (InitializePulseAudio(&PulseAudio) == 0)
  {
    fprintf(stdout, "Failed to initialize PulseAudio.\n");
    return 1;
  }

  wav::sample *StreamBuffer;

  key WriteableBytes = pa_stream_writable_size(PulseAudio.Stream);
  key ChunkLength = WriteableBytes;
  key Offset = 0;
  key StreamBufferLength = LongestAudio * sizeof(wav::sample);

  i32 PulseAudioError;

  while (Offset * sizeof(wav::sample) < StreamBufferLength)
  {
    key BytesLeft = StreamBufferLength - Offset * sizeof(wav::sample);

    if (WriteableBytes > 0)
    {
      ChunkLength = ChunkLength < BytesLeft ? ChunkLength : BytesLeft;
      PulseAudioError =
          pa_stream_begin_write(PulseAudio.Stream, (void **)&StreamBuffer, &ChunkLength);

      if (PulseAudioError < 0)
      {
        fprintf(stdout, "Failed to begin write to PulseAudio stream: %s\n",
                pa_strerror(PulseAudioError));
      }
      else
      {
        if (ChunkLength < WriteableBytes)
        {
          fprintf(stdout, "Expected %lu bytes for stream, got %lu, producing %lu samples\n",
                  WriteableBytes, ChunkLength, ChunkLength / sizeof(wav::sample));
        }

        for (key SampleIndex = 0; SampleIndex < ChunkLength / sizeof(wav::sample); SampleIndex++)
        {
          mixer::sample MixedSample = 0;
          for (key MixerIndex = 0; MixerIndex < AudioCount; MixerIndex++)
          {
            if (Audios[MixerIndex].SampleCount > Offset)
            {
              MixedSample = mixer::MixSamples(MixedSample,
                                              Audios[MixerIndex].SampleData[SampleIndex + Offset]);
            }
          }

          StreamBuffer[SampleIndex] = MixedSample;
        }

        if (ChunkLength > 0)
        {
          PulseAudioError = pa_stream_write(PulseAudio.Stream, StreamBuffer, ChunkLength, NULL, 0,
                                            PA_SEEK_RELATIVE);

          // this is a workaround for a weird issue where short chunks/sounds are not sent to
          // pulseaudio server, better than the alternative of sending continuous mute audio
          if (StreamBufferLength < ChunkLength)
          {
            pa_stream_drain(PulseAudio.Stream, NULL, NULL);
          }

          if (PulseAudioError < 0)
          {
            fprintf(stdout, "Failed to write to PulseAudio stream: %s\n",
                    pa_strerror(PulseAudioError));
          }
        }
        else
        {
          pa_stream_cancel_write(PulseAudio.Stream);
        }
      }
    }

    pa_mainloop_iterate(PulseAudio.MainLoop, 0, &PulseAudioError);
    Offset += ChunkLength / sizeof(wav::sample);
    key SleepTime = ChunkLength * 1000 / sizeof(wav::sample) / AUDIOSTREAM_SAMPLE_RATE;
    SleepTime = SleepTime > 100
                    ? SleepTime - 100
                    : SleepTime; // 100 is an arbitraty value to prevent gaps in the audio
    pa_msleep(SleepTime);
  }

  ShutdownPulseAudio(&PulseAudio);
}
