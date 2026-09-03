#include "MacroEffects.h"

namespace
{
float shapedAmount (float value, float exponent = 1.65f)
{
    return std::pow (juce::jlimit (0.0f, 1.0f, value), exponent);
}

float shapedBipolar (float value, float exponent = 1.55f)
{
    const auto magnitude = std::pow (juce::jlimit (0.0f, 1.0f, std::abs (value)), exponent);
    return std::copysign (magnitude, value);
}

float balancedWet (float curvedAmount, float rawMix)
{
    const auto mix = shapedAmount (rawMix, 1.40f);
    return mix * (0.025f + curvedAmount * 0.975f);
}
}

void MacroEffects::prepare (const juce::dsp::ProcessSpec& spec)
{
    currentSampleRate = spec.sampleRate;
    chorusProcessor.prepare (spec);
    chorusProcessor.setCentreDelay (9.0f);
    phaserProcessor.prepare (spec);
    compressorProcessor.prepare (spec);
    shimmerReverb.setSampleRate (spec.sampleRate);
    roomReverb.setSampleRate (spec.sampleRate);
    const auto blockSize = juce::jmax (8192, static_cast<int> (spec.maximumBlockSize));
    shimmerBuffer.setSize (2, blockSize, false, true, false);
    roomBuffer.setSize (2, blockSize, false, true, false);
    dryBuffer.setSize (2, blockSize, false, true, false);
    delayBuffer.setSize (2, static_cast<int> (spec.sampleRate * 8.5), false, true, false);
    flangerBuffer.setSize (2, static_cast<int> (spec.sampleRate * 0.06), false, true, false);
    delaySamplesSmoother.reset (spec.sampleRate, 0.035);
    delaySamplesSmoother.setCurrentAndTargetValue (static_cast<float> (spec.sampleRate * 0.35));
    reset();
}

void MacroEffects::reset()
{
    chorusProcessor.reset();
    phaserProcessor.reset();
    compressorProcessor.reset();
    shimmerReverb.reset();
    roomReverb.reset();
    shimmerBuffer.clear(); roomBuffer.clear(); dryBuffer.clear();
    delayBuffer.clear(); flangerBuffer.clear();
    delayWritePosition = flangerWritePosition = 0;
    crushCounter[0] = crushCounter[1] = 0;
    crushHeldSample[0] = crushHeldSample[1] = 0.0f;
    crushFilterState[0] = crushFilterState[1] = 0.0f;
    delayFilterState[0] = delayFilterState[1] = 0.0f;
    lowpassState[0] = lowpassState[1] = 0.0f;
    lowpassState2[0] = lowpassState2[1] = 0.0f;
    highpassState[0] = highpassState[1] = 0.0f;
    highpassState2[0] = highpassState2[1] = 0.0f;
    shimmerHighpassState[0] = shimmerHighpassState[1] = 0.0f;
    shimmerHighpassState2[0] = shimmerHighpassState2[1] = 0.0f;
    delaySamplesSmoother.setCurrentAndTargetValue (
        static_cast<float> (currentSampleRate * 0.35));
    flangerPhase = tremoloPhase = 0.0;
}

void MacroEffects::copyDry (const juce::AudioBuffer<float>& buffer)
{
    jassert (buffer.getNumSamples() <= dryBuffer.getNumSamples());
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        dryBuffer.copyFrom (channel, 0, buffer, channel, 0, buffer.getNumSamples());
}

void MacroEffects::blendDryWet (juce::AudioBuffer<float>& buffer, float wet)
{
    wet = juce::jlimit (0.0f, 1.0f, wet);
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        buffer.applyGain (channel, 0, buffer.getNumSamples(), wet);
        buffer.addFrom (channel, 0, dryBuffer, channel, 0, buffer.getNumSamples(), 1.0f - wet);
    }
}

