# CrushClone — CamelCrusher VST3 Recreation

A faithful recreation of the legendary CamelCrusher multi-effect plugin, built as a
native Apple Silicon VST3 using the JUCE framework.

## Signal Chain

```
Audio In → Distortion (Tube + Mech) → Low-Pass Filter → Compressor (+ Phat) → Master → Audio Out
```

## Features

- **Tube Saturation** — asymmetric tanh waveshaper with even-harmonic bias, 4× oversampled
- **Mech Distortion** — hard clipper with low+high shelf pre-emphasis, 4× oversampled
- **Resonant Low-Pass Filter** — zero-delay feedback SVF with nonlinear resonance
- **One-Knob Compressor** — maps Amount to threshold/ratio/attack/release/makeup
- **Phat Mode** — parallel compression + low-shelf boost for bass thickness
- **Master Volume + Dry/Wet Mix**
- **Full automation** via APVTS — every parameter automatable in FL Studio
- **State recall** — presets save/restore with FL Studio projects

## Requirements

| Tool         | Version            |
|--------------|--------------------|
| macOS        | 13+ (Ventura)      |
| Xcode        | 15+                |
| CMake        | 3.22+              |
| Processor    | Apple Silicon (M1/M2/M3/M4) or Intel |
| FL Studio    | 25.x (macOS)       |

## Build Instructions

### 1. Install prerequisites

```bash
# Install Xcode from the App Store, then:
xcode-select --install

# Install CMake (if not already installed)
brew install cmake
```

### 2. Clone and build

```bash
cd ~/Desktop   # or wherever you want the project
# Copy the CrushClone folder here

cd CrushClone
cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_OSX_ARCHITECTURES="arm64"
cmake --build build --config Release -j$(sysctl -n hw.ncpu)
```

### 3. The VST3 bundle

After building, the VST3 is at:
```
build/CrushClone_artefacts/Release/VST3/CrushClone.vst3
```

CMake is configured with `COPY_PLUGIN_AFTER_BUILD=TRUE`, so it will also
auto-copy to `/Library/Audio/Plug-Ins/VST3/`.

If auto-copy fails (permissions), manually copy:
```bash
sudo cp -R build/CrushClone_artefacts/Release/VST3/CrushClone.vst3 \
           /Library/Audio/Plug-Ins/VST3/
```

### 4. Load in FL Studio

1. Open FL Studio 25
2. Go to **Options → Manage plugins** (or the Plugin Manager)
3. Click **"Find installed plugins"** or **"Verify plugins"**
4. CrushClone should appear under **Effects**
5. Add it to any Mixer insert channel

### 5. Verify native architecture

```bash
file build/CrushClone_artefacts/Release/VST3/CrushClone.vst3/Contents/MacOS/CrushClone
# Should output: Mach-O 64-bit bundle arm64
```

## Project Structure

```
CrushClone/
├── CMakeLists.txt              # Build config (fetches JUCE automatically)
├── README.md
└── Source/
    ├── PluginProcessor.h/cpp   # Main audio processor + APVTS params
    ├── PluginEditor.h/cpp      # Dark-themed GUI
    └── DSP/
        ├── Oversampler.h/cpp   # 4× oversampling wrapper
        ├── Distortion.h/cpp    # Tube + Mech waveshapers
        ├── LadderFilter.h/cpp  # ZDF SVF resonant low-pass
        └── Compressor.h/cpp    # One-knob compressor + Phat mode
```

## Tuning Tips

- The **Tube** waveshaper bias (0.08) and gain range (1–20) can be adjusted
  in `Distortion.cpp` to taste
- The **Mech** pre-emphasis shelf frequencies (200 Hz / 4 kHz) shape its
  character — raise the high shelf for more sizzle
- The **Filter** resonance cap (0.95) prevents true self-oscillation —
  raise it to 0.98 if you want the filter to sing
- The **Phat** shelf frequency (120 Hz) and gain (2.5) control how much
  bass gets added — adjust in `Compressor.cpp`

## License

This is a personal project for educational purposes. CamelCrusher was
originally by Camel Audio (acquired by Apple in 2015). This is a clean-room
recreation of the concept, not a copy of any proprietary code.
