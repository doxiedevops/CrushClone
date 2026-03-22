#pragma once
#include <JuceHeader.h>

// ═══════════════════════════════════════════════════════════════════════════════
// 4× oversampler using JUCE's built-in dsp::Oversampling
// Wraps the nonlinear processing to prevent aliasing artefacts.
// ═══════════════════════════════════════════════════════════════════════════════

class Oversampler
{
public:
    Oversampler();

    void prepare (double sampleRate, int samplesPerBlock, int numChannels);
    void reset();

    // Process a block through a lambda at 4× the sample rate.
    // `processFunc` signature: void(juce::dsp::AudioBlock<float>&)
    template <typename ProcessFunc>
    void process (juce::AudioBuffer<float>& buffer, ProcessFunc&& processFunc)
    {
        juce::dsp::AudioBlock<float> inputBlock (buffer);
        auto oversampledBlock = oversampling.processSamplesUp (inputBlock);
        processFunc (oversampledBlock);
        oversampling.processSamplesDown (inputBlock);
    }

    float getLatencyInSamples() const;

private:
    // Order 2 = 4× oversampling, IIR half-band filter for quality/CPU balance
    juce::dsp::Oversampling<float> oversampling { 2, 2,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true };
};