void MacroEffects::process (juce::AudioBuffer<float>& buffer, const SiedEffectSettings& s)
{
    auto p = s;
    p.shimmer = shapedAmount (s.shimmer, 1.80f);
    p.chorus = shapedAmount (s.chorus);
    p.delay = shapedAmount (s.delay, 1.70f);
    p.crush = shapedAmount (s.crush, 1.85f);
    p.reverb = shapedAmount (s.reverb, 1.75f);
    p.drive = shapedAmount (s.drive, 1.80f);
    p.phaser = shapedAmount (s.phaser);
    p.flanger = shapedAmount (s.flanger, 1.70f);
    p.tremolo = shapedAmount (s.tremolo, 1.60f);
    p.lowpass = shapedAmount (s.lowpass, 1.55f);
    p.highpass = shapedAmount (s.highpass, 1.65f);
    p.compressor = shapedAmount (s.compressor, 1.60f);
    p.width = 1.0f + shapedBipolar (s.width - 1.0f);
    p.pan = shapedBipolar (s.pan, 1.45f);

    p.shimmerMix = balancedWet (p.shimmer, s.shimmerMix);
    p.chorusMix = balancedWet (p.chorus, s.chorusMix);
    p.delayMix = balancedWet (p.delay, s.delayMix);
    p.crushMix = balancedWet (p.crush, s.crushMix);
    p.reverbMix = balancedWet (p.reverb, s.reverbMix);
    p.driveMix = balancedWet (p.drive, s.driveMix);
    p.phaserMix = balancedWet (p.phaser, s.phaserMix);
    p.flangerMix = balancedWet (p.flanger, s.flangerMix);
    p.tremoloMix = balancedWet (p.tremolo, s.tremoloMix);
    p.lowpassMix = balancedWet (p.lowpass, s.lowpassMix);
    p.highpassMix = balancedWet (p.highpass, s.highpassMix);
    p.compressorMix = balancedWet (p.compressor, s.compressorMix);
    p.widthMix = balancedWet (std::abs (p.width - 1.0f), s.widthMix);

    processDrive (buffer, p.drive, p.driveMix, p.driveType);
    processCrush (buffer, p.crush, p.crushMix, p.crushType);
    processFilters (buffer, p.lowpass, p.lowpassMix, p.highpass, p.highpassMix,
                    p.lowpassType, p.highpassType);
    const std::array<float, 3> chorusRate { 0.10f, 0.24f, 0.06f };
    const std::array<float, 3> chorusDepth { 0.36f, 0.78f, 0.94f };
    const std::array<float, 3> chorusFeedback { 0.08f, 0.16f, 0.30f };
    const auto chorusType = juce::jlimit (0, 2, p.chorusType);
    if (p.chorus > 0.0001f && p.chorusMix > 0.0001f)
    {
        copyDry (buffer);
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        chorusProcessor.setRate (chorusRate[static_cast<size_t> (chorusType)]
                                 + p.chorus * (chorusType == 2 ? 0.24f : 0.54f));
        chorusProcessor.setDepth (0.03f + p.chorus
                                  * chorusDepth[static_cast<size_t> (chorusType)]);
        chorusProcessor.setFeedback (p.chorus
                                     * chorusFeedback[static_cast<size_t> (chorusType)]);
        chorusProcessor.setMix (1.0f);
        chorusProcessor.process (context);
        blendDryWet (buffer, p.chorusMix);
    }
    const auto phaserType = juce::jlimit (0, 2, p.phaserType);
    const std::array<float, 3> phaserRates { 0.08f, 0.035f, 0.72f };
    const std::array<float, 3> phaserCentres { 420.0f, 180.0f, 960.0f };
    if (p.phaser > 0.0001f && p.phaserMix > 0.0001f)
    {
        copyDry (buffer);
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        phaserProcessor.setRate (phaserRates[static_cast<size_t> (phaserType)]
                                 + p.phaser * (phaserType == 2 ? 2.6f : 0.48f));
        phaserProcessor.setDepth (0.18f + p.phaser * (phaserType == 1 ? 0.81f : 0.67f));
        phaserProcessor.setCentreFrequency (phaserCentres[static_cast<size_t> (phaserType)]
                                            + p.phaser * 1100.0f);
        phaserProcessor.setFeedback (p.phaser * (phaserType == 1 ? 0.78f : 0.56f));
        phaserProcessor.setMix (1.0f);
        phaserProcessor.process (context);
        blendDryWet (buffer, p.phaserMix);
    }
    processFlanger (buffer, p.flanger, p.flangerMix, p.flangerType);
    processTremolo (buffer, p.tremolo, p.tremoloMix, p.tremoloType);
    processDelay (buffer, p.delay, p.delayMix, p.delayType, p.delayDivision,
                  p.delayTimeMs, p.tempoBpm);
    processShimmer (buffer, p.shimmer, p.shimmerMix);
    processReverb (buffer, p.reverb, p.reverbMix, p.reverbType);
    const auto compressorType = juce::jlimit (0, 2, p.compressorType);
    const std::array<float, 3> thresholds { -18.0f, -12.0f, -24.0f };
    const std::array<float, 3> ratios { 4.0f, 8.0f, 12.0f };
    const std::array<float, 3> attacks { 18.0f, 2.0f, 32.0f };
    const std::array<float, 3> releases { 180.0f, 85.0f, 320.0f };
    if (p.compressor > 0.0001f && p.compressorMix > 0.0001f)
    {
        copyDry (buffer);
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        compressorProcessor.setThreshold (-2.0f + p.compressor
                                          * thresholds[static_cast<size_t> (compressorType)]);
        compressorProcessor.setRatio (1.0f + p.compressor
                                      * (ratios[static_cast<size_t> (compressorType)] - 1.0f));
        compressorProcessor.setAttack (attacks[static_cast<size_t> (compressorType)]);
        compressorProcessor.setRelease (releases[static_cast<size_t> (compressorType)]);
        compressorProcessor.process (context);
        blendDryWet (buffer, p.compressorMix);
    }
    processStereo (buffer, p.width, p.widthMix, p.pan);
    buffer.applyGain (juce::Decibels::decibelsToGain (p.outputGainDb));
}

