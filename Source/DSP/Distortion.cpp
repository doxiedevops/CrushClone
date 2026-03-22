#include "Distortion.h"
#include <cmath>

// ═════════════════════════════════════════════════════════════════════════════
// Construction / lifecycle
// ═════════════════════════════════════════════════════════════════════════════

DistortionModule::DistortionModule() {}

void DistortionModule::prepare (double sampleRate, int samplesPerBlock, int numCh)
{
    currentSampleRate = sampleRate;
    numChannels       = numCh;

    oversampler.prepare (sampleRate, samplesPerBlock, numCh);
    updateMechEQ();
}

void DistortionModule::reset()
{
    oversampler.reset();
    for (int ch = 0; ch < 2; ++ch)
    {
        mechLowShelf[ch].reset();
        mechHighShelf[ch].reset();
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// Waveshaper cores
// ═════════════════════════════════════════════════════════════════════════════

float DistortionModule::tubeSaturate (float x, float drive)
{
    // drive maps [0,1] → gain [1, 20]
    const float gain = 1.0f + drive * 19.0f;

    // Asymmetric bias — adds subtle even harmonics (tube character)
    const float bias = drive * 0.08f;
    float s = x * gain + bias;

    // Soft-clip via tanh
    s = std::tanh (s);

    // Compensate for DC offset introduced by bias
    s -= std::tanh (bias);

    return s;
}

float DistortionModule::mechClip (float x, float drive)
{
    // drive maps [0,1] → gain [1, 30]  (more aggressive than tube)
    const float gain = 1.0f + drive * 29.0f;
    float s = x * gain;

    // Hard clip with slight cubic softening at the knee
    // This avoids the pure digital buzz of a raw min/max clip
    if (s > 1.0f)
        s = 1.0f;
    else if (s < -1.0f)
        s = -1.0f;
    else
    {
        // Soft polynomial knee in [-1, 1] range for moderate gains
        // At extreme gains the cubic term becomes negligible
        const float k = 0.15f * drive;
        s = s - k * (s * s * s);
    }

    return s;
}

// ═════════════════════════════════════════════════════════════════════════════
// Mech pre-emphasis EQ — scoop mids, push lows + highs
// ═════════════════════════════════════════════════════════════════════════════

void DistortionModule::updateMechEQ()
{
    // These run at the OVERSAMPLED rate (4× base)
    const double osRate = currentSampleRate * 4.0;

    auto lowCoeffs  = IIRCoeffs::makeLowShelf  (osRate, 200.0,  0.7f, 1.8f);
    auto highCoeffs = IIRCoeffs::makeHighShelf (osRate, 4000.0, 0.7f, 2.0f);

    for (int ch = 0; ch < 2; ++ch)
    {
        *mechLowShelf[ch].coefficients  = *lowCoeffs;
        *mechHighShelf[ch].coefficients = *highCoeffs;
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// Main process
// ═════════════════════════════════════════════════════════════════════════════

void DistortionModule::process (juce::AudioBuffer<float>& buffer)
{
    if (! enabled || (tubeAmount < 0.001f && mechAmount < 0.001f))
        return;

    const float tube = tubeAmount;
    const float mech = mechAmount;
    const int   nCh  = buffer.getNumChannels();

    oversampler.process (buffer, [&] (juce::dsp::AudioBlock<float>& block)
    {
        const int numSamples = static_cast<int> (block.getNumSamples());

        for (int ch = 0; ch < nCh; ++ch)
        {
            float* data = block.getChannelPointer (static_cast<size_t> (ch));

            for (int i = 0; i < numSamples; ++i)
            {
                float dry = data[i];
                float out = 0.0f;

                // ── Tube path ────────────────────────────────────────────
                if (tube > 0.001f)
                    out += tube * tubeSaturate (dry, tube);

                // ── Mech path (pre-emphasis → hard clip) ─────────────────
                if (mech > 0.001f)
                {
                    float m = dry;
                    m = mechLowShelf[ch].processSample (m);
                    m = mechHighShelf[ch].processSample (m);
                    m = mechClip (m, mech);
                    out += mech * m;
                }

                // ── Blend: if both active, normalise so combined level
                //    doesn't exceed ~1× the input amplitude ──────────────
                const float totalDrive = tube + mech;
                if (totalDrive > 1.0f)
                    out /= totalDrive;

                data[i] = out;
            }
        }
    });
}
