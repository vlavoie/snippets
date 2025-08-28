/*
Example program to use mixer.hh
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
#include <pulseaudio.hh>
#include <wav.hh>

// glibc
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static pulse_audio PulseAudio;

i32 main(i32 Argc, char *Argv[])
{
  if (Argc < 3)
  {
    fprintf(stdout, "Two or more WAV files arguments are required to mix audio.\n");
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