void MacroEffects::processDrive (juce::AudioBuffer<float>& buffer, float amount,
                                 float mix, int type)
{
    if (amount <= 0.0001f) return;
    type = juce::jlimit (0, 3, type);
    const auto drive = 1.0f + amount * 12.0f;
    const auto makeup = 1.0f / std::tanh (drive);
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getWritePointer (channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const auto input = data[i];
            float shaped = input;
            if (type == 0)
                shaped = std::tanh (input * drive) * makeup;
            else if (type == 1)
                shaped = (std::tanh (input * drive * 0.82f)
                          + 0.10f * std::tanh (input * input * drive)) * 0.92f;
            else if (type == 2)
                shaped = std::sin (input * (1.0f + amount * 8.0f));
            else
                shaped = juce::jlimit (-0.74f, 0.74f, input * drive * 0.52f) / 0.74f;
            data[i] = juce::jmap (juce::jlimit (0.0f, 1.0f, mix), input, shaped);
        }
    }
}

void MacroEffects::processCrush (juce::AudioBuffer<float>& buffer, float amount,
                                 float mix, int type)
{
    if (amount <= 0.0001f) return;
    type = juce::jlimit (0, 2, type);
    const auto holdScale = type == 2 ? 72.0f : type == 1 ? 18.0f : 30.0f;
    const auto holdSamples = 1 + static_cast<int> (amount * amount * holdScale);
    const auto bitRange = type == 1 ? 9 : 12;
    const auto bits = juce::jlimit (4, 16, 16 - static_cast<int> (amount * bitRange));
    const auto levels = static_cast<float> (1 << (bits - 1));
    const auto cutoff = (type == 1 ? 12500.0f : 19000.0f)
                        * std::pow (type == 2 ? 0.08f : 0.16f, amount);
    const auto alpha = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * cutoff
                                        / static_cast<float> (currentSampleRate));
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getWritePointer (channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            if (crushCounter[channel]-- <= 0)
            {
                crushHeldSample[channel] = std::round (data[i] * levels) / levels;
                crushCounter[channel] = holdSamples;
            }
            crushFilterState[channel] += alpha * (crushHeldSample[channel] - crushFilterState[channel]);
            data[i] = juce::jmap (juce::jlimit (0.0f, 1.0f, mix),
                                  data[i], crushFilterState[channel]);
        }
    }
}

