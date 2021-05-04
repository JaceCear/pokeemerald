#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "gba/types.h"
#include "gba/io_reg.h"

#include "gb_audio.h"
#include "sound_mixer.h"

typedef s32 AudioSample; // update to float once converted
#define GB_CHAN_SWEEP         0
#define GB_CHAN_TONE_2        1
#define GB_TONE_CHANNEL_COUNT 2 // Gameboy channels using the "Tone" feature (2 channels: 1 and 2)
#define AUDIO_CHANNEL_COUNT   2 // Number of mono/stereo channels
#define MAX_VOLUME 0x1FFFFFFF; // 1.0f once mixer is float

static const float GbWaveDuty[4] = 
    { 1.0f / 8.0f,
      1.0f / 4.0f,
      1.0f / 2.0f,
      3.0f / 4.0f };

static const float ChannelVolumeTable[4] =
{
    [SOUND_CGB_MIX_QUARTER] = 1.0f / 4.0f,
    [SOUND_CGB_MIX_HALF]    = 1.0f / 2.0f,
    [SOUND_CGB_MIX_FULL]    = 1.0f,
    [3] = 0.0f, // "Prohibited" according to GBATEK
};

// multiply by (REG_SOUND1CNT_H & 0x3F) or (REG_SOUND2CNT_L & 0x3F) to get sound duration
static const float ToneSoundLengthRate = (1.0f / 256.0f); 

static s32 currentSampleIndex[GB_TONE_CHANNEL_COUNT][AUDIO_CHANNEL_COUNT] = { 0 };
static s32 waveStart[GB_TONE_CHANNEL_COUNT][AUDIO_CHANNEL_COUNT] = { 0 };
static AudioSample prevSampleValue[GB_TONE_CHANNEL_COUNT][AUDIO_CHANNEL_COUNT] = { 0 };

static float UpdateAndGetSweepFreq()
{
    bool32 isDecrementing = REG_SOUND1CNT_L & (1 << 3);
    
    float rate = (float)(REG_SOUND1CNT_X & 0x7FF);
    float prevRate = rate;

    float rateDiff = rate / (float)(1 << (REG_SOUND1CNT_L & 0x7));

    rate += (isDecrementing) ? -rateDiff : +rateDiff;

    if (rate < 0.0f)
        rate = 0.0f;
    else if (rate > 2047.0f)
        rate = 2047.0f;

    REG_SOUND1CNT_X &= ~0x7FF;
    REG_SOUND1CNT_X |= ((s32)rate) & 0x7FF;

    float result = (prevRate == rate) ? 0.0f : (131072.0f / (2048.0f - rate));

    return result;
}

static int sLastSweepIndex[AUDIO_CHANNEL_COUNT] = { 0 };

