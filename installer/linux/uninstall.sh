#!/bin/sh
# Removes the Furchild plugin. Your presets are kept unless you say yes.
set -e
dest="${VST3_DIR:-$HOME/.vst3}"
rm -rf "$dest/Furchild.vst3"
echo "Removed: $dest/Furchild.vst3"
data="${XDG_DATA_HOME:-$HOME/.local/share}/Studio 87/Furchild"
if [ -d "$data" ]; then
  printf "Also delete your presets in %s ? [y/N] " "$data"
  read -r answer || answer=n
  case "$answer" in
    y|Y|yes|YES) rm -rf "$data"; echo "Presets deleted." ;;
    *) echo "Presets kept." ;;
  esac
fi
