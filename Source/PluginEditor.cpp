#include "PluginEditor.h"
#include "WebUIData.h"
#include <cmath>

// ═════════════════════════════════════════════════════════════════════════════
// PluginBrowser — intercepts custom URL scheme for JS→C++ bridge
// ═════════════════════════════════════════════════════════════════════════════

CrushCloneEditor::PluginBrowser::PluginBrowser()
    : WebBrowserComponent (WebBrowserComponent::Options{})
{
}

bool CrushCloneEditor::PluginBrowser::pageAboutToLoad (const juce::String& url)
{
    // Intercept: crushclone://param/paramId/value
    if (url.startsWith ("crushclone://param/"))
    {
        auto parts = juce::StringArray::fromTokens (url.fromFirstOccurrenceOf ("crushclone://param/", false, true), "/", "");

        if (parts.size() >= 2)
        {
            auto id = juce::URL::removeEscapeChars (parts[0]);
            auto value = parts[1].getFloatValue();

            if (id == "ready" && onUIReady)
                onUIReady();
            else if (id == "preset" && onPresetFromJS)
                onPresetFromJS ((int) value);
            else if (onParamFromJS)
                onParamFromJS (id, value);
        }
        return false; // prevent actual navigation
    }

    return true; // allow normal page loads
}

void CrushCloneEditor::PluginBrowser::pageFinishedLoading (const juce::String&)
{
    // Page might signal readiness via the bridge instead
}

// ═════════════════════════════════════════════════════════════════════════════
// Editor construction
// ═════════════════════════════════════════════════════════════════════════════

CrushCloneEditor::CrushCloneEditor (CrushCloneProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setSize (452, 540);
    setResizable (false, false);

    addAndMakeVisible (browser);

    // ── Bridge callbacks ─────────────────────────────────────────────────
    browser.onParamFromJS = [this] (const juce::String& id, float value)
    {
        if (auto* param = proc.getAPVTS().getParameter (id))
            param->setValueNotifyingHost (param->convertTo0to1 (value));
    };

    browser.onPresetFromJS = [this] (int index)
    {
        proc.loadPreset (index);
        // Push all new values back to JS after a short delay for state to settle
        juce::Timer::callAfterDelay (50, [this] { pushAllParamsToJS(); });
    };

    browser.onUIReady = [this]
    {
        uiReady = true;
        pushAllParamsToJS();
    };

    // ── Write HTML to temp file and load ─────────────────────────────────
    // Use a unique subdirectory to prevent symlink/TOCTOU attacks on
    // multi-user systems (predictable temp paths are a security risk).
    auto tempDir = juce::File::getSpecialLocation (juce::File::tempDirectory)
                       .getChildFile ("crushclone_" + juce::String (juce::Random::getSystemRandom().nextInt64()));
    tempDir.createDirectory();
    htmlFile = tempDir.getChildFile ("crushclone_ui.html");
    htmlFile.replaceWithData (WebUIData::crushclone_html, WebUIData::crushclone_htmlSize);
    browser.goToURL ("file://" + htmlFile.getFullPathName());

    startTimerHz (20); // 20 Hz poll for automation changes
}

CrushCloneEditor::~CrushCloneEditor()
{
    stopTimer();
    auto parentDir = htmlFile.getParentDirectory();
    htmlFile.deleteFile();
    parentDir.deleteRecursively();
}

// ═════════════════════════════════════════════════════════════════════════════
// Push parameter values from C++ to JS
// ═════════════════════════════════════════════════════════════════════════════

static juce::String escapeForJS (const juce::String& s)
{
    return s.replace ("\\", "\\\\").replace ("'", "\\'");
}

void CrushCloneEditor::pushParamToJS (const juce::String& id, float value)
{
    if (! uiReady) return;

    auto js = "window.setPluginParam('" + escapeForJS (id) + "'," + juce::String (value, 4) + ")";
    browser.evaluateJavascript (js, nullptr);
}