static void
ApplySweep(AudioSample *outBuffer, u16 sampleRate, u16 samplesPerFrame, float prevFreq, AudioSample volume, int audioChannel)
{
    AudioSample* out = outBuffer;
    float currFreq = prevFreq;

    // Sample count and time each of the sweeps (*sweepCount) will take; every frequency step will take this same amount of time
    float secondsPerSweep = (float)((REG_SOUND1CNT_L >> 4) & 0x7) * 0.0078125f;
    s32 samplesPerSweep = (float)sampleRate * secondsPerSweep;

    bool32 sweepInit = (REG_SOUND1CNT_X & 0x8000);

    if (sweepInit)
    {
        REG_SOUND1CNT_X &= ~0x8000;

        waveStart[GB_CHAN_SWEEP][0] = currentSampleIndex[GB_CHAN_SWEEP][0];
        waveStart[GB_CHAN_SWEEP][1] = currentSampleIndex[GB_CHAN_SWEEP][1];
    //    currentSampleIndex[GB_CHAN_SWEEP][0] = 0; // BUG: Taking these away lets the sweep sound wrong
    //    currentSampleIndex[GB_CHAN_SWEEP][1] = 0; //      But currentSampleIndex should only always increase
        prevSampleValue[GB_CHAN_SWEEP][0] = 0;
        prevSampleValue[GB_CHAN_SWEEP][1] = 0;

        sLastSweepIndex[0] = 0;
        sLastSweepIndex[1] = 0;
    }
    
    for (int regionSampleIndex = 0; regionSampleIndex < samplesPerFrame; regionSampleIndex++, out++)
    {
        AudioSample volume;
        CalculateToneVolume(sampleRate, &volume, GB_CHAN_SWEEP, audioChannel);
        
        s32 wavePeriod = ((float)sampleRate) / currFreq;
        s32 relativewaveStart = currentSampleIndex[GB_CHAN_SWEEP][audioChannel] - waveStart[GB_CHAN_SWEEP][audioChannel];
        int sweepIndex = SAFE_DIV(relativewaveStart, samplesPerSweep);
        if (sweepIndex != sLastSweepIndex[audioChannel])
        {
            printf("prev: %f\n", currFreq);
            currFreq = UpdateAndGetSweepFreq();
            printf("new:  %f\n", currFreq);
            wavePeriod = ((float)sampleRate) / currFreq;
        }
        sLastSweepIndex[audioChannel] = sweepIndex;
            
        u8 channelDuty = (REG_SOUND1CNT_H >> 6) & 0x3;
        s32 waveDuty = wavePeriod * GbWaveDuty[channelDuty];

            
        if (currentSampleIndex[GB_CHAN_SWEEP][audioChannel] >= waveStart[GB_CHAN_SWEEP][audioChannel] + wavePeriod) {
            waveStart[GB_CHAN_SWEEP][audioChannel] = currentSampleIndex[GB_CHAN_SWEEP][audioChannel];
        }


        int soundLengthBound = ToneSoundLengthRate * sampleRate;
        if (currentSampleIndex[GB_CHAN_SWEEP][audioChannel] - waveStart[GB_CHAN_SWEEP][audioChannel] == soundLengthBound) // Check out this check
        {
            int regSoundLen = REG_SOUND1CNT_H & 0x3F;
            regSoundLen--;
            REG_SOUND1CNT_H &= (~0x3F) | regSoundLen;
        }

        float soundLengthIndex = (float)((64 - (REG_SOUND1CNT_H & 0x3F)) * ToneSoundLengthRate);
        soundLengthIndex *= sampleRate;

        if (!(REG_SOUND1CNT_X & (1 << 14)) || (currentSampleIndex[GB_CHAN_SWEEP][audioChannel] - waveStart[GB_CHAN_SWEEP][audioChannel]) < (int)soundLengthIndex)
        {
            AudioSample SampleValue = (
                (SAFE_MOD(currentSampleIndex[GB_CHAN_SWEEP][audioChannel] - waveStart[GB_CHAN_SWEEP][audioChannel], wavePeriod)) < waveDuty) ? volume : -volume;

            //if ((prevSampleValue[GB_CHAN_SWEEP][audioChannel] <= 0) && (SampleValue > 0))
            //    waveStart[GB_CHAN_SWEEP][audioChannel] = currentSampleIndex[GB_CHAN_SWEEP][audioChannel];

            out[audioChannel /* * MIXED_AUDIO_BUFFER_SIZE*/] += SampleValue;

            prevSampleValue[GB_CHAN_SWEEP][audioChannel] = SampleValue;
        }

        currentSampleIndex[GB_CHAN_SWEEP][audioChannel]++;
    }    
}

static const float EnvelStepTimeRate = 1.0f / 64.0f;

static s32 sEnvelopeStartIndex[GB_TONE_CHANNEL_COUNT][AUDIO_CHANNEL_COUNT] = { { 0, 0 }, { 0, 0 } };
static u16 sPrevEnvelope[GB_TONE_CHANNEL_COUNT] = { 0, 0 };
static bool8 sEnvelWasInactive[GB_TONE_CHANNEL_COUNT] = { TRUE, TRUE };
static s32 sCurrentEnvelRegion = 0;
static const u16* envelRegs[GB_TONE_CHANNEL_COUNT] = { &REG_SOUND1CNT_H, &REG_SOUND2CNT_L };
static const u16* freqRegs[GB_TONE_CHANNEL_COUNT]  = { &REG_SOUND1CNT_X, &REG_SOUND2CNT_H };
static s16 sChannelRate[GB_TONE_CHANNEL_COUNT] = { 0, 0 };

