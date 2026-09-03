#include "SiedVoice.h"

namespace
{
constexpr size_t layerIndex (SiedLayer layer)
{
    return static_cast<size_t> (layer);
}

double midiNoteToFrequency (double note)
{
    return 440.0 * std::pow (2.0, (note - 69.0) / 12.0);
}
}

void SiedSampleBank::setSample (SiedLayer layer, std::shared_ptr<const SiedSampleData> newSample)
{
    std::atomic_store_explicit (&samples[layerIndex (layer)], std::move (newSample),
                                std::memory_order_release);
}

std::shared_ptr<const SiedSampleData> SiedSampleBank::getSample (SiedLayer layer) const
{
    return std::atomic_load_explicit (&samples[layerIndex (layer)], std::memory_order_acquire);
}

SiedVoice::SiedVoice (SiedSampleBank& bank, juce::AudioProcessorValueTreeState& state)
    : sampleBank (bank), parameters (state)
{
    attackParam = parameters.getRawParameterValue ("attack");
    decayParam = parameters.getRawParameterValue ("decay");
    sustainParam = parameters.getRawParameterValue ("sustain");
    releaseParam = parameters.getRawParameterValue ("release");
    toneParam = parameters.getRawParameterValue ("tone");
    driftParam = parameters.getRawParameterValue ("drift");
    haloParam = parameters.getRawParameterValue ("halo");
    shimmerMixParam = parameters.getRawParameterValue ("shimmerMix");
    rootNoteParam = parameters.getRawParameterValue ("rootNote");
    layerRootParams = { parameters.getRawParameterValue ("oneShotARoot"),
                        parameters.getRawParameterValue ("oneShotBRoot") };
    sampleStartParam = parameters.getRawParameterValue ("sampleStart");
    sampleEndParam = parameters.getRawParameterValue ("sampleEnd");
    sample2StartParam = parameters.getRawParameterValue ("sample2Start");
    sample2EndParam = parameters.getRawParameterValue ("sample2End");
    textureStartParam = parameters.getRawParameterValue ("textureStart");
    textureRandomParam = parameters.getRawParameterValue ("textureRandom");
    oneShotALevelParam = parameters.getRawParameterValue ("oneShotALevel");
    oneShotBLevelParam = parameters.getRawParameterValue ("oneShotBLevel");
    textureLevelParam = parameters.getRawParameterValue ("textureLevel");
    layerEnabledParams = { parameters.getRawParameterValue ("oneShotAEnabled"),
                           parameters.getRawParameterValue ("oneShotBEnabled"),
                           parameters.getRawParameterValue ("textureEnabled") };
    layerTransposeParams = { parameters.getRawParameterValue ("oneShotATranspose"),
                             parameters.getRawParameterValue ("oneShotBTranspose"),
                             parameters.getRawParameterValue ("textureTranspose") };
    layerFineParams = { parameters.getRawParameterValue ("oneShotAFine"),
                        parameters.getRawParameterValue ("oneShotBFine"),
                        parameters.getRawParameterValue ("textureFine") };
    transposeParam = parameters.getRawParameterValue ("transpose");
    fineTuneParam = parameters.getRawParameterValue ("fineTune");
    velocityParam = parameters.getRawParameterValue ("velocity");
    reverseParam = parameters.getRawParameterValue ("reverse");
    loopModeParam = parameters.getRawParameterValue ("loopMode");
    glideParam = parameters.getRawParameterValue ("glide");
    monoParam = parameters.getRawParameterValue ("mono");
}

void SiedVoice::prepare (double newSampleRate, int, int)
{
    outputSampleRate = newSampleRate;
    adsr.setSampleRate (newSampleRate);
    adsr.reset();
    filterState[0] = filterState[1] = 0.0f;
    frequencySmoother.reset (newSampleRate, 0.001);
    frequencySmoother.setCurrentAndTargetValue (440.0);
    for (size_t i = 0; i < layerEnableSmoothers.size(); ++i)
    {
        layerEnableSmoothers[i].reset (newSampleRate, 0.012);
        const auto enabledGain = layerEnabledParams[i]->load() >= 0.5f ? 1.0f : 0.0f;
        layerEnableSmoothers[i].setCurrentAndTargetValue (enabledGain);
    }
}

