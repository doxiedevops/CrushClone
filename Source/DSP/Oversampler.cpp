#include "Oversampler.h"

Oversampler::Oversampler() {}

void Oversampler::prepare (double sampleRate, int samplesPerBlock, int numChannels)
{
    oversampling.initProcessing (static_cast<size_t> (samplesPerBlock));
    oversampling.reset();
}

void Oversampler::reset()
{
    oversampling.reset();
}

float Oversampler::getLatencyInSamples() const
{
    return oversampling.getLatencyInSamples();
}
