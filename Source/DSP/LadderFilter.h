#pragma once
#include <JuceHeader.h>

// ═══════════════════════════════════════════════════════════════════════════════
// Resonant Low-Pass Filter
//
// Uses JUCE's built-in dsp::StateVariableTPTFilter which implements a
// numerically stable trapezoidal SVF. No hand-rolled math to go wrong.
// ═══════════════════════════════════════════════════════════════════════════════

class LadderFilter
{
public:
    LadderFilter();

    void prepare (double sampleRate, int samplesPerBlock, int numChannels);
    void reset();
    void process (juce::AudioBuffer<float>& buffer);

    void setCutoff (float hz)   { cutoffHz = hz; }
    void setResonance (float r) { resonance = r; }
    void setEnabled (bool e)    { enabled = e; }

private:
    // One filter per channel
    juce::dsp::StateVariableTPTFilter<float> svf[2];

    float cutoffHz  = 20000.0f;
    float resonance = 0.0f;
    bool  enabled   = true;
    double currentSampleRate = 44100.0;
};