void MacroEffects::processFilters (juce::AudioBuffer<float>& buffer, float lowAmount,
                                   float lowMix, float highAmount, float highMix,
                                   int lowType, int highType)
{
    const auto lowCutoff = 20000.0f * std::pow (lowType == 1 ? 0.032f : 0.045f, lowAmount);
    const auto highCutoff = 20.0f * std::pow (highType == 1 ? 92.0f : 75.0f, highAmount);
    const auto lowAlpha = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * lowCutoff
                                           / static_cast<float> (currentSampleRate));
    const auto highAlpha = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * highCutoff
                                            / static_cast<float> (currentSampleRate));
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getWritePointer (channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const auto filterInput = lowType == 1
                                         ? std::tanh (data[i] * (1.0f + lowAmount * 1.6f))
                                         : data[i];
            lowpassState[channel] += lowAlpha * (filterInput - lowpassState[channel]);
            lowpassState2[channel] += lowAlpha * (lowpassState[channel] - lowpassState2[channel]);
            const auto resonantLow = lowpassState[channel]
                                     + (lowpassState[channel] - lowpassState2[channel])
                                           * lowAmount * 0.72f;
            const auto filteredLow = lowType == 2 ? resonantLow : lowpassState[channel];
            const auto lowWet = lowAmount > 0.0001f
                                    ? juce::jlimit (0.0f, 1.0f, lowMix) : 0.0f;
            const auto lowpassed = juce::jmap (lowWet, data[i], filteredLow);
            highpassState[channel] += highAlpha * (lowpassed - highpassState[channel]);
            highpassState2[channel] += highAlpha * (highpassState[channel]
                                                     - highpassState2[channel]);
            auto highpassed = lowpassed - highpassState[channel];
            if (highType == 1)
                highpassed = std::tanh (highpassed * (1.0f + highAmount * 1.4f));
            else if (highType == 2)
                highpassed += (highpassState[channel] - highpassState2[channel])
                              * highAmount * 0.62f;
            const auto highWet = highAmount > 0.0001f
                                     ? juce::jlimit (0.0f, 1.0f, highMix) : 0.0f;
            data[i] = juce::jmap (highWet, lowpassed, highpassed);
        }
    }
}

void MacroEffects::processDelay (juce::AudioBuffer<float>& buffer, float amount, float mix, int type,
                                 int division, float delayTimeMs, float tempoBpm)
{
    type = juce::jlimit (0, 3, type);
    division = juce::jlimit (0, 8, division);
    const std::array<double, 8> divisionBeats { 4.0, 2.0, 1.0, 0.5, 0.25,
                                                1.5, 0.75, 1.0 / 3.0 };
    const auto seconds = division == 0
                             ? juce::jlimit (0.02, 1.5, static_cast<double> (delayTimeMs) / 1000.0)
                             : divisionBeats[static_cast<size_t> (division - 1)]
                                   * 60.0 / juce::jlimit (30.0, 300.0,
                                                          static_cast<double> (tempoBpm));
    const auto targetSamples = juce::jlimit (1.0f,
                                              static_cast<float> (delayBuffer.getNumSamples() - 2),
                                              static_cast<float> (seconds * currentSampleRate));
    delaySamplesSmoother.setTargetValue (targetSamples);
    const auto feedback = 0.08f + amount * (type == 3 ? 0.56f : 0.66f);
    const auto wet = amount > 0.0001f ? juce::jlimit (0.0f, 1.0f, mix) : 0.0f;
    const auto dry = 1.0f - wet;
    const auto damping = type == 2 ? 0.11f : 0.18f + (1.0f - amount) * 0.31f;
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        auto readPosition = static_cast<double> (delayWritePosition)
                            - static_cast<double> (delaySamplesSmoother.getNextValue());
        while (readPosition < 0.0)
            readPosition += delayBuffer.getNumSamples();
        const auto indexA = static_cast<int> (readPosition) % delayBuffer.getNumSamples();
        const auto indexB = (indexA + 1) % delayBuffer.getNumSamples();
        const auto fraction = static_cast<float> (readPosition - std::floor (readPosition));
        const auto delayedLeft = juce::jmap (fraction,
                                             delayBuffer.getSample (0, indexA),
                                             delayBuffer.getSample (0, indexB));
        const auto delayedRight = juce::jmap (fraction,
                                              delayBuffer.getSample (1, indexA),
                                              delayBuffer.getSample (1, indexB));
        delayFilterState[0] += damping * (delayedLeft - delayFilterState[0]);
        delayFilterState[1] += damping * (delayedRight - delayFilterState[1]);
        const auto inputLeft = buffer.getSample (0, i);
        const auto inputRight = buffer.getNumChannels() > 1 ? buffer.getSample (1, i) : inputLeft;
        const auto feedbackLeft = type == 1 ? delayFilterState[1]
                                            : type == 3 ? (delayFilterState[0] + delayFilterState[1]) * 0.5f
                                                        : delayFilterState[0];
        const auto feedbackRight = type == 1 ? delayFilterState[0]
                                             : type == 3 ? (delayFilterState[0] - delayFilterState[1]) * 0.5f
                                                         : delayFilterState[1];
        auto writeLeft = inputLeft + feedbackLeft * feedback;
        auto writeRight = inputRight + feedbackRight * feedback;
        if (type == 2)
        {
            writeLeft = std::tanh (writeLeft * 1.18f);
            writeRight = std::tanh (writeRight * 1.18f);
        }
        delayBuffer.setSample (0, delayWritePosition, writeLeft);
        delayBuffer.setSample (1, delayWritePosition, writeRight);
        buffer.setSample (0, i, inputLeft * dry + delayFilterState[0] * wet);
        if (buffer.getNumChannels() > 1)
            buffer.setSample (1, i, inputRight * dry + delayFilterState[1] * wet);
        if (++delayWritePosition >= delayBuffer.getNumSamples()) delayWritePosition = 0;
    }
}