void CrushCloneEditor::pushAllParamsToJS()
{
    if (! uiReady) return;

    auto& vts = proc.getAPVTS();

    auto getVal = [&](const juce::String& id) -> float {
        if (auto* p = vts.getRawParameterValue (id))
            return p->load();
        return 0.0f;
    };

    float t = getVal("tubeAmount"),    m = getVal("mechAmount");
    float c = getVal("filterCutoff"),  r = getVal("filterReso");
    float ca = getVal("compAmount"),   v = getVal("masterVolume");
    float mx = getVal("dryWet");
    float dOn = getVal("distEnable"),  fOn = getVal("filterEnable");
    float cOn = getVal("compEnable"),  ph = getVal("phatMode");

    juce::String js = "window.setPluginParamBatch({"
        "tubeAmount:" + juce::String(t, 4) + ","
        "mechAmount:" + juce::String(m, 4) + ","
        "filterCutoff:" + juce::String(c, 1) + ","
        "filterReso:" + juce::String(r, 4) + ","
        "compAmount:" + juce::String(ca, 4) + ","
        "masterVolume:" + juce::String(v, 2) + ","
        "dryWet:" + juce::String(mx, 4) + ","
        "distEnable:" + juce::String(dOn, 1) + ","
        "filterEnable:" + juce::String(fOn, 1) + ","
        "compEnable:" + juce::String(cOn, 1) + ","
        "phatMode:" + juce::String(ph, 1) + ","
        "preset:" + juce::String(proc.getCurrentProgram()) +
        "})";

    browser.evaluateJavascript (js, nullptr);

    lastPushed = { t, m, c, r, ca, v, mx, dOn, fOn, cOn, ph };
}

// ═════════════════════════════════════════════════════════════════════════════
// Timer — push automation changes to JS
// ═════════════════════════════════════════════════════════════════════════════

void CrushCloneEditor::timerCallback()
{
    if (! uiReady) return;

    auto& vts = proc.getAPVTS();
    auto getVal = [&](const juce::String& id) -> float {
        if (auto* p = vts.getRawParameterValue (id))
            return p->load();
        return 0.0f;
    };

    auto changed = [](float a, float b) { return std::abs (a - b) > 0.001f; };

    float t = getVal("tubeAmount");    if (changed(t, lastPushed.tube))    { pushParamToJS("tubeAmount", t);    lastPushed.tube = t; }
    float m = getVal("mechAmount");    if (changed(m, lastPushed.mech))    { pushParamToJS("mechAmount", m);    lastPushed.mech = m; }
    float c = getVal("filterCutoff");  if (changed(c, lastPushed.cutoff))  { pushParamToJS("filterCutoff", c);  lastPushed.cutoff = c; }
    float r = getVal("filterReso");    if (changed(r, lastPushed.reso))    { pushParamToJS("filterReso", r);    lastPushed.reso = r; }
    float ca = getVal("compAmount");   if (changed(ca, lastPushed.compAmt)){ pushParamToJS("compAmount", ca);   lastPushed.compAmt = ca; }
    float v = getVal("masterVolume");  if (changed(v, lastPushed.volume))  { pushParamToJS("masterVolume", v);  lastPushed.volume = v; }
    float mx = getVal("dryWet");       if (changed(mx, lastPushed.mix))    { pushParamToJS("dryWet", mx);       lastPushed.mix = mx; }
    float dOn = getVal("distEnable");  if (changed(dOn, lastPushed.distOn)){ pushParamToJS("distEnable", dOn);  lastPushed.distOn = dOn; }
    float fOn = getVal("filterEnable");if (changed(fOn, lastPushed.filterOn)){pushParamToJS("filterEnable", fOn);lastPushed.filterOn = fOn; }
    float cOn = getVal("compEnable");  if (changed(cOn, lastPushed.compOn)){ pushParamToJS("compEnable", cOn);  lastPushed.compOn = cOn; }
    float ph = getVal("phatMode");     if (changed(ph, lastPushed.phat))   { pushParamToJS("phatMode", ph);     lastPushed.phat = ph; }
}

// ═════════════════════════════════════════════════════════════════════════════
// Paint / Layout
// ═════════════════════════════════════════════════════════════════════════════

void CrushCloneEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF0C0B12));
}

void CrushCloneEditor::resized()
{
    browser.setBounds (getLocalBounds());
}
