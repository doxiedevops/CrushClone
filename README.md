# CrushClone

**A faithful recreation of the legendary CamelCrusher multi-effect plugin — native Apple Silicon VST3 and AU, built with JUCE.**

<p align="center">
  <a href="https://github.com/doxiedevops/CrushClone/releases/latest"><img alt="Latest Release" src="https://img.shields.io/github/v/release/doxiedevops/CrushClone?style=for-the-badge&color=ff6b35"></a>
  <a href="https://github.com/doxiedevops/CrushClone/releases/latest"><img alt="Downloads" src="https://img.shields.io/github/downloads/doxiedevops/CrushClone/total?style=for-the-badge&color=4ecdc4"></a>
  <a href="LICENSE"><img alt="License: MIT" src="https://img.shields.io/badge/license-MIT-blue?style=for-the-badge"></a>
</p>

---

## 🎚️ Download & Install

> **Just want to use the plugin?** You're in the right place. No terminal, no build tools — just download the installer for your OS and run it.

### ⬇️ [**Download the latest version →**](https://github.com/doxiedevops/CrushClone/releases/latest)

| Platform | File | Formats | Size |
|----------|------|---------|------|
| 🍎 **macOS** (Universal — M1/M2/M3/M4 + Intel) | `CrushClone-v1.0.0-macOS.pkg` | VST3 · AU | ~7 MB |
| 🪟 **Windows** 10/11 (64-bit) | `CrushClone-v1.0.0-Windows.exe` | VST3 | ~2 MB |

---

### 🍎 macOS installation

