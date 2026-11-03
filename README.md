# Furchild

A free, open source, cat-themed take on a classic stereo variable-mu tube limiting amplifier (VST3 for Linux, Windows and macOS).

<<<<<<< HEAD
<p align="center"><img src="docs/screenshot.png" width="900" alt="The Furchild plugin window"></p>
<p align="center"><sub>The real plugin window, rendered by the plugin itself.</sub></p>

## Status
Pre-release test build (v0.1.0). The plugin is built from the approved mockup. CI builds it on Linux, Windows and macOS and runs the DSP tests and pluginval (strictness 5) on Linux. Not yet checked: real Windows and macOS hosts, a real Flatpak DAW, and listening tests against the hardware it is inspired by. See [ROADMAP.md](ROADMAP.md).
=======
<p align="center"><img src="docs/screenshot.png" width="900" alt="Furchild window"></p>
<p align="center"><sub>The real plugin window.</sub></p>

## Status
Test build (VST3 for Linux, Windows and macOS), v0.1.0. The window is the approved mockup ported natively to JUCE, and the compressor engine is the one from the mockup. It has not been checked on real machines yet, so treat it as a test. Download the zip from the [Releases](https://github.com/SpeakingColors/Furchild/releases) page: installers, uninstallers and the manual (PDF) are inside.

## Controls
Per channel: Input (0 to +24 dB in 1 dB steps), Threshold, Time (six attack and release pairs, two of them automatic). Shared: Left / Right or Lat / Vert, Link, Level Match, Needle GR or Level, Side-chain HP, Output, Mix, Bypass. Needle meters, transfer curve, six-second gain reduction history, valves, gain reduction lights, in and out meters, 16 factory presets and your own presets.

Every control has a hover info bubble.
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))

## Download
Get the single zip from the [Releases page](https://github.com/SpeakingColors/Furchild/releases). It holds a Windows installer (with uninstaller), a macOS installer (with an Uninstall app), a Linux Flatpak extension and plain .vst3 tarball, the PDF manual, INSTALL.txt, LICENSE and SHA256SUMS. Everything works offline.

<<<<<<< HEAD
## Why the installers show a warning
The installers are unsigned: code signing certificates cost money every year and this project is free. Windows may show a blue "unknown publisher" screen (More info, Run anyway). macOS may ask you to allow the package in System Settings, Privacy and Security. Check your download against SHA256SUMS if you like.

## Controls
Two channels with Input, Threshold and Time (six time constants, two automatic), Left / Right or Lat / Vert mode, Link, Level Match, side-chain high-pass, Output, Mix, Bypass, 16 factory presets plus your own, three looks, a resizable window and hover info bubbles on every control. Details are in the manual inside the zip.

## The design mockup
The original interactive mockup is kept in `docs/mockup/Furchild.html` (WebAudio preview; it works offline).
=======
## Build
`cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build` (needs the usual JUCE Linux packages). Core DSP tests: `sh tests/run.sh`.
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))

## Privacy
Collects nothing. No network access, no telemetry, no accounts.

## Support
<p align="center">
  <a href="https://ko-fi.com/studio87"><img src="https://img.shields.io/badge/Support_Studio_87-on_Ko--fi-FF5E5B?style=for-the-badge&logo=kofi&logoColor=white" alt="Support Studio 87 on Ko-fi"></a>
</p>

Free and open source, made by one person. If it helps your music, a coffee on [Ko-fi](https://ko-fi.com/studio87) keeps it coming.

## License
AGPL-3.0-or-later, see [LICENSE](LICENSE).
