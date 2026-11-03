#!/bin/sh
# Removes the Furchild plugin installed by the package.
set -e
sudo rm -rf "/Library/Audio/Plug-Ins/VST3/Furchild.vst3"
sudo pkgutil --forget com.studio87.furchild.vst3 >/dev/null 2>&1 || true
echo "Removed the plugin."
