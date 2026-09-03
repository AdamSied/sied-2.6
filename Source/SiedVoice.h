#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <cstdint>
#include <memory>

enum class SiedLayer : int
{
    oneShotA = 0,
    oneShotB,
    texture,
    count
};

struct SiedSampleData
{
    juce::AudioBuffer<float> audio;
    double sourceSampleRate = 44100.0;
    float gainLinear = 1.0f;
    int loopStart = 0;
    int loopEnd = 0;
    int loopCrossfade = 0;
    bool loopEnabled = false;
    juce::String displayName { "Empty" };
};

class SiedSampleBank
{
public:
    void setSample (SiedLayer, std::shared_ptr<const SiedSampleData>);
    std::shared_ptr<const SiedSampleData> getSample (SiedLayer) const;

private:
    std::array<std::shared_ptr<const SiedSampleData>, static_cast<size_t> (SiedLayer::count)> samples;
};

class SiedSound final : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override       { return true; }
    bool appliesToChannel (int) override    { return true; }
};

class SiedVoice final : public juce::SynthesiserVoice
{
public:
    SiedVoice (SiedSampleBank&, juce::AudioProcessorValueTreeState&);

    bool canPlaySound (juce::SynthesiserSound*) override;
    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int pitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int newValue) override;
    void controllerMoved (int controllerNumber, int newValue) override;
    void renderNextBlock (juce::AudioBuffer<float>&, int startSample, int numSamples) override;
    void prepare (double newSampleRate, int samplesPerBlock, int outputChannels);
    void setVoiceEnabled (bool shouldBeEnabled);
    float consumePlaybackPosition (SiedLayer);
    uint64_t getDisplayPriority() const noexcept { return displayPriority; }

private:
    struct LayerPlayback
    {
        std::shared_ptr<const SiedSampleData> sample;
        double position = 0.0;
        double octavePosition = 0.0;
        double regionStart = 0.0;
        double regionEnd = 0.0;
        double loopStart = 0.0;
        double loopEnd = 0.0;
        int loopCrossfade = 0;
        bool loopEnabled = false;
        bool reverse = false;
        bool texture = false;
        bool finished = true;
    };

    void configureLayer (SiedLayer, LayerPlayback&);
    float renderLayerSample (const LayerPlayback&, int channel, double position) const;
    void advanceLayerPosition (LayerPlayback&, double& position, double increment,
                               bool markFinished) const;
    float renderStarterOscillator (double phaseValue) const;
    void updatePitchTarget (bool useGlide);
    bool allSampleLayersFinished() const;

    SiedSampleBank& sampleBank;
    juce::AudioProcessorValueTreeState& parameters;
    std::array<LayerPlayback, static_cast<size_t> (SiedLayer::count)> layers;
    juce::ADSR adsr;
    juce::ADSR::Parameters envelope;
    juce::SmoothedValue<double, juce::ValueSmoothingTypes::Multiplicative> frequencySmoother;

    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* decayParam = nullptr;
    std::atomic<float>* sustainParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* toneParam = nullptr;
    std::atomic<float>* driftParam = nullptr;
    std::atomic<float>* haloParam = nullptr;
    std::atomic<float>* shimmerMixParam = nullptr;
    std::atomic<float>* rootNoteParam = nullptr;
    std::array<std::atomic<float>*, 2> layerRootParams {};
    std::atomic<float>* sampleStartParam = nullptr;
    std::atomic<float>* sampleEndParam = nullptr;
    std::atomic<float>* sample2StartParam = nullptr;
    std::atomic<float>* sample2EndParam = nullptr;
    std::atomic<float>* textureStartParam = nullptr;
    std::atomic<float>* textureRandomParam = nullptr;
    std::atomic<float>* oneShotALevelParam = nullptr;
    std::atomic<float>* oneShotBLevelParam = nullptr;
    std::atomic<float>* textureLevelParam = nullptr;
    std::array<std::atomic<float>*, static_cast<size_t> (SiedLayer::count)> layerEnabledParams {};
    std::array<std::atomic<float>*, static_cast<size_t> (SiedLayer::count)> layerTransposeParams {};
    std::array<std::atomic<float>*, static_cast<size_t> (SiedLayer::count)> layerFineParams {};
    std::array<juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>,
               static_cast<size_t> (SiedLayer::count)> layerEnableSmoothers;
    std::atomic<float>* transposeParam = nullptr;
    std::atomic<float>* fineTuneParam = nullptr;
    std::atomic<float>* velocityParam = nullptr;
    std::atomic<float>* reverseParam = nullptr;
    std::atomic<float>* loopModeParam = nullptr;
    std::atomic<float>* glideParam = nullptr;
    std::atomic<float>* monoParam = nullptr;

    double outputSampleRate = 44100.0;
    double phase = 0.0;
    double driftPhase = 0.0;
    double targetFrequency = 440.0;
    double previousFrequency = 440.0;
    int currentMidiNote = 60;
    int pitchWheel = 8192;
    float noteVelocity = 0.0f;
    float filterState[2] { 0.0f, 0.0f };
    bool hasPreviousPitch = false;
    std::atomic<bool> enabled { true };
    std::array<float, static_cast<size_t> (SiedLayer::count)> blockPlaybackPositions
    {{ -1.0f, -1.0f, -1.0f }};
    uint64_t displayPriority = 0;

    inline static std::atomic<uint64_t> nextDisplayPriority { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SiedVoice)
};
