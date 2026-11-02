Furchild test build (pre-release). A free, open source, cat-themed stereo tube-style limiting compressor (VST3) by Studio 87.

One zip with: Windows installer (.exe, with uninstaller), macOS installer (.pkg, with an "Uninstall Furchild" app), Linux Flatpak extension and a plain .vst3 tarball, the PDF manual (with an interface tour), INSTALL.txt, LICENSE and SHA256SUMS. Everything works offline; the plugin collects nothing and never uses the network.

Why the installers are "unsigned": code signing certificates cost money every year and this project is free. Windows may show a blue "unknown publisher" screen (More info, Run anyway); macOS may ask you to allow the package in System Settings, Privacy and Security. The SHA256SUMS file lets you check your download.

This is a test build. Measured in CI: the DSP core tests and pluginval (strictness 5) on Linux, and that all installers build. Not yet checked: real Windows and macOS hosts, a real Flatpak DAW, and listening tests against the hardware it is inspired by. See ROADMAP.md.
