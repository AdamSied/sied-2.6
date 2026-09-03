#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <SiedFactoryData.h>

namespace
{
struct FactoryPreset
{
    const char* name;
    const char* category;
    const char* data;
    int dataSize;
    int rootNote;
    float gainDb;
    float halo;
    float drift;
    float ghost;
    float crush;
    float attack;
    float release;
    float tone;
    int loopStart;
    int loopEnd;
};

const FactoryPreset factoryPresets[]
{
    { "Ascent",    "Keys",   SiedFactoryData::key_ascent_wav,    SiedFactoryData::key_ascent_wavSize,    60, -7.9f, 0.18f, 0.08f, 0.06f, 0.00f, 0.015f, 1.8f, 0.78f,      0,      0 },
    { "Easy",      "Keys",   SiedFactoryData::key_easy_wav,      SiedFactoryData::key_easy_wavSize,      60, 16.0f, 0.42f, 0.24f, 0.18f, 0.00f, 0.025f, 2.6f, 0.72f,      0,      0 },
    { "Island",    "Keys",   SiedFactoryData::key_island_wav,    SiedFactoryData::key_island_wavSize,    60, -6.9f, 0.20f, 0.14f, 0.08f, 0.00f, 0.012f, 1.7f, 0.80f,      0,      0 },
    { "Multi",     "Keys",   SiedFactoryData::key_multi_wav,     SiedFactoryData::key_multi_wavSize,     60, 10.6f, 0.28f, 0.16f, 0.10f, 0.00f, 0.018f, 2.0f, 0.76f,      0,      0 },
    { "Noise",     "Keys",   SiedFactoryData::key_noise_wav,     SiedFactoryData::key_noise_wavSize,     60,-10.0f, 0.16f, 0.10f, 0.08f, 0.08f, 0.008f, 1.4f, 0.68f,      0,      0 },
    { "Octopus",   "Keys",   SiedFactoryData::key_octopus_wav,   SiedFactoryData::key_octopus_wavSize,   60, 14.8f, 0.35f, 0.18f, 0.16f, 0.00f, 0.025f, 2.5f, 0.73f,      0,      0 },
    { "Topograph", "Keys",   SiedFactoryData::key_topograph_wav, SiedFactoryData::key_topograph_wavSize, 72,  2.7f, 0.26f, 0.16f, 0.12f, 0.00f, 0.010f, 1.8f, 0.77f,      0,      0 },
    { "Avenge",    "Pads",   SiedFactoryData::pad_avenge_wav,    SiedFactoryData::pad_avenge_wavSize,    60, 16.0f, 0.46f, 0.18f, 0.28f, 0.00f, 0.420f, 5.2f, 0.66f, 236884, 460549 },
    { "Boost",     "Pads",   SiedFactoryData::pad_boost_wav,     SiedFactoryData::pad_boost_wavSize,     60, -4.9f, 0.32f, 0.12f, 0.16f, 0.02f, 0.260f, 3.8f, 0.69f, 151331, 259880 },
    { "Hello",     "Pads",   SiedFactoryData::pad_hello_wav,     SiedFactoryData::pad_hello_wavSize,     60,  6.7f, 0.40f, 0.20f, 0.24f, 0.00f, 0.360f, 4.6f, 0.67f, 116189, 243978 },
    { "Glow",      "Plucks", SiedFactoryData::pluck_glow_wav,    SiedFactoryData::pluck_glow_wavSize,    72,  9.5f, 0.38f, 0.12f, 0.14f, 0.00f, 0.002f, 2.4f, 0.80f,      0,      0 },
    { "Pig",       "Plucks", SiedFactoryData::pluck_pig_wav,     SiedFactoryData::pluck_pig_wavSize,     72, 16.0f, 0.24f, 0.08f, 0.06f, 0.02f, 0.002f, 1.3f, 0.84f,      0,      0 },
    { "Resign",    "Plucks", SiedFactoryData::pluck_resign_wav,  SiedFactoryData::pluck_resign_wavSize,  72,  6.5f, 0.22f, 0.09f, 0.08f, 0.01f, 0.002f, 1.5f, 0.82f,      0,      0 },
    { "Vase",      "Bells",  SiedFactoryData::bell_vase_wav,     SiedFactoryData::bell_vase_wavSize,     72,  6.9f, 0.36f, 0.10f, 0.12f, 0.00f, 0.002f, 2.8f, 0.83f,      0,      0 },
    { "Light",     "Leads",  SiedFactoryData::lead_light_wav,    SiedFactoryData::lead_light_wavSize,    72, 12.8f, 0.16f, 0.10f, 0.07f, 0.00f, 0.010f, 1.5f, 0.79f,      0,      0 }
};

constexpr int factoryPresetCount = static_cast<int> (std::size (factoryPresets));

void setStateParameter (juce::AudioProcessorValueTreeState& state,
                        const juce::String& parameterID, float actualValue)
{
    if (auto* parameter = state.getParameter (parameterID))
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (actualValue));
}

void setStateParameterNormalised (juce::AudioProcessorValueTreeState& state,
                                  const juce::String& parameterID, float normalisedValue)
{
    if (auto* parameter = state.getParameter (parameterID))
        parameter->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, normalisedValue));
}

juce::String oneShotCategoryForName (const juce::String& sourceName)
{
    const auto name = sourceName.toLowerCase();
    if (name.contains ("bass") || name.contains ("sub")) return "Bass";
    if (name.contains ("bell") || name.contains ("celesta")) return "Bells";
    if (name.contains ("pad") || name.contains ("ambient")) return "Pads";
    if (name.contains ("pluck")) return "Plucks";
    if (name.contains ("lead")) return "Leads";
    if (name.contains ("piano")) return "Pianos";
    if (name.contains ("key")) return "Keys";
    if (name.contains ("synth")) return "Synths";
    if (name.contains ("flute") || name.contains ("wind")) return "Winds";
    if (name.contains ("vocal") || name.contains ("voice")) return "Vocals";
    return "Other";
}

juce::String textureCategoryForName (const juce::String& sourceName)
{
    const auto name = sourceName.toLowerCase();
    if (name.contains ("rain") || name.contains ("bird") || name.contains ("water")
        || name.contains ("forest") || name.contains ("nature")) return "Nature";
    if (name.contains ("vinyl") || name.contains ("tape") || name.contains ("cassette")
        || name.contains ("schellack")) return "Vinyl + Tape";
    if (name.contains ("noise") || name.contains ("hiss") || name.contains ("static")) return "Noise";
    if (name.contains ("room") || name.contains ("street") || name.contains ("crowd")) return "Spaces";
    return "Atmosphere";
}

bool isSupportedAudioFile (const juce::File& file)
{
    const auto extension = file.getFileExtension().toLowerCase();
    return extension == ".wav" || extension == ".aif" || extension == ".aiff"
           || extension == ".flac";
}
}

SiedAudioProcessor::SiedAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "SIED_STATE", createParameterLayout())
{
    formatManager.registerBasicFormats();
    for (int i = 0; i < 32; ++i)
        synth.addVoice (new SiedVoice (sampleBank, parameters));
    synth.setNoteStealingEnabled (true);
    synth.addSound (new SiedSound());

    rescanLibrary();
    loadFavourites();
    resetToInitPatch();
}

