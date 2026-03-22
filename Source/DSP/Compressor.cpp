#include "Compressor.h"
#include <cmath>
#include <algorithm>

// ═════════════════════════════════════════════════════════════════════════════
// Construction / lifecycle
// ═════════════════════════════════════════════════════════════════════════════

CompressorModule::CompressorModule() {}

void CompressorModule::prepare (double sampleRate, int /*samplesPerBlock*/, int /*numChannels*/)
{
    currentSampleRate = sampleRate;

    // Phat mode low shelf: +8 dB at 120 Hz
    auto coeffs = IIRCoeffs::makeLowShelf (sampleRate, 120.0, 0.7f, 2.5f);
    for (int ch = 0; ch < 2; ++ch)
    {
        *phatShelf[ch].coefficients = *coeffs;
        phatShelf[ch].reset();
    }

    envelopeState   = 0.0f;
    gainReductionDB = 0.0f;
}

void CompressorModule::reset()
{
    envelopeState   = 0.0f;
    gainReductionDB = 0.0f;
    for (int ch = 0; ch < 2; ++ch)
        phatShelf[ch].reset();
}

// ═════════════════════════════════════════════════════════════════════════════
// Envelope follower — RMS with adaptive attack/release
// ═════════════════════════════════════════════════════════════════════════════

float CompressorModule::computeEnvelope (float inputLevel)
{
    // Attack and release times derived from the Amount knob
    // Lower amount → slower/gentler; higher → faster/punchier
    const float attackMs  = juce::jmap (amount, 10.0f, 1.0f);
    const float releaseMs = juce::jmap (amount, 150.0f, 50.0f);

    const float attackCoeff  = std::exp (-1.0f / (attackMs  * 0.001f * static_cast<float>(currentSampleRate)));
    const float releaseCoeff = std::exp (-1.0f / (releaseMs * 0.001f * static_cast<float>(currentSampleRate)));

    const float coeff = (inputLevel > envelopeState) ? attackCoeff : releaseCoeff;
    envelopeState = coeff * envelopeState + (1.0f - coeff) * inputLevel;

    return envelopeState;
}

// ═════════════════════════════════════════════════════════════════════════════
// Gain computer — soft-knee compression curve
// ═════════════════════════════════════════════════════════════════════════════

float CompressorModule::computeGain (float envelopeDB) const
{
    // Amount maps to threshold and ratio simultaneously:
    //   amount 0.0 → threshold  0 dB, ratio 1:1 (no compression)
    //   amount 1.0 → threshold -30 dB, ratio 8:1 (heavy squash)
    const float threshold = juce::jmap (amount, 0.0f, -30.0f);
    const float ratio     = juce::jmap (amount, 1.0f, 8.0f);
    const float kneeWidth = 6.0f;  // dB — soft knee

    float gainDB = 0.0f;

    if (envelopeDB < threshold - kneeWidth * 0.5f)
    {
        // Below knee — no compression
        gainDB = 0.0f;
    }
    else if (envelopeDB > threshold + kneeWidth * 0.5f)
    {
        // Above knee — full compression
        gainDB = (threshold + (envelopeDB - threshold) / ratio) - envelopeDB;
    }
    else
    {
        // Inside soft knee
        const float x = envelopeDB - threshold + kneeWidth * 0.5f;
        gainDB = ((1.0f / ratio - 1.0f) * x * x) / (2.0f * kneeWidth);
    }

    return gainDB;
}

// ═════════════════════════════════════════════════════════════════════════════
// Process
// ═════════════════════════════════════════════════════════════════════════════

void CompressorModule::process (juce::AudioBuffer<float>& buffer)
{
    if (! enabled || amount < 0.001f)
        return;

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Makeup gain: compensate for the volume loss from compression
    const float makeupDB   = amount * 12.0f;
    const float makeupGain = std::pow (10.0f, makeupDB / 20.0f);

    // If Phat mode, we need the dry signal for parallel blending
    juce::AudioBuffer<float> dryBuffer;
    if (phatMode)
    {
        dryBuffer.makeCopyOf (buffer);
    }

    for (int i = 0; i < numSamples; ++i)
    {
        // ── Sidechain: peak across all channels ──────────────────────────
        float peakLevel = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            peakLevel = std::max (peakLevel, std::abs (buffer.getSample (ch, i)));

        // Convert to dB, floor at -80 dB
        const float peakDB = (peakLevel > 1e-4f)
                           ? 20.0f * std::log10 (peakLevel)
                           : -80.0f;

        // ── Envelope and gain ────────────────────────────────────────────
        const float envDB  = 20.0f * std::log10 (std::max (computeEnvelope (peakLevel), 1e-8f));
        const float gainDB = computeGain (envDB);
        gainReductionDB    = gainDB;

        const float gainLinear = std::pow (10.0f, gainDB / 20.0f) * makeupGain;

        // ── Apply gain to all channels ───────────────────────────────────
        for (int ch = 0; ch < numChannels; ++ch)
            buffer.setSample (ch, i, buffer.getSample (ch, i) * gainLinear);
    }

    // ── Phat mode: parallel compression + low shelf ──────────────────────
    if (phatMode)
    {
        // Blend factor: more Amount → more parallel mix
        const float phatMix = juce::jmap (amount, 0.2f, 0.65f);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* wet = buffer.getWritePointer (ch);
            const float* dry = dryBuffer.getReadPointer (ch);

            for (int i = 0; i < numSamples; ++i)
            {
                // Apply low shelf to the compressed signal for bass thickness
                float shelved = phatShelf[ch].processSample (wet[i]);

                // Parallel blend: dry + compressed/shelved
                wet[i] = dry[i] * (1.0f - phatMix) + shelved * phatMix;
            }
        }
    }
}
