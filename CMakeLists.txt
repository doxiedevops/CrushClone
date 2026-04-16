cmake_minimum_required(VERSION 3.22)
project(CrushClone VERSION 1.0.0)

# ── Fetch JUCE ────────────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.12          # pinned to stable release for reproducibility
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# ── Plugin target ─────────────────────────────────────────────────────────────
juce_add_plugin(CrushClone
    COMPANY_NAME            "HTNY Studios"
    PLUGIN_MANUFACTURER_CODE Htny
    PLUGIN_CODE              CrCl
    FORMATS                  VST3 AU Standalone   # build all three; deploy VST3
    PRODUCT_NAME             "CrushClone"
    BUNDLE_ID                "com.htnystudios.crushclone"
    IS_SYNTH                 FALSE
    NEEDS_MIDI_INPUT         FALSE
    NEEDS_MIDI_OUTPUT        FALSE
    IS_MIDI_EFFECT           FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD  FALSE                 # auto-install after build
    VST3_COPY_DIR            "/Library/Audio/Plug-Ins/VST3"
    AU_COPY_DIR              "$ENV{HOME}/Library/Audio/Plug-Ins/Components"
)

juce_generate_juce_header(CrushClone)

# ── Web UI binary data ───────────────────────────────────────────────────────
juce_add_binary_data(CrushCloneWebUI
    HEADER_NAME "WebUIData.h"
    NAMESPACE WebUIData
    SOURCES
        Source/WebUI/crushclone.html
)

# ── Source files ──────────────────────────────────────────────────────────────
target_sources(CrushClone
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/DSP/Distortion.cpp
        Source/DSP/LadderFilter.cpp
        Source/DSP/Compressor.cpp
        Source/DSP/Oversampler.cpp
)

# ── Compile definitions ──────────────────────────────────────────────────────
target_compile_definitions(CrushClone
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

# ── Link JUCE modules ────────────────────────────────────────────────────────
target_link_libraries(CrushClone
    PRIVATE
        CrushCloneWebUI
        juce::juce_audio_utils
        juce::juce_gui_extra
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ── C++17 ─────────────────────────────────────────────────────────────────────
target_compile_features(CrushClone PRIVATE cxx_std_17)