juce::AudioProcessorValueTreeState::ParameterLayout SiedAudioProcessor::createParameterLayout()
{
    using ID = juce::ParameterID;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "halo", 1 }, "Shimmer", 0.0f, 1.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "drift", 1 }, "Chorus", 0.0f, 1.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "ghost", 1 }, "Delay", 0.0f, 1.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "crush", 1 }, "Crush", 0.0f, 1.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "reverb", 1 }, "Reverb", 0.0f, 1.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "drive", 1 }, "Drive", 0.0f, 1.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "phaser", 1 }, "Phaser", 0.0f, 1.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "flanger", 1 }, "Flanger", 0.0f, 1.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "tremolo", 1 }, "Tremolo", 0.0f, 1.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "width", 1 }, "Width", 0.0f, 2.0f, 1.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "lowpass", 1 }, "Low-pass", 0.0f, 1.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "highpass", 1 }, "High-pass", 0.0f, 1.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "compressor", 1 }, "Compressor", 0.0f, 1.0f, 0.0f));
    struct MixParameter
    {
        const char* id;
        const char* name;
        float defaultValue;
    };
    for (const auto& item : std::array<MixParameter, 13>
    {{
        { "shimmerMix", "Shimmer Mix", 0.68f }, { "chorusMix", "Chorus Mix", 0.50f },
        { "delayMix", "Delay Mix", 0.38f }, { "crushMix", "Crush Mix", 0.50f },
        { "reverbMix", "Reverb Mix", 0.42f }, { "driveMix", "Drive Mix", 0.55f },
        { "phaserMix", "Phaser Mix", 0.50f }, { "flangerMix", "Flanger Mix", 0.50f },
        { "tremoloMix", "Tremolo Mix", 0.75f }, { "widthMix", "Width Mix", 1.00f },
        { "lowpassMix", "Low-pass Mix", 1.00f }, { "highpassMix", "High-pass Mix", 1.00f },
        { "compressorMix", "Compressor Mix", 1.00f }
    }})
        layout.add (std::make_unique<juce::AudioParameterFloat> (ID { item.id, 1 }, item.name,
                                                                  0.0f, 1.0f,
                                                                  item.defaultValue));
    layout.add (std::make_unique<juce::AudioParameterChoice> (ID { "delayType", 1 }, "Delay Type",
                                                               juce::StringArray { "Stereo", "Ping-pong", "Tape", "Diffuse" }, 1));
    layout.add (std::make_unique<juce::AudioParameterChoice> (ID { "delayDivision", 1 }, "Delay Time Mode",
                                                               juce::StringArray { "Free (ms)", "1/1", "1/2", "1/4", "1/8", "1/16", "1/4 dotted", "1/8 dotted", "1/8 triplet" }, 3));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "delayTimeMs", 1 }, "Delay Time",
                                                              juce::NormalisableRange<float> (20.0f, 1500.0f, 1.0f, 0.36f), 350.0f));
    layout.add (std::make_unique<juce::AudioParameterChoice> (ID { "reverbType", 1 }, "Reverb Type",
                                                               juce::StringArray { "Room", "Hall", "Plate", "Cloud" }, 1));
    layout.add (std::make_unique<juce::AudioParameterChoice> (ID { "lowpassType", 1 }, "Low-pass Type",
                                                               juce::StringArray { "Clean", "Warm", "Resonant" }, 0));
    layout.add (std::make_unique<juce::AudioParameterChoice> (ID { "highpassType", 1 }, "High-pass Type",
                                                               juce::StringArray { "Clean", "Warm", "Resonant" }, 0));
    layout.add (std::make_unique<juce::AudioParameterChoice> (ID { "chorusType", 1 }, "Chorus Type",
                                                               juce::StringArray { "Gentle", "Wide", "Ensemble" }, 0));
    layout.add (std::make_unique<juce::AudioParameterChoice> (ID { "crushType", 1 }, "Crush Type",
                                                               juce::StringArray { "Digital", "Vintage", "Glitch" }, 0));
    layout.add (std::make_unique<juce::AudioParameterChoice> (ID { "driveType", 1 }, "Drive Type",
                                                               juce::StringArray { "Soft", "Tube", "Fold", "Clip" }, 0));
    layout.add (std::make_unique<juce::AudioParameterChoice> (ID { "phaserType", 1 }, "Phaser Type",
                                                               juce::StringArray { "Smooth", "Deep", "Fast" }, 0));
    layout.add (std::make_unique<juce::AudioParameterChoice> (ID { "flangerType", 1 }, "Flanger Type",
                                                               juce::StringArray { "Jet", "Tape", "Metal" }, 0));
    layout.add (std::make_unique<juce::AudioParameterChoice> (ID { "tremoloType", 1 }, "Tremolo Type",
                                                               juce::StringArray { "Sine", "Square", "Chop" }, 0));
    layout.add (std::make_unique<juce::AudioParameterChoice> (ID { "compressorType", 1 }, "Compressor Type",
                                                               juce::StringArray { "Glue", "Punch", "Pump" }, 0));

    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "attack", 1 }, "Attack",
                                                              juce::NormalisableRange<float> (0.001f, 5.0f, 0.001f, 0.35f), 0.025f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "decay", 1 }, "Decay",
                                                              juce::NormalisableRange<float> (0.01f, 8.0f, 0.001f, 0.4f), 1.2f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "sustain", 1 }, "Sustain", 0.0f, 1.0f, 0.72f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "release", 1 }, "Release",
                                                              juce::NormalisableRange<float> (0.02f, 12.0f, 0.001f, 0.35f), 2.4f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "tone", 1 }, "Tone", 0.0f, 1.0f, 0.78f));
    layout.add (std::make_unique<juce::AudioParameterInt> (ID { "rootNote", 1 }, "Sample Root", 24, 96, 60));
    layout.add (std::make_unique<juce::AudioParameterInt> (ID { "oneShotARoot", 1 }, "One-shot A Root", 0, 96, 0));
    layout.add (std::make_unique<juce::AudioParameterInt> (ID { "oneShotBRoot", 1 }, "One-shot B Root", 0, 96, 0));
    layout.add (std::make_unique<juce::AudioParameterInt> (ID { "transpose", 1 }, "Transpose", -24, 24, 0));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "fineTune", 1 }, "Fine Tune", -100.0f, 100.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "velocity", 1 }, "Velocity", 0.0f, 1.0f, 1.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "sampleStart", 1 }, "One-shot A Start", 0.0f, 1.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "sampleEnd", 1 }, "One-shot A End", 0.0f, 1.0f, 1.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "sample2Start", 1 }, "One-shot B Start", 0.0f, 1.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "sample2End", 1 }, "One-shot B End", 0.0f, 1.0f, 1.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "textureStart", 1 }, "Texture Start", 0.0f, 1.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "textureRandom", 1 }, "Texture Random Start", 0.0f, 1.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "oneShotALevel", 1 }, "One-shot A Level", -60.0f, 6.0f, -3.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "oneShotBLevel", 1 }, "One-shot B Level", -60.0f, 6.0f, -7.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "textureLevel", 1 }, "Texture Level", -60.0f, 0.0f, -26.0f));
    layout.add (std::make_unique<juce::AudioParameterBool> (ID { "oneShotAEnabled", 1 }, "One-shot A On", true));
    layout.add (std::make_unique<juce::AudioParameterBool> (ID { "oneShotBEnabled", 1 }, "One-shot B On", false));
    layout.add (std::make_unique<juce::AudioParameterBool> (ID { "textureEnabled", 1 }, "Texture On", true));
    layout.add (std::make_unique<juce::AudioParameterInt> (ID { "oneShotATranspose", 1 }, "One-shot A Transpose", -24, 24, 0));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "oneShotAFine", 1 }, "One-shot A Fine", -100.0f, 100.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterInt> (ID { "oneShotBTranspose", 1 }, "One-shot B Transpose", -24, 24, 0));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "oneShotBFine", 1 }, "One-shot B Fine", -100.0f, 100.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterInt> (ID { "textureTranspose", 1 }, "Texture Transpose", -24, 24, 0));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "textureFine", 1 }, "Texture Fine", -100.0f, 100.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterBool> (ID { "reverse", 1 }, "Reverse", false));
    layout.add (std::make_unique<juce::AudioParameterChoice> (ID { "loopMode", 1 }, "One-shot Loop Mode",
                                                               juce::StringArray { "Auto", "One-shot", "Loop Region" }, 1));
    layout.add (std::make_unique<juce::AudioParameterInt> (ID { "voices", 1 }, "Voices", 1, 32, 16));
    layout.add (std::make_unique<juce::AudioParameterBool> (ID { "mono", 1 }, "Mono", false));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "glide", 1 }, "Glide",
                                                              juce::NormalisableRange<float> (0.0f, 2.0f, 0.001f, 0.38f), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "pan", 1 }, "Pan", -1.0f, 1.0f, 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (ID { "output", 1 }, "Output", -24.0f, 6.0f, -3.0f));
    for (const auto& lock : std::array<std::pair<const char*, const char*>, 18>
    {{
        { "lockOneShotA", "Lock One-shot A" }, { "lockOneShotB", "Lock One-shot B" },
        { "lockTexture", "Lock Texture" }, { "lockEnvelope", "Lock Envelope" },
        { "lockShimmer", "Lock Shimmer" }, { "lockVoice", "Lock Voice" },
        { "lockChorus", "Lock Chorus" }, { "lockDelay", "Lock Delay" },
        { "lockReverb", "Lock Reverb" }, { "lockDrive", "Lock Drive" },
        { "lockCrush", "Lock Crush" }, { "lockCompressor", "Lock Compressor" },
        { "lockPhaser", "Lock Phaser" }, { "lockFlanger", "Lock Flanger" },
        { "lockTremolo", "Lock Tremolo" }, { "lockWidth", "Lock Width" },
        { "lockLowpass", "Lock Low-pass" }, { "lockHighpass", "Lock High-pass" }
    }})
        layout.add (std::make_unique<juce::AudioParameterBool> (
            ID { lock.first, 1 }, lock.second, false));
    return layout;
}

void SiedAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<SiedVoice*> (synth.getVoice (i)))
            voice->prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());

    macroEffects.prepare ({ sampleRate, static_cast<juce::uint32> (samplesPerBlock),
                            static_cast<juce::uint32> (getTotalNumOutputChannels()) });
}

void SiedAudioProcessor::releaseResources()
{
    macroEffects.reset();
}

bool SiedAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void SiedAudioProcessor::configureVoiceCount()
{
    const auto mono = parameters.getRawParameterValue ("mono")->load() >= 0.5f;
    const auto requested = juce::jlimit (1, 32, static_cast<int> (std::round (
                                            parameters.getRawParameterValue ("voices")->load())));
    const auto enabledVoices = mono ? 1 : requested;
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<SiedVoice*> (synth.getVoice (i)))
            voice->setVoiceEnabled (i < enabledVoices);
}

void SiedAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    if (resetEffectsRequested.exchange (false, std::memory_order_acq_rel))
        macroEffects.reset();
    buffer.clear();
    configureVoiceCount();
    synth.renderNextBlock (buffer, midi, 0, buffer.getNumSamples());

    std::array<float, static_cast<size_t> (SiedLayer::count)> playheads
    {{ -1.0f, -1.0f, -1.0f }};
    std::array<uint64_t, static_cast<size_t> (SiedLayer::count)> playheadPriorities
    {{ 0, 0, 0 }};
    for (int voiceIndex = 0; voiceIndex < synth.getNumVoices(); ++voiceIndex)
    {
        auto* voice = dynamic_cast<SiedVoice*> (synth.getVoice (voiceIndex));
        if (voice == nullptr)
            continue;
        for (size_t layerNumber = 0; layerNumber < playheads.size(); ++layerNumber)
        {
            const auto candidate = voice->consumePlaybackPosition (
                static_cast<SiedLayer> (layerNumber));
            if (candidate >= 0.0f && voice->getDisplayPriority() >= playheadPriorities[layerNumber])
            {
                playheads[layerNumber] = candidate;
                playheadPriorities[layerNumber] = voice->getDisplayPriority();
            }
        }
    }
    for (size_t layerNumber = 0; layerNumber < playheads.size(); ++layerNumber)
        displayedPlayheads[layerNumber].store (playheads[layerNumber],
                                               std::memory_order_relaxed);

    SiedEffectSettings settings;
    settings.shimmer = parameters.getRawParameterValue ("halo")->load();
    settings.chorus = parameters.getRawParameterValue ("drift")->load();
    settings.delay = parameters.getRawParameterValue ("ghost")->load();
    settings.crush = parameters.getRawParameterValue ("crush")->load();
    settings.reverb = parameters.getRawParameterValue ("reverb")->load();
    settings.drive = parameters.getRawParameterValue ("drive")->load();
    settings.phaser = parameters.getRawParameterValue ("phaser")->load();
    settings.flanger = parameters.getRawParameterValue ("flanger")->load();
    settings.tremolo = parameters.getRawParameterValue ("tremolo")->load();
    settings.width = parameters.getRawParameterValue ("width")->load();
    settings.lowpass = parameters.getRawParameterValue ("lowpass")->load();
    settings.highpass = parameters.getRawParameterValue ("highpass")->load();
    settings.compressor = parameters.getRawParameterValue ("compressor")->load();
    settings.shimmerMix = parameters.getRawParameterValue ("shimmerMix")->load();
    settings.chorusMix = parameters.getRawParameterValue ("chorusMix")->load();
    settings.delayMix = parameters.getRawParameterValue ("delayMix")->load();
    settings.crushMix = parameters.getRawParameterValue ("crushMix")->load();
    settings.reverbMix = parameters.getRawParameterValue ("reverbMix")->load();
    settings.driveMix = parameters.getRawParameterValue ("driveMix")->load();
    settings.phaserMix = parameters.getRawParameterValue ("phaserMix")->load();
    settings.flangerMix = parameters.getRawParameterValue ("flangerMix")->load();
    settings.tremoloMix = parameters.getRawParameterValue ("tremoloMix")->load();
    settings.widthMix = parameters.getRawParameterValue ("widthMix")->load();
    settings.lowpassMix = parameters.getRawParameterValue ("lowpassMix")->load();
    settings.highpassMix = parameters.getRawParameterValue ("highpassMix")->load();
    settings.compressorMix = parameters.getRawParameterValue ("compressorMix")->load();
    settings.delayType = static_cast<int> (parameters.getRawParameterValue ("delayType")->load());
    settings.delayDivision = static_cast<int> (parameters.getRawParameterValue ("delayDivision")->load());
    settings.delayTimeMs = parameters.getRawParameterValue ("delayTimeMs")->load();
    settings.reverbType = static_cast<int> (parameters.getRawParameterValue ("reverbType")->load());
    settings.lowpassType = static_cast<int> (parameters.getRawParameterValue ("lowpassType")->load());
    settings.highpassType = static_cast<int> (parameters.getRawParameterValue ("highpassType")->load());
    settings.chorusType = static_cast<int> (parameters.getRawParameterValue ("chorusType")->load());
    settings.crushType = static_cast<int> (parameters.getRawParameterValue ("crushType")->load());
    settings.driveType = static_cast<int> (parameters.getRawParameterValue ("driveType")->load());
    settings.phaserType = static_cast<int> (parameters.getRawParameterValue ("phaserType")->load());
    settings.flangerType = static_cast<int> (parameters.getRawParameterValue ("flangerType")->load());
    settings.tremoloType = static_cast<int> (parameters.getRawParameterValue ("tremoloType")->load());
    settings.compressorType = static_cast<int> (parameters.getRawParameterValue ("compressorType")->load());
    if (auto* playHead = getPlayHead())
        if (auto position = playHead->getPosition())
            if (auto bpm = position->getBpm())
                settings.tempoBpm = static_cast<float> (*bpm);
    settings.pan = parameters.getRawParameterValue ("pan")->load();
    settings.outputGainDb = parameters.getRawParameterValue ("output")->load();
    macroEffects.process (buffer, settings);
}

juce::AudioProcessorEditor* SiedAudioProcessor::createEditor()
{
    return new SiedAudioProcessorEditor (*this);
}

bool SiedAudioProcessor::installReader (std::unique_ptr<juce::AudioFormatReader> reader,
                                        SiedLayer layer, const juce::String& name, float gainDb,
                                        const juce::String& sourcePath, bool autoNormalise,
                                        int loopStart, int loopEnd, bool detectRoot)
{
    if (reader == nullptr || reader->lengthInSamples <= 1)
        return false;

    const auto maximumLengthSeconds = layer == SiedLayer::texture ? 120.0 : 60.0;
    const auto maximumSamples = static_cast<juce::int64> (reader->sampleRate * maximumLengthSeconds);
    const auto length = static_cast<int> (juce::jmin (reader->lengthInSamples, maximumSamples));
    auto sample = std::make_shared<SiedSampleData>();
    sample->audio.setSize (2, length);
    if (! reader->read (&sample->audio, 0, length, 0, true, true))
        return false;

    sample->sourceSampleRate = reader->sampleRate;
    if (autoNormalise)
    {
        float peak = 0.0f;
        for (int channel = 0; channel < sample->audio.getNumChannels(); ++channel)
            peak = juce::jmax (peak, sample->audio.getMagnitude (channel, 0, length));
        const auto targetDb = layer == SiedLayer::texture ? -12.0f : -6.0f;
        const auto targetPeak = juce::Decibels::decibelsToGain (targetDb);
        sample->gainLinear = peak > 0.000001f
                                 ? juce::jlimit (0.125f, 4.0f, targetPeak / peak) : 1.0f;
    }
    else
    {
        sample->gainLinear = juce::Decibels::decibelsToGain (gainDb);
    }

    sample->loopStart = juce::jlimit (0, length - 2, loopStart);
    sample->loopEnd = juce::jlimit (0, length - 1, loopEnd);
    sample->loopEnabled = sample->loopEnd - sample->loopStart > 8192;
    sample->loopCrossfade = sample->loopEnabled
                                ? juce::jmin (4096, (sample->loopEnd - sample->loopStart) / 4) : 0;
    if (layer == SiedLayer::texture)
    {
        sample->loopStart = 0;
        sample->loopEnd = length - 1;
        sample->loopEnabled = true;
        sample->loopCrossfade = juce::jmin (16384, length / 6);
    }
    sample->displayName = name;
    sampleBank.setSample (layer, sample);
    const auto index = static_cast<size_t> (layer);
    loadedSamplePaths[index] = sourcePath;
    currentSoundNames[index] = sample->displayName;
    if (detectRoot && layer != SiedLayer::texture)
    {
        const auto rootID = layer == SiedLayer::oneShotA ? "oneShotARoot"
                                                          : "oneShotBRoot";
        const auto fineID = layer == SiedLayer::oneShotA ? "oneShotAFine"
                                                          : "oneShotBFine";
        if (const auto detected = detectRootNote (sample->audio, sample->sourceSampleRate))
        {
            setStateParameter (parameters, rootID, static_cast<float> (detected->midiNote));
            setStateParameter (parameters, fineID, detected->correctionCents);
            const auto note = juce::MidiMessage::getMidiNoteName (
                detected->midiNote, true, true, 4);
            const auto cents = juce::roundToInt (detected->correctionCents);
            lastDetectedRootText = "AUTO " + note + "  "
                                 + (cents > 0 ? "+" : "") + juce::String (cents) + " ct";
        }
        else
        {
            // A low-confidence result is less useful than no result. Fall back to
            // the global root and neutral fine tune instead of retuning the sample.
            setStateParameter (parameters, rootID, 0.0f);
            setStateParameter (parameters, fineID, 0.0f);
            lastDetectedRootText = "TUNING MANUAL";
        }
    }
    return true;
}

bool SiedAudioProcessor::loadSampleFile (const juce::File& file, SiedLayer layer)
{
    return loadSampleFileInternal (file, layer, true);
}

