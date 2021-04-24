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

static s32 sweepStart[AUDIO_CHANNEL_COUNT] = { 0 };

static float sweepStartFrequency = 0.0f;
static float sweepProgress[AUDIO_CHANNEL_COUNT] = { 0.0f };

static float GetNthSweepFreq(float startFreq, int n, bool32 isDecrementing)
{
    float newFreq = startFreq;

    while (n > 0) // TODO: Should this be > or >= ?
    {
        float diffFreq = newFreq / (float)(1 << n);

        newFreq += isDecrementing ? -diffFreq : diffFreq;

        n--;
    }

    return newFreq;
}

static s32 sweepRemainingSamples[AUDIO_CHANNEL_COUNT] = {0};

static void
ApplySweep(AudioSample *outBuffer, u16 sampleRate, u16 samplesPerFrame, float prevFreq, AudioSample volume, int audioChannel)
{
    AudioSample* out = outBuffer;
    float currFreq = prevFreq;

    const MsPerFrame = 1.0f / 60.0f;

    u16 sweepReg = REG_SOUND1CNT_L;

    int sweepCount = sweepReg & 0x7;
    bool32 isDecrementSweep = sweepReg & (1 << 3);
    
    // Sample count and time each of the sweeps (*sweepCount) will take; every frequency step will take this same amount of time
    float secondsPerSweep = (float)((sweepReg >> 4) & 0x7) * 0.0078f;
    s32 samplesPerSweep = (float)sampleRate * secondsPerSweep;

    bool32 sweepInit = (REG_SOUND1CNT_X & 0x8000);

    if (sweepInit)
    {
        printf("Chan1: Sweep-Initted.\n");
        REG_SOUND1CNT_X &= ~0x8000;

        currFreq = 131072.0f / (2048.0f - (float)(REG_SOUND1CNT_X & 0x7FF));

        // TODO: Reset everything that is needed for the previously playing sound
        // so that the new settings can be applied
        waveStart[0][0] = 0;
        waveStart[0][1] = 0;
        currentSampleIndex[0][0] = 0;
        currentSampleIndex[0][1] = 0;
        prevSampleValue[0][0] = 0;
        prevSampleValue[0][1] = 0;

        sweepRemainingSamples[0] = samplesPerSweep * (REG_SOUND1CNT_L & 0x7);
        sweepRemainingSamples[1] = samplesPerSweep * (REG_SOUND1CNT_L & 0x7);
    }
    printf("%04X\n", sweepReg);
    s32 initialSampleIndex = currentSampleIndex[0][audioChannel];
    s32 remainingLoopSamples;
    do
    {
        currFreq = GetNthSweepFreq(prevFreq, currentSampleIndex[0][audioChannel] / samplesPerSweep, isDecrementSweep);

        s32 wavePeriod = sampleRate / currFreq;
        u8 channelDuty = (REG_SOUND1CNT_H >> 6) & 0x3;
        s32 waveDuty = wavePeriod * GbWaveDuty[channelDuty];

        remainingLoopSamples = (currentSampleIndex[0][audioChannel] - initialSampleIndex);

        s32 outputSampleCount = min(samplesPerSweep, samplesPerFrame);

        if((remainingLoopSamples > 0) && remainingLoopSamples < outputSampleCount)
            outputSampleCount = remainingLoopSamples;

        //printf("samples: %d ; %d\n", samplesPerSweep, outputSampleCount);
        //printf("freq   : %f\n", currFreq);


        for (int regionSampleIndex = (currentSampleIndex[0][audioChannel] % outputSampleCount); regionSampleIndex < outputSampleCount; regionSampleIndex++, out++)
        {
            AudioSample SampleValue = (((waveStart[0][audioChannel] + currentSampleIndex[0][audioChannel]) % wavePeriod) < waveDuty) ? volume : -volume;

            if (prevSampleValue[0][audioChannel] <= 0 && SampleValue > 0)
                waveStart[0][audioChannel] = currentSampleIndex[0][audioChannel];

            //printf("%c", (SampleValue > 0) ? '-' : '_');

            out[audioChannel * MIXED_AUDIO_BUFFER_SIZE] += SampleValue;
            prevSampleValue[0][audioChannel] = SampleValue;
            currentSampleIndex[0][audioChannel]++;

            if(--sweepRemainingSamples[audioChannel] == 0)
                return;
        }
    } while (((out - outBuffer) / sizeof(AudioSample)) > samplesPerFrame);
}

void
GbTest(AudioSample* outBuffer, s32 sampleRate, u16 samplesPerFrame)
{
    // TODO: Implement channel 1 sweeps
    //       Properly implement envelope of both 1 and 2
    for (int gbChannel = 0; gbChannel < GB_TONE_CHANNEL_COUNT; gbChannel++)
    {
        u16 sndxcnt_LenEnvelope = (gbChannel == 0) ? REG_SOUND1CNT_H : REG_SOUND2CNT_L;
        u16 sndxcnt_freqCtrl = (gbChannel == 0) ? REG_SOUND1CNT_X : REG_SOUND2CNT_H;

        for (int audioChannel = 0; audioChannel < AUDIO_CHANNEL_COUNT; audioChannel++)
        {
            AudioSample* out = outBuffer;

            bool32 isChannelEnabled = REG_SOUNDCNT_L & (1 << (8 + (audioChannel * 4) + gbChannel));
            if (isChannelEnabled) {
                // Somehow using the "correct" volume control makes some output disappear
                // Example: When Torchic falls down in the opening
                float channelVolume = (float)((sndxcnt_LenEnvelope >> 12) & 0xF) / 15.0f;
                
                float masterVol = (float)((REG_SOUNDCNT_L >> (audioChannel * 4)) & 0x3) / 7.0f;
                
                float frequency = 131072.0f / (2048.0f - (float)(sndxcnt_freqCtrl & 0x7FF));
                AudioSample volume = masterVol * channelVolume * (float)MAX_VOLUME;


                if (volume > 0)
                {
                    if (gbChannel == 0)
                    {
#if 1
                        bool32 sweepIsActive = ((REG_SOUND1CNT_L >> 4) & 0x7);
                        
                        if (sweepIsActive) {
                            ApplySweep(outBuffer, sampleRate, samplesPerFrame,
                                frequency, volume, audioChannel);

                            continue;
                        }
#endif
                    }

                    int wavePeriod = sampleRate / frequency;

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
