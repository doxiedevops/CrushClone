#pragma once
#include <JuceHeader.h>
#include "Oversampler.h"

// ═══════════════════════════════════════════════════════════════════════════════
// Distortion Module — faithful recreation of CamelCrusher's dual-mode distortion
//
// TUBE: Asymmetric tanh waveshaper with DC bias for even-harmonic warmth.
//       Soft-clips smoothly, preserving low-end body.
//
// MECH: Hard clipper preceded by a low+high shelf pre-emphasis EQ that
//       scoops the mids and pushes the extremes — the aggressive,
//       "mechanical" character of the original.
//
// Both paths are 4× oversampled to prevent aliasing.
// ═══════════════════════════════════════════════════════════════════════════════

class DistortionModule
{
public:
    DistortionModule();

    void prepare (double sampleRate, int samplesPerBlock, int numChannels);
    void reset();
    void process (juce::AudioBuffer<float>& buffer);

    // Parameters [0.0 … 1.0]
    void setTubeAmount (float v)    { tubeAmount = v; }
    void setMechAmount (float v)    { mechAmount = v; }
    void setEnabled (bool e)        { enabled = e; }

private:
    // ── Waveshapers ──────────────────────────────────────────────────────────
    static float tubeSaturate (float x, float drive);
    static float mechClip     (float x, float drive);

    // ── Pre-emphasis EQ for Mech mode ────────────────────────────────────────
    void updateMechEQ();

    // Using JUCE's IIR filters for the pre-emphasis
    using IIRCoeffs = juce::dsp::IIR::Coefficients<float>;
    using IIRFilter = juce::dsp::IIR::Filter<float>;

    // Per-channel filters (stereo)
    IIRFilter mechLowShelf[2];
    IIRFilter mechHighShelf[2];

    // ── State ────────────────────────────────────────────────────────────────
    Oversampler oversampler;
    float tubeAmount = 0.0f;
    float mechAmount = 0.0f;
    bool  enabled    = true;

    double currentSampleRate = 44100.0;
    int    numChannels       = 2;
};
