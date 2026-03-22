#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace ParamIDs
{
    static const juce::String tubeAmount   = "tubeAmount";
    static const juce::String mechAmount   = "mechAmount";
    static const juce::String distEnable   = "distEnable";
    static const juce::String filterCutoff = "filterCutoff";
    static const juce::String filterReso   = "filterReso";
    static const juce::String filterEnable = "filterEnable";
    static const juce::String compAmount   = "compAmount";
    static const juce::String phatMode     = "phatMode";
    static const juce::String compEnable   = "compEnable";
    static const juce::String masterVolume = "masterVolume";
    static const juce::String dryWet       = "dryWet";
}

// ═════════════════════════════════════════════════════════════════════════════
// Presets
// ═════════════════════════════════════════════════════════════════════════════

void CrushCloneProcessor::initPresets()
{
    //                          name              tube  mech  cutoff    reso  comp  phat   dOn   fOn   cOn   mst   mix
    presets.push_back ({ "Default",              0.0f, 0.0f, 20000.f, 0.0f, 0.0f, false, true, false,true, 0.0f, 1.0f });
    presets.push_back ({ "DRM BigBadBeat",       0.5f, 0.3f,  4000.f, 0.2f, 0.6f, true,  true, true, true, 0.0f, 1.0f });
    presets.push_back ({ "BAS FatSub",           0.7f, 0.0f,   800.f, 0.3f, 0.5f, true,  true, true, true, 0.0f, 1.0f });
    presets.push_back ({ "GTR Crunch",           0.4f, 0.5f,  6000.f, 0.1f, 0.3f, false, true, true, true, 0.0f, 1.0f });
    presets.push_back ({ "SYN WarmPad",          0.3f, 0.0f,  3000.f, 0.4f, 0.2f, false, true, true, true, 0.0f, 1.0f });
    presets.push_back ({ "VOX Gritty",           0.6f, 0.2f,  5000.f, 0.15f,0.4f, false, true, true, true, 0.0f, 1.0f });
    presets.push_back ({ "MAS Loudener",         0.2f, 0.1f, 12000.f, 0.0f, 0.7f, true,  true, true, true, 2.0f, 1.0f });
    presets.push_back ({ "DRM LoFiHit",          0.8f, 0.6f,  2000.f, 0.5f, 0.4f, false, true, true, true,-2.0f, 1.0f });
    presets.push_back ({ "SYN Screamer",         0.3f, 0.8f,  7000.f, 0.6f, 0.5f, true,  true, true, true,-1.0f, 1.0f });
    presets.push_back ({ "Gentle Warmth",        0.2f, 0.0f, 15000.f, 0.0f, 0.1f, false, true, true, true, 0.0f, 0.5f });
    presets.push_back ({ "Telephone",            0.1f, 0.0f,  3500.f, 0.3f, 0.0f, false, true, true, false,0.0f, 1.0f });
    presets.push_back ({ "Destroy Everything",   1.0f, 1.0f,  1000.f, 0.7f, 0.8f, true,  true, true, true,-4.0f, 1.0f });
}

void CrushCloneProcessor::loadPreset (int index)
{
    if (index < 0 || index >= (int) presets.size()) return;
    currentPreset = index;
    const auto& p = presets[(size_t) index];

    auto set = [&](const juce::String& id, float v) {
        if (auto* param = apvts.getParameter (id))
            param->setValueNotifyingHost (param->convertTo0to1 (v));
    };

    set (ParamIDs::tubeAmount,   p.tube);
    set (ParamIDs::mechAmount,   p.mech);
    set (ParamIDs::filterCutoff, p.cutoff);
    set (ParamIDs::filterReso,   p.reso);
    set (ParamIDs::compAmount,   p.comp);
    set (ParamIDs::phatMode,     p.phat ? 1.0f : 0.0f);
    set (ParamIDs::distEnable,   p.distOn ? 1.0f : 0.0f);
    set (ParamIDs::filterEnable, p.filterOn ? 1.0f : 0.0f);
    set (ParamIDs::compEnable,   p.compOn ? 1.0f : 0.0f);
    set (ParamIDs::masterVolume, p.master);
    set (ParamIDs::dryWet,       p.mix);
}

void CrushCloneProcessor::setCurrentProgram (int index) { loadPreset (index); }

const juce::String CrushCloneProcessor::getProgramName (int index)
{
    if (index >= 0 && index < (int) presets.size())
        return presets[(size_t) index].name;
    return {};
}

// ═════════════════════════════════════════════════════════════════════════════
// Construction
// ═════════════════════════════════════════════════════════════════════════════

CrushCloneProcessor::CrushCloneProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "CrushCloneState", createParameterLayout())
{
    tubeAmountParam   = apvts.getRawParameterValue (ParamIDs::tubeAmount);
    mechAmountParam   = apvts.getRawParameterValue (ParamIDs::mechAmount);
    distEnableParam   = apvts.getRawParameterValue (ParamIDs::distEnable);
    filterCutoffParam = apvts.getRawParameterValue (ParamIDs::filterCutoff);
    filterResoParam   = apvts.getRawParameterValue (ParamIDs::filterReso);
    filterEnableParam = apvts.getRawParameterValue (ParamIDs::filterEnable);
    compAmountParam   = apvts.getRawParameterValue (ParamIDs::compAmount);
    phatModeParam     = apvts.getRawParameterValue (ParamIDs::phatMode);
    compEnableParam   = apvts.getRawParameterValue (ParamIDs::compEnable);
    masterVolumeParam = apvts.getRawParameterValue (ParamIDs::masterVolume);
    dryWetParam       = apvts.getRawParameterValue (ParamIDs::dryWet);

    initPresets();
}