void MacroEffects::processFlanger (juce::AudioBuffer<float>& buffer, float amount,
                                   float mix, int type)
{
    if (amount <= 0.0001f) return;
    type = juce::jlimit (0, 2, type);
    const std::array<double, 3> baseDelaySeconds { 0.0015, 0.0048, 0.0007 };
    const std::array<double, 3> delayRanges { 0.0065, 0.0036, 0.0018 };
    const std::array<double, 3> baseRates { 0.06, 0.025, 0.42 };
    const std::array<float, 3> feedbacks { 0.48f, 0.28f, 0.68f };
    const std::array<float, 3> wetLevels { 0.55f, 0.44f, 0.62f };
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const auto modulation = (std::sin (flangerPhase) + 1.0) * 0.5;
        const auto delaySamples = 1.0
                                  + (baseDelaySeconds[static_cast<size_t> (type)]
                                     + modulation * delayRanges[static_cast<size_t> (type)])
                                        * currentSampleRate;
        auto readPosition = static_cast<double> (flangerWritePosition) - delaySamples;
        while (readPosition < 0.0) readPosition += flangerBuffer.getNumSamples();
        const auto indexA = static_cast<int> (readPosition) % flangerBuffer.getNumSamples();
        const auto indexB = (indexA + 1) % flangerBuffer.getNumSamples();
        const auto fraction = static_cast<float> (readPosition - std::floor (readPosition));
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            const auto input = buffer.getSample (channel, i);
            const auto delayed = juce::jmap (fraction, flangerBuffer.getSample (channel, indexA),
                                             flangerBuffer.getSample (channel, indexB));
            const auto feedback = feedbacks[static_cast<size_t> (type)];
            flangerBuffer.setSample (channel, flangerWritePosition,
                                     input + delayed * amount * feedback);
            const auto coloured = type == 2 ? std::tanh (delayed * 1.35f) : delayed;
            buffer.setSample (channel, i,
                              input + coloured * juce::jlimit (0.0f, 1.0f, mix)
                                      * wetLevels[static_cast<size_t> (type)]);
        }
        if (++flangerWritePosition >= flangerBuffer.getNumSamples()) flangerWritePosition = 0;
        flangerPhase += juce::MathConstants<double>::twoPi
                        * (baseRates[static_cast<size_t> (type)]
                           + amount * (type == 2 ? 1.25 : 0.34)) / currentSampleRate;
        if (flangerPhase >= juce::MathConstants<double>::twoPi) flangerPhase -= juce::MathConstants<double>::twoPi;
    }
}

void MacroEffects::processTremolo (juce::AudioBuffer<float>& buffer, float amount,
                                   float mix, int type)
{
    if (amount <= 0.0001f) return;
    type = juce::jlimit (0, 2, type);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const auto sine = static_cast<float> ((std::sin (tremoloPhase) + 1.0) * 0.5);
        const auto modulation = type == 0 ? sine
                                : type == 1 ? (sine >= 0.5f ? 1.0f : 0.0f)
                                            : (sine >= 0.72f ? 1.0f : 0.0f);
        const auto depth = amount * juce::jlimit (0.0f, 1.0f, mix)
                           * (type == 2 ? 0.96f : type == 1 ? 0.90f : 0.82f);
        const auto gain = 1.0f - depth + modulation * depth;
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            buffer.setSample (channel, i, buffer.getSample (channel, i) * gain);
        const auto rate = type == 0 ? 1.2 + amount * 7.8
                          : type == 1 ? 2.0 + amount * 10.0
                                      : 3.0 + amount * 15.0;
        tremoloPhase += juce::MathConstants<double>::twoPi * rate / currentSampleRate;
        if (tremoloPhase >= juce::MathConstants<double>::twoPi) tremoloPhase -= juce::MathConstants<double>::twoPi;
    }
}