void SiedVoice::setVoiceEnabled (bool shouldBeEnabled)
{
    const auto wasEnabled = enabled.exchange (shouldBeEnabled, std::memory_order_relaxed);
    if (wasEnabled && ! shouldBeEnabled && isVoiceActive())
        stopNote (0.0f, true);
}

bool SiedVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return enabled.load (std::memory_order_relaxed)
           && dynamic_cast<SiedSound*> (sound) != nullptr;
}

float SiedVoice::consumePlaybackPosition (SiedLayer layer)
{
    auto& position = blockPlaybackPositions[layerIndex (layer)];
    const auto result = position;
    position = -1.0f;
    return result;
}

void SiedVoice::configureLayer (SiedLayer layer, LayerPlayback& playback)
{
    playback = {};
    playback.sample = sampleBank.getSample (layer);
    if (playback.sample == nullptr || playback.sample->audio.getNumSamples() < 2)
        return;

    const auto lastSample = static_cast<double> (playback.sample->audio.getNumSamples() - 1);
    playback.texture = layer == SiedLayer::texture;
    playback.finished = false;

    if (playback.texture)
    {
        playback.regionStart = 0.0;
        playback.regionEnd = lastSample;
        playback.loopStart = 0.0;
        playback.loopEnd = lastSample;
        playback.loopEnabled = lastSample > 64.0;
        playback.loopCrossfade = juce::jlimit (64, 16384,
                                               static_cast<int> (lastSample / 6.0));
        playback.loopCrossfade = juce::jmin (playback.loopCrossfade,
                                             static_cast<int> (lastSample / 3.0));

        const auto baseStart = juce::jlimit (0.0, 0.999,
                                             static_cast<double> (textureStartParam->load()));
        const auto randomRange = juce::jlimit (0.0, 1.0,
                                               static_cast<double> (textureRandomParam->load()));
        const auto randomOffset = juce::Random::getSystemRandom().nextDouble() * randomRange;
        const auto normalisedStart = std::fmod (baseStart + randomOffset, 1.0);
        playback.position = normalisedStart * playback.loopEnd;
        playback.octavePosition = playback.position;
        return;
    }

    const auto* start = layer == SiedLayer::oneShotA ? sampleStartParam : sample2StartParam;
    const auto* end = layer == SiedLayer::oneShotA ? sampleEndParam : sample2EndParam;
    const auto startNormalised = juce::jlimit (0.0, 0.995, static_cast<double> (start->load()));
    const auto endNormalised = juce::jlimit (startNormalised + 0.005, 1.0,
                                             static_cast<double> (end->load()));
    playback.regionStart = std::floor (startNormalised * lastSample);
    playback.regionEnd = juce::jmax (playback.regionStart + 2.0,
                                     std::ceil (endNormalised * lastSample));
    playback.reverse = reverseParam->load() >= 0.5f;

    const auto loopMode = static_cast<int> (std::round (loopModeParam->load()));
    const auto factoryLoopAvailable = playback.sample->loopEnabled
                                      && playback.regionStart <= playback.sample->loopStart
                                      && playback.regionEnd >= playback.sample->loopEnd;
    playback.loopEnabled = loopMode == 2 || (loopMode == 0 && factoryLoopAvailable);
    if (loopMode == 2)
    {
        playback.loopStart = playback.regionStart;
        playback.loopEnd = playback.regionEnd;
        playback.loopCrossfade = juce::jlimit (64, 4096,
                                               static_cast<int> ((playback.regionEnd
                                                                  - playback.regionStart) / 6.0));
    }
    else
    {
        playback.loopStart = playback.sample->loopStart;
        playback.loopEnd = playback.sample->loopEnd;
        playback.loopCrossfade = playback.sample->loopCrossfade;
    }

    const auto loopLength = playback.loopEnd - playback.loopStart;
    playback.loopCrossfade = juce::jmin (playback.loopCrossfade,
                                         static_cast<int> (loopLength / 3.0));
    playback.loopEnabled = playback.loopEnabled && loopLength > 64.0
                           && playback.loopCrossfade > 0;
    playback.position = playback.reverse ? playback.regionEnd - 2.0 : playback.regionStart;
    playback.octavePosition = playback.position;
}

