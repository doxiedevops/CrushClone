#pragma once
#include <JuceHeader.h>

// ═══════════════════════════════════════════════════════════════════════════════
// Compressor Module — CamelCrusher–style one-knob dynamics
//
// AMOUNT knob: Single control that simultaneously adjusts threshold, ratio,
//   attack, release, and makeup gain for an always-musical response.
//
// PHAT MODE: Engages parallel (New York) compression with a low-frequency boost.
//   The dry signal is blended with a heavily compressed + low-shelved copy,
//   thickening the bass while preserving transients.
// ═══════════════════════════════════════════════════════════════════════════════

class CompressorModule
{
public:
    CompressorModule();

    void prepare (double sampleRate, int samplesPerBlock, int numChannels);
    void reset();
    void process (juce::AudioBuffer<float>& buffer);

    // Amount: 0.0 = off, 1.0 = maximum compression
    void setAmount (float v)        { amount = v; }
    // Phat mode on/off
    void setPhatMode (bool on)      { phatMode = on; }
    void setEnabled (bool e)        { enabled = e; }

    // Read-only: current gain reduction in dB (for metering)
    float getGainReductionDB() const { return gainReductionDB; }

private:
    // ── Envelope follower ────────────────────────────────────────────────────
    float computeEnvelope (float inputLevel);

    // ── Gain computer ────────────────────────────────────────────────────────
    float computeGain (float envelopeDB) const;

    // ── Phat mode low shelf ──────────────────────────────────────────────────
    using IIRCoeffs = juce::dsp::IIR::Coefficients<float>;
    using IIRFilter = juce::dsp::IIR::Filter<float>;
    IIRFilter phatShelf[2];

    // ── State ────────────────────────────────────────────────────────────────
    float envelopeState   = 0.0f;     // smoothed RMS level
    float gainReductionDB = 0.0f;     // current GR for metering

    // ── Parameters ───────────────────────────────────────────────────────────
    float  amount   = 0.0f;
    bool   phatMode = false;
    bool   enabled  = true;

    double currentSampleRate = 44100.0;
};