1. Download **`CrushClone-v1.0.0-macOS.pkg`** from the [releases page](https://github.com/doxiedevops/CrushClone/releases/latest)
2. Double-click the `.pkg` file
3. If macOS says *"CrushClone can't be opened because it is from an unidentified developer"*:
   - Open **System Settings → Privacy & Security**
   - Scroll down to the security message and click **"Open Anyway"**
   - Re-run the installer
4. Follow the installer wizard — it installs to:
   - **VST3:** `/Library/Audio/Plug-Ins/VST3/`
   - **AU:** `/Library/Audio/Plug-Ins/Components/`
5. Open your DAW and rescan plugins

### 🪟 Windows installation

1. Download **`CrushClone-v1.0.0-Windows.exe`** from the [releases page](https://github.com/doxiedevops/CrushClone/releases/latest)
2. Right-click the `.exe` → **Run as administrator**
3. If Windows SmartScreen warns *"Windows protected your PC"*:
   - Click **"More info"**
   - Click **"Run anyway"**
4. Follow the installer — it installs to:
   - **VST3:** `C:\Program Files\Common Files\VST3\`
5. Open your DAW and rescan plugins

### 🎛️ Finding CrushClone in your DAW

After installation, rescan your VST3/AU plugins:

| DAW | How to rescan |
|-----|---------------|
| **FL Studio** | Options → Manage plugins → **Find installed plugins** |
| **Ableton Live** | Preferences → Plug-Ins → **Rescan** |
| **Logic Pro** | Automatic on next launch (AU validation) |
| **Studio One** | Options → Locations → VST Plug-Ins → **Reset** |
| **Reaper** | Preferences → Plug-ins → VST → **Re-scan** |
| **Cubase** | Studio → VST Plug-in Manager → **Update** |

CrushClone will appear under **Effects** — typically in the **Distortion** or **Dynamics** category.

---

## ✨ Features

- **Tube Saturation** — asymmetric tanh waveshaper with even-harmonic bias, 4× oversampled
- **Mech Distortion** — hard clipper with low/high shelf pre-emphasis, 4× oversampled
- **Resonant Low-Pass Filter** — zero-delay feedback SVF with nonlinear resonance
- **One-Knob Compressor** — maps a single control to threshold, ratio, attack, release, and makeup gain
- **Phat Mode** — parallel compression + low-shelf boost for bass thickness
- **Master Volume** + **Dry/Wet Mix**
- Full automation via APVTS — every parameter is automatable
- State recall — presets save and restore with your project

### Signal chain

```
Audio In → Distortion (Tube + Mech) → Low-Pass Filter → Compressor (+ Phat) → Master → Audio Out
```

---

## 🐛 Troubleshooting

**macOS: "The application can't be opened" / Gatekeeper blocks the installer**
CrushClone isn't notarized with Apple yet. Override Gatekeeper once via **System Settings → Privacy & Security → Open Anyway**.

**Windows: SmartScreen blocks the installer**
CrushClone isn't code-signed with a commercial certificate yet. Click **"More info" → "Run anyway"**.

**Plugin doesn't appear in my DAW**
Rescan your plugin folders (see the table above). If CrushClone still doesn't show up, verify the files exist at the install paths listed above.

**Issues or feature requests?**
[Open an issue](https://github.com/doxiedevops/CrushClone/issues) — include your DAW, OS version, and a description of what you're hearing or seeing.

---

<br>

# 🛠️ For Developers

*Everything below is for building CrushClone from source. If you just want to use the plugin, stop here — the installers above are all you need.*

## Requirements

| Tool | Version |
|------|---------|
| macOS | 13+ (Ventura) or Windows 10+ |
| Xcode (macOS) | 15+ |
| Visual Studio (Windows) | 2022 with C++ workload |
| CMake | 3.22+ |
| Processor | Apple Silicon (M1/M2/M3/M4) or Intel / x64 |

## Build instructions

### 1. Install prerequisites

**macOS:**
```bash
xcode-select --install
brew install cmake
```

**Windows:**
Install Visual Studio 2022 with the C++ workload, and [CMake](https://cmake.org/download/).

### 2. Clone and build

```bash
git clone https://github.com/doxiedevops/CrushClone.git
cd CrushClone

cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build --config Release --parallel
```

### 3. Locate the built plugin

After building, the VST3 bundle is at:

```
build/CrushClone_artefacts/Release/VST3/CrushClone.vst3
```

On macOS, the AU component is at:

```
build/CrushClone_artefacts/Release/AU/CrushClone.component
```

CMake is configured with `COPY_PLUGIN_AFTER_BUILD=TRUE`, so it attempts to auto-copy to your system plugin folder. If permissions block this, copy manually:

```bash
# macOS
sudo cp -R build/CrushClone_artefacts/Release/VST3/CrushClone.vst3 \
           /Library/Audio/Plug-Ins/VST3/
```

### 4. Verify native architecture (macOS)

```bash
file build/CrushClone_artefacts/Release/VST3/CrushClone.vst3/Contents/MacOS/CrushClone
# Should output: Mach-O 64-bit bundle arm64 (or universal binary)
```

## Project structure

```
CrushClone/
├── CMakeLists.txt              # Build config (fetches JUCE automatically)
├── README.md
├── .github/
│   ├── workflows/
│   │   └── build-and-release.yml  # CI/CD pipeline
│   └── scripts/
│       ├── create-pkg.sh          # macOS .pkg builder
│       └── installer.nsi          # Windows NSIS installer script
└── Source/
    ├── PluginProcessor.h/cpp      # Main audio processor + APVTS params
    ├── PluginEditor.h/cpp         # Dark-themed GUI
    └── DSP/
        ├── Oversampler.h/cpp      # 4× oversampling wrapper
        ├── Distortion.h/cpp       # Tube + Mech waveshapers
        ├── LadderFilter.h/cpp     # ZDF SVF resonant low-pass
        └── Compressor.h/cpp       # One-knob compressor + Phat mode
```

## Tuning tips

- The **Tube** waveshaper bias (0.08) and gain range (1–20) can be adjusted in `Distortion.cpp`
- The **Mech** pre-emphasis shelf frequencies (200 Hz / 4 kHz) shape its character — raise the high shelf for more sizzle
- The **Filter** resonance cap (0.95) prevents true self-oscillation — raise it to 0.98 if you want the filter to sing
- The **Phat** shelf frequency (120 Hz) and gain (2.5) control how much bass gets added — adjust in `Compressor.cpp`

## Releasing a new version

The CI pipeline builds and publishes automatically when you push a semver tag:

```bash
git tag v1.1.0
git push origin v1.1.0
```

Both macOS and Windows installers will be built in parallel and posted to the [Releases](https://github.com/doxiedevops/CrushClone/releases) page.

---

## 📄 License

Released under the [MIT License](LICENSE).

CamelCrusher was originally by Camel Audio (acquired by Apple in 2015). CrushClone is a clean-room recreation of the concept, not a copy of any proprietary code.