void SiedVoice::startNote (int midiNoteNumber, float velocity,
                           juce::SynthesiserSound*, int pitchWheelPosition)
{
    displayPriority = nextDisplayPriority.fetch_add (1, std::memory_order_relaxed) + 1;
    currentMidiNote = midiNoteNumber;
    noteVelocity = velocity;
    pitchWheel = pitchWheelPosition;

    configureLayer (SiedLayer::oneShotA, layers[layerIndex (SiedLayer::oneShotA)]);
    configureLayer (SiedLayer::oneShotB, layers[layerIndex (SiedLayer::oneShotB)]);
    configureLayer (SiedLayer::texture, layers[layerIndex (SiedLayer::texture)]);

    phase = 0.0;
    driftPhase = juce::Random::getSystemRandom().nextDouble()
                 * juce::MathConstants<double>::twoPi;
    filterState[0] = filterState[1] = 0.0f;

    envelope.attack = attackParam->load();
    envelope.decay = decayParam->load();
    envelope.sustain = sustainParam->load();
    envelope.release = releaseParam->load();
    adsr.setParameters (envelope);
    adsr.noteOn();
    updatePitchTarget (hasPreviousPitch && (monoParam->load() >= 0.5f || glideParam->load() > 0.0f));
}

void SiedVoice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        adsr.reset();
        for (auto& layer : layers)
            layer = {};
        clearCurrentNote();
    }
}

void SiedVoice::pitchWheelMoved (int newValue)
{
    pitchWheel = newValue;
    updatePitchTarget (false);
}

void SiedVoice::controllerMoved (int, int)
{
}

void SiedVoice::updatePitchTarget (bool useGlide)
{
    const auto bendSemitones = ((static_cast<double> (pitchWheel) - 8192.0) / 8192.0) * 2.0;
    const auto noteWithBend = static_cast<double> (currentMidiNote) + bendSemitones
                              + static_cast<double> (transposeParam->load())
                              + static_cast<double> (fineTuneParam->load()) / 100.0;
    const auto newFrequency = midiNoteToFrequency (noteWithBend);
    const auto glideSeconds = juce::jlimit (0.0, 2.0, static_cast<double> (glideParam->load()));

    if (useGlide && glideSeconds > 0.001 && hasPreviousPitch)
    {
        frequencySmoother.setCurrentAndTargetValue (juce::jmax (1.0, previousFrequency));
        frequencySmoother.reset (outputSampleRate, glideSeconds);
        frequencySmoother.setTargetValue (newFrequency);
    }
    else
    {
        frequencySmoother.reset (outputSampleRate, 0.006);
        frequencySmoother.setCurrentAndTargetValue (newFrequency);
    }

    targetFrequency = newFrequency;
    previousFrequency = newFrequency;
    hasPreviousPitch = true;
}

float SiedVoice::renderStarterOscillator (double phaseValue) const
{
    const auto sine = std::sin (phaseValue);
    const auto softTriangle = (2.0 / juce::MathConstants<double>::pi) * std::asin (sine);
    const auto octave = std::sin (phaseValue * 2.0) * 0.12;
    return static_cast<float> (sine * 0.68 + softTriangle * 0.25 + octave);
}

