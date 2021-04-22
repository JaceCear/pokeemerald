#include "global.h"
#include "gba/types.h"
#include "gba/io_reg.h"

#include "gb_audio.h"
#include "sound_mixer.h"

typedef s32 AudioSample; // update to float once converted
#define GB_TONE_CHANNEL_COUNT 2 // Gameboy channels using the "Tone" feature
#define AUDIO_CHANNEL_COUNT   2 // Number of mono/stereo channels
#define MAX_VOLUME 0x3FFFFFFF; // 1.0f once mixer is float

static float GbWaveDuty[4] = { 1.0f / 8.0f,
                               1.0f / 4.0f,
                               1.0f / 2.0f,
                               3.0f / 4.0f };

static float ChannelVolumeTable[4] =
{
    [SOUND_CGB_MIX_QUARTER] = (1.0f / 4.0f),
    [SOUND_CGB_MIX_HALF]    = (1.0f / 2.0f),
    [SOUND_CGB_MIX_FULL]    = (1.0f),
    [3] = 0.0f, // "Prohibited" according to GBATEK
};

static s32 currentSampleIndex[GB_TONE_CHANNEL_COUNT][AUDIO_CHANNEL_COUNT] = { 0 };
static s32 waveStart[GB_TONE_CHANNEL_COUNT][AUDIO_CHANNEL_COUNT] = { 0 };
static AudioSample prevSampleValue[GB_TONE_CHANNEL_COUNT][AUDIO_CHANNEL_COUNT] = { 0 };

void
GbTest(s32* outBuffer, s32 sampleRate, u16 samplesPerFrame)
{
    // TODO: Implement channel 1 sweeps
    for (int gbChannel = 0; gbChannel < GB_TONE_CHANNEL_COUNT; gbChannel++)
    {
        u16 sndxcnt_sweep = (gbChannel == 0) ? REG_SOUND1CNT_L : 0;
        u16 sndxcnt_LenEnvelope = (gbChannel == 0) ? REG_SOUND1CNT_H : REG_SOUND2CNT_L;
        u16 sndxcnt_freqCtrl = (gbChannel == 0) ? REG_SOUND1CNT_X : REG_SOUND2CNT_H;

        for (int audioChannel = 0; audioChannel < AUDIO_CHANNEL_COUNT; audioChannel++)
        {
            s32* out = outBuffer;

            bool32 isChannelEnabled = REG_SOUNDCNT_L & (1 << (8 + (audioChannel * 4) + gbChannel));
            if (isChannelEnabled) {
                // Somehow using the "correct" volume control makes some output disappear
                // Example: When Torchic falls down in the opening
                float channelVolume = (float)((sndxcnt_LenEnvelope >> 12) & 0xF) / 15.0f;
                
                float masterVol = (float)((REG_SOUNDCNT_L >> (audioChannel * 4)) & 0x3) / 7.0f;
                

                float frequency = 131072.0f / (2048.0f - (float)(sndxcnt_freqCtrl & 0x7FF));
                s32 volume = masterVol * channelVolume * (float)MAX_VOLUME;


                if (volume > 0)
                {
                    int wavePeriod = sampleRate / frequency;

                    bool32 channelInit = (sndxcnt_freqCtrl & 0x8000);
                    bool32 channelStopOnZeroLength = (sndxcnt_freqCtrl & 0x4000);

                    u8 channelDuty = (sndxcnt_LenEnvelope >> 6) & 0x3;
                    int waveDuty = wavePeriod * GbWaveDuty[channelDuty];

                    for (int i = 0; i < samplesPerFrame; i++, out++)
                    {
                        AudioSample SampleValue = (((currentSampleIndex[gbChannel][audioChannel] - waveStart[gbChannel][audioChannel]) % wavePeriod) < waveDuty)
                            ? volume : -volume;

                        if ((prevSampleValue[gbChannel][audioChannel] <= 0) && (SampleValue > 0))
                            waveStart[gbChannel][audioChannel] = currentSampleIndex[gbChannel][audioChannel];

                        out[audioChannel * MIXED_AUDIO_BUFFER_SIZE] += SampleValue;

                        prevSampleValue[gbChannel][audioChannel] = SampleValue;

                        currentSampleIndex[gbChannel][audioChannel]++;
                    }
                }
                else
                {
                    prevSampleValue[gbChannel][audioChannel] = 0;
                    currentSampleIndex[gbChannel][audioChannel] += samplesPerFrame;
                }
            }
        }
    }
}