bool SiedAudioProcessor::loadSampleFileInternal (const juce::File& file, SiedLayer layer,
                                                  bool resetPlaybackRegion)
{
    if (! file.existsAsFile() || ! isSupportedAudioFile (file))
        return false;

    auto reader = std::unique_ptr<juce::AudioFormatReader> (formatManager.createReaderFor (file));
    if (! installReader (std::move (reader), layer, file.getFileNameWithoutExtension(), 0.0f,
                         file.getFullPathName(), true, 0, 0, resetPlaybackRegion))
        return false;

    if (layer == SiedLayer::oneShotA || layer == SiedLayer::oneShotB)
    {
        const auto slot = layer == SiedLayer::oneShotA ? 0 : 1;
        selectedOneShots[static_cast<size_t> (slot)] = -1;
        for (int i = 0; i < getOneShotCount(); ++i)
            if (oneShotLibrary[static_cast<size_t> (i)].file == file)
                selectedOneShots[static_cast<size_t> (slot)] = i;
        if (resetPlaybackRegion)
        {
            setStateParameter (parameters, slot == 0 ? "sampleStart" : "sample2Start", 0.0f);
            setStateParameter (parameters, slot == 0 ? "sampleEnd" : "sample2End", 1.0f);
        }
    }
    else
    {
        selectedTexture = -1;
        for (int i = 0; i < getTextureCount(); ++i)
            if (textureLibrary[static_cast<size_t> (i)].file == file)
                selectedTexture = i;
        if (resetPlaybackRegion)
        {
            setStateParameter (parameters, "textureStart", 0.0f);
            setStateParameter (parameters, "textureRandom", 0.42f);
        }
    }
    return true;
}

bool SiedAudioProcessor::loadFactoryToLayer (int presetIndex, SiedLayer layer,
                                              bool applyPresetParameters)
{
    if (! juce::isPositiveAndBelow (presetIndex, factoryPresetCount)
        || layer == SiedLayer::texture)
        return false;

    const auto& preset = factoryPresets[presetIndex];
    auto stream = std::make_unique<juce::MemoryInputStream> (preset.data,
                                                             static_cast<size_t> (preset.dataSize), false);
    auto reader = std::unique_ptr<juce::AudioFormatReader> (formatManager.createReaderFor (std::move (stream)));
    if (! installReader (std::move (reader), layer, preset.name, preset.gainDb, {}, false,
                         preset.loopStart, preset.loopEnd, true))
        return false;

    const auto slot = layer == SiedLayer::oneShotA ? 0 : 1;
    selectedOneShots[static_cast<size_t> (slot)] = presetIndex;
    setStateParameter (parameters, slot == 0 ? "oneShotARoot" : "oneShotBRoot",
                       static_cast<float> (preset.rootNote));
    if (applyPresetParameters)
    {
        setStateParameter (parameters, "rootNote", static_cast<float> (preset.rootNote));
        setStateParameter (parameters, "halo", preset.halo);
        setStateParameter (parameters, "drift", preset.drift);
        setStateParameter (parameters, "ghost", preset.ghost);
        setStateParameter (parameters, "crush", juce::jmin (0.08f, preset.crush));
        setStateParameter (parameters, "attack", preset.attack);
        setStateParameter (parameters, "release", preset.release);
        setStateParameter (parameters, "tone", preset.tone);
        setStateParameter (parameters, "sampleStart", 0.0f);
        setStateParameter (parameters, "sampleEnd", 1.0f);
        setStateParameter (parameters, "transpose", 0.0f);
        setStateParameter (parameters, "fineTune", 0.0f);
    }
    return true;
}

bool SiedAudioProcessor::loadEmbeddedTexture()
{
    auto stream = std::make_unique<juce::MemoryInputStream> (SiedFactoryData::texture_vinyl_wav,
                         static_cast<size_t> (SiedFactoryData::texture_vinyl_wavSize), false);
    auto reader = std::unique_ptr<juce::AudioFormatReader> (formatManager.createReaderFor (std::move (stream)));
    if (! installReader (std::move (reader), SiedLayer::texture, "Vinyl Crackles", -12.0f,
                         {}, false))
        return false;
    selectedTexture = 0;
    return true;
}

bool SiedAudioProcessor::loadFactoryPreset (int presetIndex)
{
    return loadFactoryToLayer (presetIndex, SiedLayer::oneShotA, true);
}

bool SiedAudioProcessor::loadOneShot (SiedLayer layer, int libraryIndex)
{
    if ((layer != SiedLayer::oneShotA && layer != SiedLayer::oneShotB)
        || ! juce::isPositiveAndBelow (libraryIndex, getOneShotCount()))
        return false;

    const auto& entry = oneShotLibrary[static_cast<size_t> (libraryIndex)];
    const auto loaded = entry.embeddedIndex >= 0
                            ? loadFactoryToLayer (entry.embeddedIndex, layer, false)
                            : loadSampleFileInternal (entry.file, layer, true);
    if (loaded)
    {
        const auto slot = layer == SiedLayer::oneShotA ? 0 : 1;
        selectedOneShots[static_cast<size_t> (slot)] = libraryIndex;
    }
    return loaded;
}

bool SiedAudioProcessor::loadTexture (int libraryIndex)
{
    if (! juce::isPositiveAndBelow (libraryIndex, getTextureCount()))
        return false;
    const auto& entry = textureLibrary[static_cast<size_t> (libraryIndex)];
    const auto loaded = entry.embeddedIndex == 0
                            ? loadEmbeddedTexture()
                            : loadSampleFileInternal (entry.file, SiedLayer::texture, true);
    if (loaded)
        selectedTexture = libraryIndex;
    return loaded;
}

bool SiedAudioProcessor::recallOneShot (SiedLayer layer, int libraryIndex)
{
    if ((layer != SiedLayer::oneShotA && layer != SiedLayer::oneShotB)
        || ! juce::isPositiveAndBelow (libraryIndex, getOneShotCount()))
        return false;
    const auto& entry = oneShotLibrary[static_cast<size_t> (libraryIndex)];
    const auto loaded = entry.embeddedIndex >= 0
                            ? loadFactoryToLayer (entry.embeddedIndex, layer, false)
                            : loadSampleFileInternal (entry.file, layer, false);
    if (loaded)
        selectedOneShots[layer == SiedLayer::oneShotA ? 0 : 1] = libraryIndex;
    return loaded;
}

bool SiedAudioProcessor::recallTexture (int libraryIndex)
{
    if (! juce::isPositiveAndBelow (libraryIndex, getTextureCount()))
        return false;
    const auto& entry = textureLibrary[static_cast<size_t> (libraryIndex)];
    const auto loaded = entry.embeddedIndex == 0
                            ? loadEmbeddedTexture()
                            : loadSampleFileInternal (entry.file, SiedLayer::texture, false);
    if (loaded)
        selectedTexture = libraryIndex;
    return loaded;
}

bool SiedAudioProcessor::stepOneShot (SiedLayer layer, int direction)
{
    if ((layer != SiedLayer::oneShotA && layer != SiedLayer::oneShotB)
        || oneShotLibrary.empty() || direction == 0)
        return false;

    const auto slot = layer == SiedLayer::oneShotA ? 0 : 1;
    auto current = selectedOneShots[static_cast<size_t> (slot)];
    if (! juce::isPositiveAndBelow (current, getOneShotCount()))
        current = direction > 0 ? -1 : 0;
    const auto next = (current + (direction > 0 ? 1 : -1) + getOneShotCount())
                      % getOneShotCount();
    return loadOneShot (layer, next);
}

bool SiedAudioProcessor::stepTexture (int direction)
{
    if (textureLibrary.empty() || direction == 0)
        return false;

    auto current = selectedTexture;
    if (! juce::isPositiveAndBelow (current, getTextureCount()))
        current = direction > 0 ? -1 : 0;
    const auto next = (current + (direction > 0 ? 1 : -1) + getTextureCount())
                      % getTextureCount();
    return loadTexture (next);
}

void SiedAudioProcessor::randomizeOneShots()
{
    captureRandomizationUndo();
    auto& random = juce::Random::getSystemRandom();
    randomizeOneShots (random);
}

void SiedAudioProcessor::randomizeOneShots (juce::Random& random)
{
    if (oneShotLibrary.empty())
        return;

    auto first = selectedOneShots[0];
    auto second = selectedOneShots[1];
    if (! isLocked ("lockOneShotA"))
        first = random.nextInt (getOneShotCount());
    if (! isLocked ("lockOneShotB"))
        second = random.nextInt (getOneShotCount());
    if (! isLocked ("lockOneShotA") && ! isLocked ("lockOneShotB")
        && getOneShotCount() > 1 && second == first)
        second = (second + 1) % getOneShotCount();

    if (! isLocked ("lockOneShotA"))
    {
        loadOneShot (SiedLayer::oneShotA, first);
        setStateParameter (parameters, "sampleStart",
                           random.nextBool() ? 0.0f : random.nextFloat() * 0.08f);
        setStateParameter (parameters, "sampleEnd", 0.86f + random.nextFloat() * 0.14f);
        setStateParameter (parameters, "oneShotALevel", -4.5f + random.nextFloat() * 3.0f);
        setStateParameter (parameters, "oneShotAEnabled", 1.0f);
        setStateParameter (parameters, "oneShotATranspose", 0.0f);
    }
    if (! isLocked ("lockOneShotB"))
    {
        loadOneShot (SiedLayer::oneShotB, second);
        setStateParameter (parameters, "sample2Start",
                           random.nextBool() ? 0.0f : random.nextFloat() * 0.08f);
        setStateParameter (parameters, "sample2End", 0.86f + random.nextFloat() * 0.14f);
        setStateParameter (parameters, "oneShotBLevel", -10.0f + random.nextFloat() * 5.0f);
        setStateParameter (parameters, "oneShotBEnabled", 1.0f);
        setStateParameter (parameters, "oneShotBTranspose", 0.0f);
    }
}