float SiedVoice::renderLayerSample (const LayerPlayback& playback, int channel,
                                    double position) const
{
    if (playback.sample == nullptr || playback.finished
        || playback.sample->audio.getNumSamples() < 2)
        return 0.0f;

    const auto length = playback.sample->audio.getNumSamples();
    if (position < playback.regionStart
        || position >= juce::jmin (playback.regionEnd, static_cast<double> (length - 1)))
        return 0.0f;

    const auto sourceChannel = juce::jmin (channel, playback.sample->audio.getNumChannels() - 1);
    const auto* data = playback.sample->audio.getReadPointer (sourceChannel);
    const auto interpolate = [data, length] (double samplePosition)
    {
        samplePosition = juce::jlimit (0.0, static_cast<double> (length - 2), samplePosition);
        const auto index = static_cast<int> (samplePosition);
        const auto fraction = static_cast<float> (samplePosition - static_cast<double> (index));
        return juce::jmap (fraction, data[index], data[index + 1]);
    };

    const auto primary = interpolate (position);
    if (playback.loopEnabled && playback.loopCrossfade > 0)
    {
        if (! playback.reverse && position >= playback.loopEnd - playback.loopCrossfade
            && position < playback.loopEnd)
        {
            const auto offset = position - (playback.loopEnd - playback.loopCrossfade);
            const auto secondary = interpolate (playback.loopStart + offset);
            const auto blend = juce::jlimit (0.0, 1.0,
                                             offset / static_cast<double> (playback.loopCrossfade));
            const auto dryGain = std::cos (blend * juce::MathConstants<double>::halfPi);
            const auto wetGain = std::sin (blend * juce::MathConstants<double>::halfPi);
            return static_cast<float> (primary * dryGain + secondary * wetGain);
        }

        if (playback.reverse && position >= playback.loopStart
            && position < playback.loopStart + playback.loopCrossfade)
        {
            const auto offset = position - playback.loopStart;
            const auto secondary = interpolate (playback.loopEnd - playback.loopCrossfade + offset);
            const auto blend = juce::jlimit (0.0, 1.0,
                                             offset / static_cast<double> (playback.loopCrossfade));
            const auto primaryGain = std::sin (blend * juce::MathConstants<double>::halfPi);
            const auto secondaryGain = std::cos (blend * juce::MathConstants<double>::halfPi);
            return static_cast<float> (primary * primaryGain + secondary * secondaryGain);
        }
    }

    if (! playback.loopEnabled)
    {
        constexpr double edgeFadeSamples = 128.0;
        const auto fadeIn = juce::jlimit (0.0, 1.0,
                                          (position - playback.regionStart) / edgeFadeSamples);
        const auto fadeOut = juce::jlimit (0.0, 1.0,
                                           (playback.regionEnd - position) / edgeFadeSamples);
        return primary * static_cast<float> (juce::jmin (fadeIn, fadeOut));
    }

    return primary;
}

void SiedVoice::advanceLayerPosition (LayerPlayback& playback, double& position,
                                      double increment, bool markFinished) const
{
    if (playback.sample == nullptr || playback.finished)
        return;

    position += playback.reverse ? -increment : increment;
    if (playback.loopEnabled)
    {
        const auto loopLength = playback.loopEnd - playback.loopStart;
        if (! playback.reverse && position >= playback.loopEnd)
            position = playback.loopStart + std::fmod (position - playback.loopEnd, loopLength);
        else if (playback.reverse && position <= playback.loopStart)
            position = playback.loopEnd - 0.000001
                       - std::fmod (playback.loopStart - position, loopLength);
        return;
    }

    if (markFinished
        && (playback.reverse ? position <= playback.regionStart : position >= playback.regionEnd))
        playback.finished = true;
}

bool SiedVoice::allSampleLayersFinished() const
{
    bool foundSample = false;
    for (const auto& layer : layers)
    {
        foundSample = foundSample || layer.sample != nullptr;
        if (layer.sample != nullptr && ! layer.finished)
            return false;
    }
    return foundSample;
}

