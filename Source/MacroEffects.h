#pragma once

#include <JuceHeader.h>
#include <array>

struct SiedEffectSettings
{
    float shimmer = 0.0f, chorus = 0.0f, delay = 0.0f, crush = 0.0f;
    float reverb = 0.0f, drive = 0.0f, phaser = 0.0f, flanger = 0.0f;
    float tremolo = 0.0f, width = 1.0f, lowpass = 0.0f, highpass = 0.0f;
    float compressor = 0.0f, pan = 0.0f, outputGainDb = -2.0f;
    float shimmerMix = 1.0f, chorusMix = 1.0f, delayMix = 1.0f, crushMix = 1.0f;
    float reverbMix = 1.0f, driveMix = 1.0f, phaserMix = 1.0f, flangerMix = 1.0f;
    float tremoloMix = 1.0f, widthMix = 1.0f, lowpassMix = 1.0f;
    float highpassMix = 1.0f, compressorMix = 1.0f;
    float delayTimeMs = 350.0f, tempoBpm = 120.0f;
    int delayType = 0, reverbType = 0, lowpassType = 0, highpassType = 0;
    int delayDivision = 0, chorusType = 0, crushType = 0, driveType = 0;
    int phaserType = 0, flangerType = 0, tremoloType = 0, compressorType = 0;
};

class MacroEffects
{
public:
    void prepare (const juce::dsp::ProcessSpec&);
    void reset();
    void process (juce::AudioBuffer<float>&, const SiedEffectSettings&);

private:
    void processCrush (juce::AudioBuffer<float>&, float, float, int);
    void processDelay (juce::AudioBuffer<float>&, float, float, int, int, float, float);
    void processShimmer (juce::AudioBuffer<float>&, float, float);
    void processReverb (juce::AudioBuffer<float>&, float, float, int);
    void processDrive (juce::AudioBuffer<float>&, float, float, int);
    void processFlanger (juce::AudioBuffer<float>&, float, float, int);
    void processTremolo (juce::AudioBuffer<float>&, float, float, int);
    void processFilters (juce::AudioBuffer<float>&, float, float, float, float, int, int);
    void processStereo (juce::AudioBuffer<float>&, float, float, float);
    void copyDry (const juce::AudioBuffer<float>&);
    void blendDryWet (juce::AudioBuffer<float>&, float);

    double currentSampleRate = 44100.0;
    juce::dsp::Chorus<float> chorusProcessor;
    juce::dsp::Phaser<float> phaserProcessor;
    juce::dsp::Compressor<float> compressorProcessor;
    juce::Reverb shimmerReverb, roomReverb;
    juce::AudioBuffer<float> shimmerBuffer, roomBuffer, delayBuffer, flangerBuffer, dryBuffer;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delaySamplesSmoother;
    int delayWritePosition = 0, flangerWritePosition = 0;
    int crushCounter[2] { 0, 0 };
    float crushHeldSample[2] { 0.0f, 0.0f };
    float crushFilterState[2] { 0.0f, 0.0f };
    float delayFilterState[2] { 0.0f, 0.0f };
    float lowpassState[2] { 0.0f, 0.0f };
    float lowpassState2[2] { 0.0f, 0.0f };
    float highpassState[2] { 0.0f, 0.0f };
    float highpassState2[2] { 0.0f, 0.0f };
    float shimmerHighpassState[2] { 0.0f, 0.0f };
    float shimmerHighpassState2[2] { 0.0f, 0.0f };
    double flangerPhase = 0.0, tremoloPhase = 0.0;
};