void SiedAudioProcessor::randomizeTexture()
{
    captureRandomizationUndo();
    auto& random = juce::Random::getSystemRandom();
    randomizeTexture (random);
}

void SiedAudioProcessor::randomizeTexture (juce::Random& random)
{
    if (textureLibrary.empty() || isLocked ("lockTexture"))
        return;
    loadTexture (random.nextInt (getTextureCount()));
    setStateParameter (parameters, "textureStart", random.nextFloat() * 0.95f);
    setStateParameter (parameters, "textureRandom", 0.18f + random.nextFloat() * 0.72f);
    setStateParameter (parameters, "textureLevel", -20.0f + random.nextFloat() * 6.0f);
    setStateParameter (parameters, "textureEnabled", 1.0f);
    setStateParameter (parameters, "textureTranspose", 0.0f);
    setStateParameter (parameters, "textureFine", 0.0f);
}

void SiedAudioProcessor::randomizePreset()
{
    captureRandomizationUndo();
    auto& random = juce::Random::getSystemRandom();
    randomizeOneShots (random);
    randomizeTexture (random);

    const auto randomiseSubtleEffect = [&] (const char* amountID, const char* mixID,
                                            const char* lockID,
                                            float probability, float minimumAmount,
                                            float maximumAmount, float minimumMix,
                                            float maximumMix)
    {
        if (isLocked (lockID))
            return;
        setStateParameter (parameters, amountID,
                           juce::String (amountID) == "width" ? 1.0f : 0.0f);
        setStateParameter (parameters, mixID, 0.0f);
        if (random.nextFloat() >= probability)
            return;
        setStateParameter (parameters, amountID,
                           minimumAmount + random.nextFloat() * (maximumAmount - minimumAmount));
        setStateParameter (parameters, mixID,
                           minimumMix + random.nextFloat() * (maximumMix - minimumMix));
    };
    randomiseSubtleEffect ("halo", "shimmerMix", "lockShimmer", 0.30f, 0.04f, 0.18f, 0.14f, 0.32f);
    randomiseSubtleEffect ("drift", "chorusMix", "lockChorus", 0.42f, 0.05f, 0.23f, 0.16f, 0.34f);
    randomiseSubtleEffect ("ghost", "delayMix", "lockDelay", 0.25f, 0.04f, 0.17f, 0.12f, 0.27f);
    randomiseSubtleEffect ("reverb", "reverbMix", "lockReverb", 0.48f, 0.035f, 0.14f, 0.10f, 0.24f);
    randomiseSubtleEffect ("drive", "driveMix", "lockDrive", 0.14f, 0.03f, 0.11f, 0.10f, 0.22f);
    randomiseSubtleEffect ("crush", "crushMix", "lockCrush", 0.08f, 0.02f, 0.07f, 0.08f, 0.18f);
    randomiseSubtleEffect ("phaser", "phaserMix", "lockPhaser", 0.20f, 0.04f, 0.16f, 0.12f, 0.26f);
    randomiseSubtleEffect ("flanger", "flangerMix", "lockFlanger", 0.14f, 0.03f, 0.12f, 0.10f, 0.22f);
    randomiseSubtleEffect ("tremolo", "tremoloMix", "lockTremolo", 0.13f, 0.03f, 0.12f, 0.10f, 0.22f);
    randomiseSubtleEffect ("width", "widthMix", "lockWidth", 0.30f, 0.86f, 1.15f, 0.14f, 0.30f);
    randomiseSubtleEffect ("lowpass", "lowpassMix", "lockLowpass", 0.16f, 0.03f, 0.13f, 0.14f, 0.30f);
    randomiseSubtleEffect ("highpass", "highpassMix", "lockHighpass", 0.10f, 0.02f, 0.08f, 0.10f, 0.24f);
    randomiseSubtleEffect ("compressor", "compressorMix", "lockCompressor", 0.24f, 0.04f, 0.16f, 0.14f, 0.30f);

    if (! isLocked ("lockDelay"))
    {
        setStateParameter (parameters, "delayType", static_cast<float> (random.nextInt (4)));
        setStateParameter (parameters, "delayDivision", static_cast<float> (2 + random.nextInt (5)));
        setStateParameter (parameters, "delayTimeMs", 120.0f + random.nextFloat() * 480.0f);
    }
    if (! isLocked ("lockReverb")) setStateParameter (parameters, "reverbType", static_cast<float> (random.nextInt (4)));
    if (! isLocked ("lockLowpass")) setStateParameter (parameters, "lowpassType", static_cast<float> (random.nextInt (3)));
    if (! isLocked ("lockHighpass")) setStateParameter (parameters, "highpassType", static_cast<float> (random.nextInt (3)));
    if (! isLocked ("lockChorus")) setStateParameter (parameters, "chorusType", static_cast<float> (random.nextInt (3)));
    if (! isLocked ("lockCrush")) setStateParameter (parameters, "crushType", static_cast<float> (random.nextInt (3)));
    if (! isLocked ("lockDrive")) setStateParameter (parameters, "driveType", static_cast<float> (random.nextInt (4)));
    if (! isLocked ("lockPhaser")) setStateParameter (parameters, "phaserType", static_cast<float> (random.nextInt (3)));
    if (! isLocked ("lockFlanger")) setStateParameter (parameters, "flangerType", static_cast<float> (random.nextInt (3)));
    if (! isLocked ("lockTremolo")) setStateParameter (parameters, "tremoloType", static_cast<float> (random.nextInt (3)));
    if (! isLocked ("lockCompressor")) setStateParameter (parameters, "compressorType", static_cast<float> (random.nextInt (3)));

    if (! isLocked ("lockEnvelope"))
    {
        const auto attackRoll = random.nextFloat();
        const auto attackPosition = attackRoll < 0.72f ? 0.05f + random.nextFloat() * 0.10f
                                  : attackRoll < 0.92f ? 0.15f + random.nextFloat() * 0.19f
                                                       : 0.34f + random.nextFloat() * 0.28f;
        setStateParameterNormalised (parameters, "attack", attackPosition);
        const auto releaseRoll = random.nextFloat();
        const auto releasePosition = releaseRoll < 0.80f ? 0.10f + random.nextFloat() * 0.30f
                                   : releaseRoll < 0.96f ? 0.40f + random.nextFloat() * 0.30f
                                                        : 0.70f + random.nextFloat() * 0.25f;
        setStateParameterNormalised (parameters, "release", releasePosition);
    }
    if (! isLocked ("lockVoice"))
    {
        setStateParameter (parameters, "pan", 0.0f);
        setStateParameter (parameters, "output", -4.5f);
    }
    requestEffectsReset();
}

void SiedAudioProcessor::captureRandomizationUndo()
{
    randomizationUndoHistory.push_back (createStateTree());
    if (randomizationUndoHistory.size() > 32)
        randomizationUndoHistory.erase (randomizationUndoHistory.begin());
}

bool SiedAudioProcessor::undoLastRandomization()
{
    if (randomizationUndoHistory.empty())
        return false;
    auto state = randomizationUndoHistory.back().createCopy();
    randomizationUndoHistory.pop_back();
    restoreStateTree (std::move (state));
    return true;
}

void SiedAudioProcessor::resetToInitPatch()
{
    for (auto* parameter : AudioProcessor::getParameters())
        parameter->setValueNotifyingHost (parameter->getDefaultValue());

    auto topograph = findOneShotByName ("key - topograph", true);
    if (topograph < 0)
        topograph = findOneShotByName ("Topograph", true);
    if (topograph >= 0)
    {
        loadOneShot (SiedLayer::oneShotA, topograph);
        const auto embedded = oneShotLibrary[static_cast<size_t> (topograph)].embeddedIndex;
        setStateParameter (parameters, "rootNote", embedded == 6 ? 72.0f : 60.0f);
    }

    sampleBank.setSample (SiedLayer::oneShotB, {});
    selectedOneShots[1] = -1;
    loadedSamplePaths[1].clear();
    currentSoundNames[1] = "OFF";

    auto forest = findTextureByName ("Forest", true);
    if (forest < 0)
        forest = findTextureByName ("Forest", false);
    if (forest >= 0)
        loadTexture (forest);
    else
        loadEmbeddedTexture();
    setStateParameter (parameters, "textureStart", 0.0f);
    setStateParameter (parameters, "textureRandom", 0.0f);

    resetEffectsRequested.store (true, std::memory_order_release);
}