void MacroEffects::processShimmer (juce::AudioBuffer<float>& buffer, float amount, float mix)
{
    if (amount <= 0.0001f) return;
    jassert (buffer.getNumSamples() <= shimmerBuffer.getNumSamples());
    shimmerBuffer.clear();
    const auto highpassCutoff = 520.0f + amount * 260.0f;
    const auto highpassAlpha = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * highpassCutoff
                                                / static_cast<float> (currentSampleRate));
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const auto* source = buffer.getReadPointer (channel);
        auto* destination = shimmerBuffer.getWritePointer (channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            shimmerHighpassState[channel] += highpassAlpha
                                             * (source[i] - shimmerHighpassState[channel]);
            const auto firstPass = source[i] - shimmerHighpassState[channel];
            shimmerHighpassState2[channel] += highpassAlpha
                                              * (firstPass - shimmerHighpassState2[channel]);
            destination[i] = firstPass - shimmerHighpassState2[channel];
        }
    }
    juce::Reverb::Parameters p;
    p.roomSize = 0.79f + amount * 0.19f; p.damping = 0.40f - amount * 0.16f;
    p.wetLevel = 1.0f; p.dryLevel = 0.0f; p.width = 1.0f;
    shimmerReverb.setParameters (p);
    shimmerReverb.processStereo (shimmerBuffer.getWritePointer (0), shimmerBuffer.getWritePointer (1),
                                  buffer.getNumSamples());
    const auto wet = juce::jlimit (0.0f, 1.0f, mix);
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        buffer.applyGain (channel, 0, buffer.getNumSamples(), 1.0f - wet);
        buffer.addFrom (channel, 0, shimmerBuffer, channel, 0,
                        buffer.getNumSamples(), wet);
    }
}

void MacroEffects::processReverb (juce::AudioBuffer<float>& buffer, float amount,
                                  float mix, int type)
{
    if (amount <= 0.0001f) return;
    jassert (buffer.getNumSamples() <= roomBuffer.getNumSamples());
    roomBuffer.clear();
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        roomBuffer.copyFrom (channel, 0, buffer, channel, 0, buffer.getNumSamples());
    juce::Reverb::Parameters p;
    type = juce::jlimit (0, 3, type);
    const std::array<float, 4> baseSize { 0.24f, 0.55f, 0.38f, 0.72f };
    const std::array<float, 4> sizeRange { 0.48f, 0.42f, 0.50f, 0.27f };
    const std::array<float, 4> damping { 0.66f, 0.48f, 0.36f, 0.24f };
    p.roomSize = baseSize[static_cast<size_t> (type)]
                 + amount * sizeRange[static_cast<size_t> (type)];
    p.damping = juce::jmax (0.08f, damping[static_cast<size_t> (type)] - amount * 0.20f);
    p.wetLevel = 1.0f; p.dryLevel = 0.0f;
    p.width = type == 2 ? 0.76f : 0.88f + amount * 0.12f;
    roomReverb.setParameters (p);
    roomReverb.processStereo (roomBuffer.getWritePointer (0), roomBuffer.getWritePointer (1),
                               buffer.getNumSamples());
    const auto wet = juce::jlimit (0.0f, 1.0f, mix);
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        buffer.applyGain (channel, 0, buffer.getNumSamples(), 1.0f - wet);
        buffer.addFrom (channel, 0, roomBuffer, channel, 0,
                        buffer.getNumSamples(), wet);
    }
}

void MacroEffects::processStereo (juce::AudioBuffer<float>& buffer, float width,
                                  float mix, float pan)
{
    if (buffer.getNumChannels() < 2) return;
    auto* left = buffer.getWritePointer (0);
    auto* right = buffer.getWritePointer (1);
    const auto leftGain = pan > 0.0f ? 1.0f - pan : 1.0f;
    const auto rightGain = pan < 0.0f ? 1.0f + pan : 1.0f;
    mix = juce::jlimit (0.0f, 1.0f, mix);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const auto mid = (left[i] + right[i]) * 0.5f;
        const auto side = (left[i] - right[i]) * 0.5f * width;
        const auto widenedLeft = mid + side;
        const auto widenedRight = mid - side;
        left[i] = juce::jmap (mix, left[i], widenedLeft) * leftGain;
        right[i] = juce::jmap (mix, right[i], widenedRight) * rightGain;
    }
}
