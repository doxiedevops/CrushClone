#pragma once
#include <JuceHeader.h>
#include "DSP/Distortion.h"
#include "DSP/LadderFilter.h"
#include "DSP/Compressor.h"

struct CrushPreset
{
    juce::String name;
    float tube, mech, cutoff, reso, comp;
    bool phat, distOn, filterOn, compOn;
    float master, mix;
};

class CrushCloneProcessor : public juce::AudioProcessor
{
public:
    CrushCloneProcessor();
    ~CrushCloneProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool   acceptsMidi()  const override { return false; }
    bool   producesMidi() const override { return false; }
    bool   isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int  getNumPrograms() override    { return (int) presets.size(); }
    int  getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation    (juce::MemoryBlock& destData) override;
    void setStateInformation    (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    float getGainReductionDB() const { return compressor.getGainReductionDB(); }

    const std::vector<CrushPreset>& getPresets() const { return presets; }
    void loadPreset (int index);

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts;

    DistortionModule  distortion;
    LadderFilter      filter;
    CompressorModule  compressor;

    std::atomic<float>* tubeAmountParam   = nullptr;
    std::atomic<float>* mechAmountParam   = nullptr;
    std::atomic<float>* distEnableParam   = nullptr;
    std::atomic<float>* filterCutoffParam = nullptr;
    std::atomic<float>* filterResoParam   = nullptr;
    std::atomic<float>* filterEnableParam = nullptr;
    std::atomic<float>* compAmountParam   = nullptr;
    std::atomic<float>* phatModeParam     = nullptr;
    std::atomic<float>* compEnableParam   = nullptr;
    std::atomic<float>* masterVolumeParam = nullptr;
    std::atomic<float>* dryWetParam       = nullptr;

    std::vector<CrushPreset> presets;
    int currentPreset = 0;
    void initPresets();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrushCloneProcessor)
};