CrushCloneProcessor::~CrushCloneProcessor() {}

// ═════════════════════════════════════════════════════════════════════════════
// Parameters — note: filter defaults to OFF to prevent issues on load
// ═════════════════════════════════════════════════════════════════════════════

juce::AudioProcessorValueTreeState::ParameterLayout CrushCloneProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::tubeAmount, 1 }, "Tube",
        juce::NormalisableRange<float> (0.f, 1.f, 0.01f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::mechAmount, 1 }, "Mech",
        juce::NormalisableRange<float> (0.f, 1.f, 0.01f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::distEnable, 1 }, "Distortion On", true));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::filterCutoff, 1 }, "Cutoff",
        juce::NormalisableRange<float> (20.f, 20000.f, 1.f, 0.3f), 20000.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::filterReso, 1 }, "Resonance",
        juce::NormalisableRange<float> (0.f, 1.f, 0.01f), 0.0f));

    // FILTER DEFAULTS TO OFF — user turns it on when they want it
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::filterEnable, 1 }, "Filter On", false));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::compAmount, 1 }, "Comp Amount",
        juce::NormalisableRange<float> (0.f, 1.f, 0.01f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::phatMode, 1 }, "Phat", false));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::compEnable, 1 }, "Compressor On", true));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::masterVolume, 1 }, "Master",
        juce::NormalisableRange<float> (-24.f, 12.f, 0.1f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::dryWet, 1 }, "Mix",
        juce::NormalisableRange<float> (0.f, 1.f, 0.01f), 1.0f));

    return { params.begin(), params.end() };
}

// ═════════════════════════════════════════════════════════════════════════════
// Prepare / release
// ═════════════════════════════════════════════════════════════════════════════

void CrushCloneProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const int numCh = getTotalNumOutputChannels();
    distortion.prepare (sampleRate, samplesPerBlock, numCh);
    filter.prepare     (sampleRate, samplesPerBlock, numCh);
    compressor.prepare (sampleRate, samplesPerBlock, numCh);
}

void CrushCloneProcessor::releaseResources()
{
    distortion.reset();
    filter.reset();
    compressor.reset();
}

bool CrushCloneProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    const auto& mainIn  = layouts.getMainInputChannelSet();

    if (mainOut != juce::AudioChannelSet::stereo()
        && mainOut != juce::AudioChannelSet::mono())
        return false;

    return mainIn == mainOut;
}

// ═════════════════════════════════════════════════════════════════════════════
// Process block
// ═════════════════════════════════════════════════════════════════════════════

void CrushCloneProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Read all parameters
    distortion.setTubeAmount (tubeAmountParam->load());
    distortion.setMechAmount (mechAmountParam->load());
    distortion.setEnabled    (distEnableParam->load() > 0.5f);

    filter.setCutoff    (filterCutoffParam->load());
    filter.setResonance (filterResoParam->load());
    filter.setEnabled   (filterEnableParam->load() > 0.5f);

    compressor.setAmount   (compAmountParam->load());
    compressor.setPhatMode (phatModeParam->load() > 0.5f);
    compressor.setEnabled  (compEnableParam->load() > 0.5f);

    const float masterDB = masterVolumeParam->load();
    const float dryWet   = dryWetParam->load();

    // Dry copy for mix
    juce::AudioBuffer<float> dryBuffer;
    if (dryWet < 0.999f)
        dryBuffer.makeCopyOf (buffer);

    // === Signal chain ===
    distortion.process (buffer);
    filter.process (buffer);
    compressor.process (buffer);

    // Master gain
    if (std::abs (masterDB) > 0.05f)
    {
        const float gain = std::pow (10.0f, masterDB / 20.0f);
        buffer.applyGain (gain);
    }

    // Dry/wet
    if (dryWet < 0.999f)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* wet = buffer.getWritePointer (ch);
            const float* dry = dryBuffer.getReadPointer (ch);
            for (int i = 0; i < numSamples; ++i)
                wet[i] = dry[i] * (1.0f - dryWet) + wet[i] * dryWet;
        }
    }

    // === SAFETY HARD LIMITER — protect speakers no matter what ===
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = buffer.getWritePointer (ch);
        for (int i = 0; i < numSamples; ++i)
        {
            if (data[i] > 2.0f)        data[i] = 2.0f;
            else if (data[i] < -2.0f)   data[i] = -2.0f;
            // Kill denormals / NaN / Inf
            if (! std::isfinite (data[i])) data[i] = 0.0f;
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// State
// ═════════════════════════════════════════════════════════════════════════════

void CrushCloneProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("currentPreset", currentPreset, nullptr);
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void CrushCloneProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml && xml->hasTagName (apvts.state.getType()))
    {
        auto s = juce::ValueTree::fromXml (*xml);
        currentPreset = s.getProperty ("currentPreset", 0);
        apvts.replaceState (s);
    }
}

juce::AudioProcessorEditor* CrushCloneProcessor::createEditor()
{
    return new CrushCloneEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CrushCloneProcessor();
}
