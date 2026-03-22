#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class CrushCloneEditor : public juce::AudioProcessorEditor,
                          public juce::Timer
{
public:
    explicit CrushCloneEditor (CrushCloneProcessor&);
    ~CrushCloneEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    CrushCloneProcessor& proc;

    // WebBrowser subclass to intercept navigation for the JS→C++ bridge
    class PluginBrowser : public juce::WebBrowserComponent
    {
    public:
        PluginBrowser();
        bool pageAboutToLoad (const juce::String& url) override;
        void pageFinishedLoading (const juce::String& url) override;

        std::function<void (const juce::String& id, float value)> onParamFromJS;
        std::function<void (int index)> onPresetFromJS;
        std::function<void()> onUIReady;
    };

    PluginBrowser browser;
    juce::File htmlFile;
    bool uiReady = false;

    // Track last pushed values to avoid feedback loops
    struct ParamState {
        float tube=0, mech=0, cutoff=20000, reso=0, compAmt=0;
        float volume=0, mix=1;
        float distOn=1, filterOn=0, compOn=1, phat=0;
    };
    ParamState lastPushed;

    void pushAllParamsToJS();
    void pushParamToJS (const juce::String& id, float value);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CrushCloneEditor)
};