int SiedAudioProcessor::findOneShotByName (const juce::String& name, bool exact) const
{
    for (int i = 0; i < getOneShotCount(); ++i)
    {
        const auto& candidate = oneShotLibrary[static_cast<size_t> (i)].name;
        if (exact ? candidate.equalsIgnoreCase (name)
                  : candidate.containsIgnoreCase (name))
            return i;
    }
    return -1;
}

int SiedAudioProcessor::findTextureByName (const juce::String& name, bool exact) const
{
    for (int i = 0; i < getTextureCount(); ++i)
    {
        const auto& candidate = textureLibrary[static_cast<size_t> (i)].name;
        if (exact ? candidate.equalsIgnoreCase (name)
                  : candidate.containsIgnoreCase (name))
            return i;
    }
    return -1;
}

void SiedAudioProcessor::addExternalLibraryFiles (const juce::File& folder, bool textures)
{
    if (! folder.isDirectory())
        return;
    auto& destination = textures ? textureLibrary : oneShotLibrary;
    for (const auto& entry : juce::RangedDirectoryIterator (folder, true, "*", juce::File::findFiles))
    {
        const auto file = entry.getFile();
        if (! isSupportedAudioFile (file))
            continue;
        LibraryEntry item;
        item.name = file.getFileNameWithoutExtension();
        item.category = textures ? textureCategoryForName (item.name)
                                 : oneShotCategoryForName (item.name);
        item.file = file;

        // Keep the first external copy. The shared factory bank is scanned
        // before the user library so corrected factory masters stay authoritative.
        const auto duplicate = std::find_if (destination.begin(), destination.end(),
                                             [&item] (const LibraryEntry& existing)
                                             {
                                                 return existing.file != juce::File {}
                                                     && existing.name.equalsIgnoreCase (item.name);
                                             });
        if (duplicate != destination.end())
            continue;

        destination.push_back (std::move (item));
    }
}

void SiedAudioProcessor::rescanLibrary()
{
    oneShotLibrary.clear();
    textureLibrary.clear();
    for (int i = 0; i < factoryPresetCount; ++i)
        oneShotLibrary.push_back ({ factoryPresets[i].name, factoryPresets[i].category, {}, i });
    textureLibrary.push_back ({ "Vinyl Crackles", "Vinyl + Tape", {}, 0 });

    const auto oneShotEmbeddedCount = oneShotLibrary.size();
    const auto textureEmbeddedCount = textureLibrary.size();
    const auto sharedRoot = juce::File::getSpecialLocation (juce::File::commonApplicationDataDirectory)
                                .getChildFile ("SIED").getChildFile ("Library");
    addExternalLibraryFiles (sharedRoot.getChildFile ("Oneshots"), false);
    addExternalLibraryFiles (sharedRoot.getChildFile ("Textures"), true);

    const auto userRoot = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                              .getChildFile ("SIED").getChildFile ("Library");
    addExternalLibraryFiles (userRoot.getChildFile ("Oneshots"), false);
    addExternalLibraryFiles (userRoot.getChildFile ("Textures"), true);

    const auto sortEntries = [] (const LibraryEntry& a, const LibraryEntry& b)
    {
        const auto categoryComparison = a.category.compareNatural (b.category);
        return categoryComparison == 0 ? a.name.compareNatural (b.name) < 0
                                       : categoryComparison < 0;
    };
    std::sort (oneShotLibrary.begin() + static_cast<std::ptrdiff_t> (oneShotEmbeddedCount),
               oneShotLibrary.end(), sortEntries);
    std::sort (textureLibrary.begin() + static_cast<std::ptrdiff_t> (textureEmbeddedCount),
               textureLibrary.end(), sortEntries);
}

int SiedAudioProcessor::getOneShotCount() const
{
    return static_cast<int> (oneShotLibrary.size());
}

juce::String SiedAudioProcessor::getOneShotName (int index) const
{
    return juce::isPositiveAndBelow (index, getOneShotCount())
               ? oneShotLibrary[static_cast<size_t> (index)].name : juce::String {};
}

juce::String SiedAudioProcessor::getOneShotCategory (int index) const
{
    return juce::isPositiveAndBelow (index, getOneShotCount())
               ? oneShotLibrary[static_cast<size_t> (index)].category : juce::String {};
}

int SiedAudioProcessor::getTextureCount() const
{
    return static_cast<int> (textureLibrary.size());
}

juce::String SiedAudioProcessor::getTextureName (int index) const
{
    return juce::isPositiveAndBelow (index, getTextureCount())
               ? textureLibrary[static_cast<size_t> (index)].name : juce::String {};
}

juce::String SiedAudioProcessor::getTextureCategory (int index) const
{
    return juce::isPositiveAndBelow (index, getTextureCount())
               ? textureLibrary[static_cast<size_t> (index)].category : juce::String {};
}

int SiedAudioProcessor::getSelectedOneShot (SiedLayer layer) const
{
    if (layer != SiedLayer::oneShotA && layer != SiedLayer::oneShotB)
        return -1;
    return selectedOneShots[layer == SiedLayer::oneShotA ? 0 : 1];
}

juce::String SiedAudioProcessor::getCurrentSoundName (SiedLayer layer) const
{
    return currentSoundNames[static_cast<size_t> (layer)];
}

juce::String SiedAudioProcessor::getLibraryFolderPath() const
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("SIED").getChildFile ("Library").getFullPathName();
}

bool SiedAudioProcessor::isLocked (const char* lockID) const
{
    if (const auto* value = parameters.getRawParameterValue (lockID))
        return value->load() >= 0.5f;
    return false;
}

bool SiedAudioProcessor::isRandomizationLocked (const juce::String& parameterID) const
{
    return isLocked (parameterID.toRawUTF8());
}

juce::String SiedAudioProcessor::getLastDetectedRootText() const
{
    return lastDetectedRootText;
}

juce::File SiedAudioProcessor::getFavouritesFile() const
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("SIED").getChildFile ("favorites.txt");
}

juce::String SiedAudioProcessor::favouriteKey (SiedLayer layer, int libraryIndex) const
{
    const auto texture = layer == SiedLayer::texture;
    if (texture ? ! juce::isPositiveAndBelow (libraryIndex, getTextureCount())
                : ! juce::isPositiveAndBelow (libraryIndex, getOneShotCount()))
        return {};
    const auto category = texture ? getTextureCategory (libraryIndex)
                                  : getOneShotCategory (libraryIndex);
    const auto name = texture ? getTextureName (libraryIndex)
                              : getOneShotName (libraryIndex);
    return juce::String (texture ? "T|" : "O|")
           + category.toLowerCase() + "/" + name.toLowerCase();
}

bool SiedAudioProcessor::isFavourite (SiedLayer layer, int libraryIndex) const
{
    const auto key = favouriteKey (layer, libraryIndex);
    return key.isNotEmpty() && favouriteKeys.contains (key);
}

void SiedAudioProcessor::toggleFavourite (SiedLayer layer, int libraryIndex)
{
    const auto key = favouriteKey (layer, libraryIndex);
    if (key.isEmpty())
        return;
    const auto index = favouriteKeys.indexOf (key);
    if (index >= 0)
        favouriteKeys.remove (index);
    else
        favouriteKeys.add (key);
    favouriteKeys.sort (true);
    saveFavourites();
}

void SiedAudioProcessor::loadFavourites()
{
    favouriteKeys.clear();
    const auto file = getFavouritesFile();
    if (file.existsAsFile())
        favouriteKeys.addLines (file.loadFileAsString());
    favouriteKeys.trim();
    favouriteKeys.removeEmptyStrings();
}

void SiedAudioProcessor::saveFavourites() const
{
    const auto file = getFavouritesFile();
    if (file.getParentDirectory().createDirectory().wasOk())
        file.replaceWithText (favouriteKeys.joinIntoString ("\n") + "\n");
}