void
CalculateToneVolume(int sampleRate, AudioSample* volume, int gbChannel, int audioChannel)
{
#if 1 // Maybe keep as as assert()?
    if (!(gbChannel < 1 /*AUDIO_CHANNEL_COUNT*/))
        return;


    float masterVol = (float)((REG_SOUNDCNT_L >> (audioChannel * 4)) & 0x7) / 7.0f;
    *volume = masterVol * ((*envelRegs[gbChannel] >> 12) & 0xF) / 15.0f * MAX_VOLUME;
#else
    u16* envelReg = envelRegs[gbChannel];
    float stepTime = (float)((*envelReg >> 8) & 0x7) * EnvelStepTimeRate;
    float toneLength = (float)(64 - (*envelReg & 0x3F)) / 256.0f;

    if (audioChannel > 0)
        return;
    

    if (sPrevEnvelope[gbChannel] != *envelReg)
    {
        sPrevEnvelope[gbChannel] = *envelReg;
        sChannelRate[gbChannel] = ((*envelReg >> 12) & 0xF);
        sEnvelopeStartIndex[gbChannel][audioChannel] = currentSampleIndex[gbChannel][audioChannel];
    }

    s32 samplesSinceStart = (currentSampleIndex[gbChannel][audioChannel] - sEnvelopeStartIndex[gbChannel][audioChannel]);

    bool32 wasInactive;
    bool32 wasIncreasing = sPrevEnvelope[gbChannel] & (1 << 11);
    wasInactive = (!wasIncreasing && ((sPrevEnvelope[gbChannel] >> 12) & 0xF) == 0);
    wasInactive |= (samplesSinceStart-1 > toneLength * sampleRate);
    
    bool32 isInactive;
    bool32 isIncreasing = *envelReg & (1 << 11);
    isInactive = (!isIncreasing && sChannelRate[gbChannel] == 0);
    isInactive |= ((*freqRegs[gbChannel] & (1 << 14)) && (samplesSinceStart > toneLength * sampleRate));


    if (!isInactive)
    {
        if (wasInactive)
        {
            sChannelRate[gbChannel] = ((*envelReg >> 12) & 0xF);
            sEnvelopeStartIndex[gbChannel][audioChannel] = currentSampleIndex[gbChannel][audioChannel];
        }

        s32 samplesPerRegion = (s32)(stepTime * sampleRate);
        
        s16 rate = sChannelRate[gbChannel];
        rate += (isIncreasing) ? +SAFE_DIV(samplesSinceStart, samplesPerRegion)
                               : -SAFE_DIV(samplesSinceStart, samplesPerRegion);

        if (rate < 0)
            rate = 0;

        if (rate > 15)
            rate = 15;

        *envelReg &= ~(0xF << 12);
        *envelReg |= (rate << 12);
        
        float masterVol = (float)((REG_SOUNDCNT_L >> (audioChannel * 4)) & 0x7) / 7.0f;
        float dmgVol = ChannelVolumeTable[(REG_SOUNDCNT_H & 0x3)];
        *volume = masterVol * dmgVol * (((float)((*envelReg >> 12) & 0xF)) / 15.0f) * (float)MAX_VOLUME;
    }
    else
    {
        sEnvelopeStartIndex[gbChannel][audioChannel] = currentSampleIndex[gbChannel][audioChannel];
        *volume = 0;
    }
#endif
}

// Mixes GB Sound Channels 1 and 2
void
MixGBToneChannels(AudioSample* outBuffer, s32 sampleRate, u16 samplesPerFrame)
{
    for (int gbChannel = 0; gbChannel < GB_TONE_CHANNEL_COUNT; gbChannel++)
    {
        u16 sndxcnt_freqCtrl = (gbChannel == 0) ? REG_SOUND1CNT_X : REG_SOUND2CNT_H;

        for (int audioChannel = 0; audioChannel < 1/*AUDIO_CHANNEL_COUNT*/; audioChannel++)
        {
            AudioSample* out = outBuffer;

            bool32 isChannelEnabled = REG_SOUNDCNT_L & (1 << (8 + (audioChannel * 4) + gbChannel));
            if (isChannelEnabled) {

                float frequency = 131072.0f / (2048.0f - (float)(*freqRegs[gbChannel] & 0x7FF));

                
                if (gbChannel == 0)
                {
                    bool32 sweepIsActive = ((REG_SOUND1CNT_L >> 4) & 0x7);
                        
                    if (sweepIsActive)
                    {
                        ApplySweep(outBuffer, sampleRate, samplesPerFrame,
                            frequency, 0, audioChannel);

                        continue;
                    }
                }
                
                int wavePeriod = sampleRate / frequency;

                u8 channelDuty = (*envelRegs[gbChannel] >> 6) & 0x3;
                int waveDuty = wavePeriod * GbWaveDuty[channelDuty];

                for (int i = 0; i < samplesPerFrame; i++, out++)
                {
                    AudioSample volume;
                    CalculateToneVolume(sampleRate, &volume, gbChannel, audioChannel);

                    AudioSample SampleValue = (((currentSampleIndex[gbChannel][audioChannel] - waveStart[gbChannel][audioChannel]) % wavePeriod) < waveDuty)
                        ? volume : -volume;

                    if ((prevSampleValue[gbChannel][audioChannel] <= 0) && (SampleValue > 0))
                        waveStart[gbChannel][audioChannel] = currentSampleIndex[gbChannel][audioChannel];

                    out[audioChannel /* * MIXED_AUDIO_BUFFER_SIZE*/] += SampleValue;

                    prevSampleValue[gbChannel][audioChannel] = SampleValue;

                    currentSampleIndex[gbChannel][audioChannel]++;
                }
            }
        }
    }
}
