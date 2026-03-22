#include "LadderFilter.h"
#include <algorithm>

LadderFilter::LadderFilter() {}

void LadderFilter::prepare (double sampleRate, int samplesPerBlock, int numChannels)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels      = 1;  // we process per-channel

    for (int ch = 0; ch < 2; ++ch)
    {
        svf[ch].prepare (spec);
        svf[ch].setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        svf[ch].setCutoffFrequency (20000.0f);
        svf[ch].setResonance (0.7071f);  // default = no resonance boost
        svf[ch].reset();
    }
}

void LadderFilter::reset()
{
    for (int ch = 0; ch < 2; ++ch)
        svf[ch].reset();
}

void LadderFilter::process (juce::AudioBuffer<float>& buffer)
{
    if (! enabled)
        return;

    // If cutoff is wide open (>= 18 kHz), bypass entirely — the filter
    // is inaudible at that point and skipping it avoids any numerical edge cases
    if (cutoffHz >= 18000.0f && resonance < 0.05f)
        return;

    // Clamp cutoff to safe range: 20 Hz to 80% of Nyquist
    const float maxSafe = static_cast<float> (currentSampleRate) * 0.4f;
    const float safeCutoff = std::max (20.0f, std::min (cutoffHz, maxSafe));

    // Map resonance 0..1 to JUCE's resonance range (0.7071 = flat, ~5.0 = screaming)
    // Cap well below self-oscillation
    const float safeReso = 0.7071f + resonance * 3.5f;

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = std::min (buffer.getNumChannels(), 2);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        svf[ch].setCutoffFrequency (safeCutoff);
        svf[ch].setResonance (safeReso);

        float* data = buffer.getWritePointer (ch);
        for (int i = 0; i < numSamples; ++i)
            data[i] = svf[ch].processSample (0, data[i]);
    }
}