std::optional<SiedAudioProcessor::RootDetection> SiedAudioProcessor::detectRootNote (
    const juce::AudioBuffer<float>& audio, double sampleRate)
{
    if (audio.getNumSamples() < 2048 || sampleRate <= 0.0)
        return std::nullopt;

    const auto stride = juce::jmax (1, juce::roundToInt (sampleRate / 12000.0));
    const auto analysisRate = sampleRate / static_cast<double> (stride);
    const auto available = juce::jmin (audio.getNumSamples() / stride, 98304);
    if (available < 2048)
        return std::nullopt;

    std::vector<float> mono (static_cast<size_t> (available), 0.0f);
    for (int i = 0; i < available; ++i)
    {
        const auto sourceIndex = i * stride;
        for (int channel = 0; channel < audio.getNumChannels(); ++channel)
            mono[static_cast<size_t> (i)] += audio.getSample (channel, sourceIndex);
        mono[static_cast<size_t> (i)] /= static_cast<float> (audio.getNumChannels());
    }

    const auto windowSize = juce::jmin (4096, available);
    struct FrameCandidate { int start = 0; double energy = 0.0; };
    std::vector<FrameCandidate> candidates;
    for (int start = 0; start + windowSize <= available; start += 1024)
    {
        double energy = 0.0;
        for (int i = 0; i < windowSize; i += 4)
        {
            const auto value = mono[static_cast<size_t> (start + i)];
            energy += static_cast<double> (value * value);
        }
        candidates.push_back ({ start, energy });
    }
    if (candidates.empty())
        return std::nullopt;
    std::sort (candidates.begin(), candidates.end(), [] (const auto& a, const auto& b)
    {
        return a.energy > b.energy;
    });
    if (candidates.front().energy < 1.0e-7)
        return std::nullopt;

    const auto analyseFrame = [&] (int frameStart)
        -> std::optional<std::pair<double, float>>
    {
        std::vector<float> frame (static_cast<size_t> (windowSize));
        double mean = 0.0;
        for (int i = 0; i < windowSize; ++i)
            mean += mono[static_cast<size_t> (frameStart + i)];
        mean /= static_cast<double> (windowSize);
        for (int i = 0; i < windowSize; ++i)
        {
            const auto window = 0.5f - 0.5f * std::cos (
                juce::MathConstants<float>::twoPi * static_cast<float> (i)
                / static_cast<float> (windowSize - 1));
            frame[static_cast<size_t> (i)] =
                (mono[static_cast<size_t> (frameStart + i)] - static_cast<float> (mean)) * window;
        }

        const auto minimumLag = juce::jmax (2, juce::roundToInt (analysisRate / 1800.0));
        const auto maximumLag = juce::jmin (windowSize / 2 - 2,
                                           juce::roundToInt (analysisRate / 38.0));
        if (maximumLag <= minimumLag + 2)
            return std::nullopt;

        std::vector<double> difference (static_cast<size_t> (maximumLag + 1), 0.0);
        std::vector<double> normalised (static_cast<size_t> (maximumLag + 1), 1.0);
        for (int lag = 1; lag <= maximumLag; ++lag)
            for (int i = 0; i < windowSize - lag; ++i)
            {
                const auto delta = static_cast<double> (frame[static_cast<size_t> (i)]
                                  - frame[static_cast<size_t> (i + lag)]);
                difference[static_cast<size_t> (lag)] += delta * delta;
            }
        double running = 0.0;
        for (int lag = 1; lag <= maximumLag; ++lag)
        {
            running += difference[static_cast<size_t> (lag)];
            normalised[static_cast<size_t> (lag)] = running > 0.0
                ? difference[static_cast<size_t> (lag)] * lag / running : 1.0;
        }

        auto bestLag = -1;
        for (int lag = minimumLag + 1; lag < maximumLag; ++lag)
            if (normalised[static_cast<size_t> (lag)] < 0.12
                && normalised[static_cast<size_t> (lag)]
                       <= normalised[static_cast<size_t> (lag - 1)]
                && normalised[static_cast<size_t> (lag)]
                       < normalised[static_cast<size_t> (lag + 1)])
            {
                bestLag = lag;
                break;
            }
        if (bestLag < 0)
        {
            bestLag = minimumLag;
            for (int lag = minimumLag + 1; lag <= maximumLag; ++lag)
                if (normalised[static_cast<size_t> (lag)]
                    < normalised[static_cast<size_t> (bestLag)])
                    bestLag = lag;
        }

        const auto score = normalised[static_cast<size_t> (bestLag)];
        const auto confidence = static_cast<float> (juce::jlimit (0.0, 1.0, 1.0 - score));
        if (confidence < 0.82f || score > 0.18)
            return std::nullopt;

        auto refinedLag = static_cast<double> (bestLag);
        if (bestLag > minimumLag && bestLag < maximumLag)
        {
            const auto left = normalised[static_cast<size_t> (bestLag - 1)];
            const auto centre = normalised[static_cast<size_t> (bestLag)];
            const auto right = normalised[static_cast<size_t> (bestLag + 1)];
            const auto denominator = left - 2.0 * centre + right;
            if (std::abs (denominator) > 1.0e-12)
                refinedLag += 0.5 * (left - right) / denominator;
        }
        const auto frequency = analysisRate / refinedLag;
        if (frequency < 38.0 || frequency > 1800.0)
            return std::nullopt;
        return std::make_pair (69.0 + 12.0 * std::log2 (frequency / 440.0),
                               confidence);
    };

    std::vector<std::pair<double, float>> pitches;
    std::vector<int> selectedStarts;
    for (const auto& candidate : candidates)
    {
        const auto overlaps = std::any_of (selectedStarts.begin(), selectedStarts.end(),
                                           [&] (int start)
                                           {
                                               return std::abs (start - candidate.start)
                                                      < windowSize / 2;
                                           });
        if (overlaps)
            continue;
        selectedStarts.push_back (candidate.start);
        if (const auto pitch = analyseFrame (candidate.start))
            pitches.push_back (*pitch);
        if (selectedStarts.size() >= 7)
            break;
    }
    if (pitches.size() < 3)
        return std::nullopt;

    std::sort (pitches.begin(), pitches.end(), [] (const auto& a, const auto& b)
    {
        return a.first < b.first;
    });
    const auto median = pitches[pitches.size() / 2].first;
    double weightedPitch = 0.0;
    double weightTotal = 0.0;
    std::vector<double> inliers;
    for (const auto& pitch : pitches)
        if (std::abs (pitch.first - median) <= 0.35)
        {
            weightedPitch += pitch.first * pitch.second;
            weightTotal += pitch.second;
            inliers.push_back (pitch.first);
        }
    if (inliers.size() < 3 || inliers.size() * 2 < pitches.size() || weightTotal <= 0.0)
        return std::nullopt;

    const auto pitch = weightedPitch / weightTotal;
    double variance = 0.0;
    for (const auto value : inliers)
        variance += (value - pitch) * (value - pitch);
    const auto deviation = std::sqrt (variance / static_cast<double> (inliers.size()));
    if (deviation > 0.16)
        return std::nullopt;

    const auto midiNote = juce::jlimit (24, 96, juce::roundToInt (pitch));
    const auto sourceOffsetCents = static_cast<float> ((pitch - midiNote) * 100.0);
    const auto averageConfidence = static_cast<float> (weightTotal / inliers.size());
    return RootDetection { midiNote,
                           juce::jlimit (-100.0f, 100.0f, -sourceOffsetCents),
                           averageConfidence };
}

int SiedAudioProcessor::getFactoryPresetCount() const
{
    return factoryPresetCount;
}

juce::String SiedAudioProcessor::getFactoryPresetName (int presetIndex) const
{
    return juce::isPositiveAndBelow (presetIndex, factoryPresetCount)
               ? juce::String (factoryPresets[presetIndex].name) : juce::String {};
}

juce::String SiedAudioProcessor::getFactoryPresetCategory (int presetIndex) const
{
    return juce::isPositiveAndBelow (presetIndex, factoryPresetCount)
               ? juce::String (factoryPresets[presetIndex].category) : juce::String {};
}

void SiedAudioProcessor::getStateInformation (juce::MemoryBlock& destination)
{
    auto state = createStateTree();
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destination);
}

juce::ValueTree SiedAudioProcessor::captureSceneTree() const
{
    juce::ValueTree scene ("SIED_SCENE");
    for (auto* baseParameter : AudioProcessor::getParameters())
        if (const auto* parameter = dynamic_cast<const juce::RangedAudioParameter*> (baseParameter))
            scene.setProperty (juce::Identifier ("p_" + parameter->getParameterID()),
                               parameter->getValue(), nullptr);
    scene.setProperty ("oneShotAPath", loadedSamplePaths[0], nullptr);
    scene.setProperty ("oneShotBPath", loadedSamplePaths[1], nullptr);
    scene.setProperty ("texturePath", loadedSamplePaths[2], nullptr);
    scene.setProperty ("oneShotAIndex", selectedOneShots[0], nullptr);
    scene.setProperty ("oneShotBIndex", selectedOneShots[1], nullptr);
    scene.setProperty ("textureIndex", selectedTexture, nullptr);
    scene.setProperty ("oneShotAName", currentSoundNames[0], nullptr);
    scene.setProperty ("oneShotBName", currentSoundNames[1], nullptr);
    scene.setProperty ("textureName", currentSoundNames[2], nullptr);
    return scene;
}

void SiedAudioProcessor::captureScene (int sceneIndex)
{
    if (! juce::isPositiveAndBelow (sceneIndex, 2))
        return;
    scenes[static_cast<size_t> (sceneIndex)] = captureSceneTree();
    lastSceneSampleSide = -1;
}

bool SiedAudioProcessor::hasScene (int sceneIndex) const
{
    return juce::isPositiveAndBelow (sceneIndex, 2)
           && scenes[static_cast<size_t> (sceneIndex)].isValid();
}