void SiedVoice::renderNextBlock (juce::AudioBuffer<float>& output, int startSample, int numSamples)
{
    if (! isVoiceActive())
        return;

    blockPlaybackPositions.fill (-1.0f);

    envelope.attack = attackParam->load();
    envelope.decay = decayParam->load();
    envelope.sustain = sustainParam->load();
    envelope.release = releaseParam->load();
    adsr.setParameters (envelope);

    const auto drift = std::pow (juce::jlimit (0.0f, 1.0f, driftParam->load()), 1.65f);
    const auto shimmerAmount = std::pow (
        juce::jlimit (0.0f, 1.0f, haloParam->load()), 1.80f);
    const auto shimmerBlend = std::pow (
        juce::jlimit (0.0f, 1.0f, shimmerMixParam->load()), 1.40f);
    const auto octaveMix = shimmerAmount * shimmerBlend * 0.36f;
    const auto tone = toneParam->load();
    const auto cutoff = 350.0f * std::pow (56.0f, tone);
    const auto filterCoefficient = 1.0f - std::exp (-juce::MathConstants<float>::twoPi
                                                     * cutoff / static_cast<float> (outputSampleRate));
    const std::array<float, 3> levelGains
    {
        juce::Decibels::decibelsToGain (oneShotALevelParam->load()),
        juce::Decibels::decibelsToGain (oneShotBLevelParam->load()),
        juce::Decibels::decibelsToGain (textureLevelParam->load())
    };
    for (size_t i = 0; i < layerEnableSmoothers.size(); ++i)
        layerEnableSmoothers[i].setTargetValue (layerEnabledParams[i]->load() >= 0.5f
                                                    ? 1.0f : 0.0f);
    const auto hasAnySample = std::any_of (layers.begin(), layers.end(), [] (const auto& layer)
    {
        return layer.sample != nullptr;
    });

    for (int i = 0; i < numSamples; ++i)
    {
        std::array<float, 3> enabledGains {};
        for (size_t layerNumber = 0; layerNumber < enabledGains.size(); ++layerNumber)
            enabledGains[layerNumber] = layerEnableSmoothers[layerNumber].getNextValue();

        const auto driftCents = std::sin (driftPhase) * 5.0 * drift * drift;
        const auto driftRatio = std::pow (2.0, driftCents / 1200.0);
        driftPhase += juce::MathConstants<double>::twoPi * (0.12 + 0.25 * drift) / outputSampleRate;
        if (driftPhase >= juce::MathConstants<double>::twoPi)
            driftPhase -= juce::MathConstants<double>::twoPi;

        const auto frequency = frequencySmoother.getNextValue();
        const auto phaseIncrement = juce::MathConstants<double>::twoPi * frequency / outputSampleRate;
        const auto envelopeValue = adsr.getNextSample();
        const auto starterValue = hasAnySample ? 0.0f : renderStarterOscillator (phase);
        const auto starterOctave = hasAnySample ? 0.0f : renderStarterOscillator (phase * 2.0);
        const auto velocitySensitivity = velocityParam->load();
        const auto velocityGain = 1.0f - velocitySensitivity + velocitySensitivity * noteVelocity;

        for (int channel = 0; channel < output.getNumChannels(); ++channel)
        {
            float mixed = starterValue + starterOctave * octaveMix;
            for (size_t layerNumber = 0; layerNumber < layers.size(); ++layerNumber)
            {
                const auto& layer = layers[layerNumber];
                if (layer.sample == nullptr || layer.finished)
                    continue;
                if (enabledGains[layerNumber] > 0.001f)
                    blockPlaybackPositions[layerNumber] = static_cast<float> (juce::jlimit (
                        0.0, 1.0, layer.position
                                  / static_cast<double> (layer.sample->audio.getNumSamples() - 1)));
                const auto base = renderLayerSample (layer, channel, layer.position);
                const auto octave = layer.texture ? 0.0f
                                                  : renderLayerSample (layer, channel,
                                                                       layer.octavePosition);
                mixed += (base + octave * octaveMix) * layer.sample->gainLinear
                         * levelGains[layerNumber] * enabledGains[layerNumber];
            }

            filterState[channel] += filterCoefficient * (mixed - filterState[channel]);
            output.addSample (channel, startSample + i,
                              filterState[channel] * envelopeValue * velocityGain * 0.38f);
        }

        for (size_t layerNumber = 0; layerNumber < layers.size(); ++layerNumber)
        {
            auto& layer = layers[layerNumber];
            if (layer.sample == nullptr || layer.finished)
                continue;
            auto layerRoot = rootNoteParam->load();
            if (layerNumber < layerRootParams.size()
                && layerRootParams[layerNumber]->load() > 0.5f)
                layerRoot = layerRootParams[layerNumber]->load();
            const auto pitchRatio = frequency / midiNoteToFrequency (layerRoot);
            const auto sourceRatio = layer.sample->sourceSampleRate / outputSampleRate;
            const auto layerSemitones = static_cast<double> (layerTransposeParams[layerNumber]->load())
                                        + static_cast<double> (layerFineParams[layerNumber]->load())
                                              / 100.0;
            const auto layerTuneRatio = std::pow (2.0, layerSemitones / 12.0);
            const auto increment = (layer.texture ? sourceRatio * layerTuneRatio
                                                  : pitchRatio * sourceRatio * layerTuneRatio)
                                   * driftRatio;
            advanceLayerPosition (layer, layer.position, increment, true);
            if (! layer.texture)
                advanceLayerPosition (layer, layer.octavePosition, increment * 2.0, false);
        }

        phase += phaseIncrement * driftRatio;
        if (phase >= juce::MathConstants<double>::twoPi)
            phase -= juce::MathConstants<double>::twoPi;

        if (allSampleLayersFinished())
        {
            adsr.reset();
            clearCurrentNote();
            break;
        }

        if (! adsr.isActive())
        {
            for (auto& layer : layers)
                layer = {};
            clearCurrentNote();
            break;
        }
    }
}