void SiedAudioProcessor::restoreSceneSamples (const juce::ValueTree& scene)
{
    const std::array<SiedLayer, 2> layers { SiedLayer::oneShotA, SiedLayer::oneShotB };
    const std::array<const char*, 2> pathKeys { "oneShotAPath", "oneShotBPath" };
    const std::array<const char*, 2> nameKeys { "oneShotAName", "oneShotBName" };
    const std::array<const char*, 2> indexKeys { "oneShotAIndex", "oneShotBIndex" };
    for (size_t slot = 0; slot < layers.size(); ++slot)
    {
        bool loaded = false;
        const auto path = scene.getProperty (pathKeys[slot]).toString();
        if (path.isNotEmpty() && juce::File (path).existsAsFile())
            loaded = loadSampleFileInternal (juce::File (path), layers[slot], false);
        if (! loaded)
        {
            const auto name = scene.getProperty (nameKeys[slot]).toString();
            const auto index = name.isNotEmpty() ? findOneShotByName (name, true) : -1;
            if (index >= 0)
                loaded = recallOneShot (layers[slot], index);
        }
        if (! loaded)
        {
            const auto index = static_cast<int> (scene.getProperty (indexKeys[slot], -1));
            if (juce::isPositiveAndBelow (index, getOneShotCount()))
                recallOneShot (layers[slot], index);
        }
    }

    bool loaded = false;
    const auto texturePath = scene.getProperty ("texturePath").toString();
    if (texturePath.isNotEmpty() && juce::File (texturePath).existsAsFile())
        loaded = loadSampleFileInternal (juce::File (texturePath), SiedLayer::texture, false);
    if (! loaded)
    {
        const auto name = scene.getProperty ("textureName").toString();
        const auto index = name.isNotEmpty() ? findTextureByName (name, true) : -1;
        if (index >= 0)
            loaded = recallTexture (index);
    }
    if (! loaded)
    {
        const auto index = static_cast<int> (scene.getProperty ("textureIndex", -1));
        if (juce::isPositiveAndBelow (index, getTextureCount()))
            recallTexture (index);
    }
}

void SiedAudioProcessor::applySceneMorph (float position)
{
    if (! scenes[0].isValid() || ! scenes[1].isValid())
        return;
    position = juce::jlimit (0.0f, 1.0f, position);
    for (auto* baseParameter : AudioProcessor::getParameters())
    {
        auto* parameter = dynamic_cast<juce::RangedAudioParameter*> (baseParameter);
        if (parameter == nullptr || parameter->getParameterID().startsWith ("lock"))
            continue;
        const auto property = juce::Identifier ("p_" + parameter->getParameterID());
        const auto a = static_cast<float> (scenes[0].getProperty (property, parameter->getValue()));
        const auto b = static_cast<float> (scenes[1].getProperty (property, parameter->getValue()));
        const auto discrete = dynamic_cast<juce::AudioParameterBool*> (parameter) != nullptr
                           || dynamic_cast<juce::AudioParameterChoice*> (parameter) != nullptr
                           || dynamic_cast<juce::AudioParameterInt*> (parameter) != nullptr;
        parameter->setValueNotifyingHost (discrete ? (position < 0.5f ? a : b)
                                                   : a + (b - a) * position);
    }
    const auto sampleSide = position < 0.5f ? 0 : 1;
    if (sampleSide != lastSceneSampleSide)
    {
        restoreSceneSamples (scenes[static_cast<size_t> (sampleSide)]);
        lastSceneSampleSide = sampleSide;
    }
}

juce::ValueTree SiedAudioProcessor::createStateTree()
{
    auto state = parameters.copyState();
    state.setProperty ("oneShotAPath", loadedSamplePaths[0], nullptr);
    state.setProperty ("oneShotBPath", loadedSamplePaths[1], nullptr);
    state.setProperty ("texturePath", loadedSamplePaths[2], nullptr);
    state.setProperty ("oneShotAFactory", selectedOneShots[0], nullptr);
    state.setProperty ("oneShotBFactory", selectedOneShots[1], nullptr);
    state.setProperty ("textureIndex", selectedTexture, nullptr);
    state.setProperty ("oneShotAName", currentSoundNames[0], nullptr);
    state.setProperty ("oneShotBName", currentSoundNames[1], nullptr);
    state.setProperty ("textureName", currentSoundNames[2], nullptr);
    if (const auto existing = state.getChildWithName ("SCENES"); existing.isValid())
        state.removeChild (existing, nullptr);
    juce::ValueTree sceneContainer ("SCENES");
    for (int i = 0; i < 2; ++i)
        if (scenes[static_cast<size_t> (i)].isValid())
        {
            auto copy = scenes[static_cast<size_t> (i)].createCopy();
            copy.setProperty ("slot", i, nullptr);
            sceneContainer.addChild (copy, -1, nullptr);
        }
    state.addChild (sceneContainer, -1, nullptr);
    return state;
}

void SiedAudioProcessor::restoreStateTree (juce::ValueTree state)
{
    if (! state.isValid())
        return;
    if (const auto container = state.getChildWithName ("SCENES"); container.isValid())
        for (const auto& child : container)
        {
            const auto slot = static_cast<int> (child.getProperty ("slot", -1));
            if (juce::isPositiveAndBelow (slot, 2))
                scenes[static_cast<size_t> (slot)] = child.createCopy();
        }
    parameters.replaceState (state);

    const std::array<const char*, 2> pathKeys { "oneShotAPath", "oneShotBPath" };
    const std::array<const char*, 2> nameKeys { "oneShotAName", "oneShotBName" };
    const std::array<const char*, 2> factoryKeys { "oneShotAFactory", "oneShotBFactory" };
    const std::array<SiedLayer, 2> slotLayers { SiedLayer::oneShotA, SiedLayer::oneShotB };
    for (size_t slot = 0; slot < slotLayers.size(); ++slot)
    {
        bool loaded = false;
        const auto path = state.getProperty (pathKeys[slot]).toString();
        if (path.isNotEmpty() && juce::File (path).existsAsFile())
            loaded = loadSampleFileInternal (juce::File (path), slotLayers[slot], false);

        if (! loaded)
        {
            const auto name = state.getProperty (nameKeys[slot]).toString();
            const auto namedIndex = name.isNotEmpty() ? findOneShotByName (name, true) : -1;
            if (namedIndex >= 0)
                loaded = recallOneShot (slotLayers[slot], namedIndex);
        }

        if (! loaded)
        {
            const auto savedIndex = static_cast<int> (state.getProperty (
                factoryKeys[slot], slot == 0 ? 0 : 10));
            if (juce::isPositiveAndBelow (savedIndex, getOneShotCount()))
                loaded = recallOneShot (slotLayers[slot], savedIndex);
        }

        if (! loaded)
            loadFactoryToLayer (slot == 0 ? 0 : 10, slotLayers[slot], false);
    }

    bool textureLoaded = false;
    const auto texturePath = state.getProperty ("texturePath").toString();
    if (texturePath.isNotEmpty() && juce::File (texturePath).existsAsFile())
        textureLoaded = loadSampleFileInternal (juce::File (texturePath),
                                                SiedLayer::texture, false);
    if (! textureLoaded)
    {
        const auto textureName = state.getProperty ("textureName").toString();
        const auto namedIndex = textureName.isNotEmpty() ? findTextureByName (textureName, true) : -1;
        if (namedIndex >= 0)
            textureLoaded = recallTexture (namedIndex);
    }
    if (! textureLoaded)
    {
        const auto savedIndex = static_cast<int> (state.getProperty ("textureIndex", 0));
        if (juce::isPositiveAndBelow (savedIndex, getTextureCount()))
            textureLoaded = recallTexture (savedIndex);
    }
    if (! textureLoaded)
        loadEmbeddedTexture();

    requestEffectsReset();
}

void SiedAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
    {
        auto state = juce::ValueTree::fromXml (*xml);
        restoreStateTree (state);
    }
}

juce::String SiedAudioProcessor::getUserPresetFolderPath() const
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("SIED").getChildFile ("Presets").getFullPathName();
}

juce::StringArray SiedAudioProcessor::getUserPresetNames() const
{
    juce::StringArray names;
    const juce::File folder (getUserPresetFolderPath());
    if (! folder.isDirectory())
        return names;
    for (const auto& entry : juce::RangedDirectoryIterator (
             folder, false, "*.siedpreset", juce::File::findFiles))
        names.add (entry.getFile().getFileNameWithoutExtension());
    names.sortNatural();
    return names;
}

bool SiedAudioProcessor::saveUserPreset (const juce::String& presetName)
{
    const auto legalName = juce::File::createLegalFileName (presetName.trim());
    if (legalName.isEmpty())
        return false;
    const juce::File folder (getUserPresetFolderPath());
    if (folder.createDirectory().failed())
        return false;
    auto state = createStateTree();
    state.setProperty ("presetName", legalName, nullptr);
    if (auto xml = state.createXml())
        return xml->writeTo (folder.getChildFile (legalName + ".siedpreset"));
    return false;
}

bool SiedAudioProcessor::loadUserPreset (const juce::String& presetName)
{
    const auto legalName = juce::File::createLegalFileName (presetName.trim());
    const juce::File file = juce::File (getUserPresetFolderPath())
                                .getChildFile (legalName + ".siedpreset");
    if (! file.existsAsFile())
        return false;
    if (auto xml = juce::XmlDocument::parse (file))
    {
        auto state = juce::ValueTree::fromXml (*xml);
        if (state.isValid())
        {
            restoreStateTree (state);
            return true;
        }
    }
    return false;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SiedAudioProcessor();
}
