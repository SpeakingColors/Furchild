#!/bin/sh
<<<<<<< HEAD
# Build and run the core tests (no JUCE needed). Add SAN=1 to run with the address and undefined behaviour sanitizers.
set -e
cd "$(dirname "$0")/.."
FLAGS="-std=c++17 -O2 -Wall -Wextra"
[ -n "$SAN" ] && FLAGS="$FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer"
g++ $FLAGS -o /tmp/furchild_test tests/test_core.cpp
/tmp/furchild_test
=======
# Build and run the core tests (no JUCE needed), plain and with sanitizers.
set -e
cd "$(dirname "$0")/.."
g++ -std=c++17 -O2 -Wall -Wextra -o /tmp/fc_test tests/test_core.cpp
/tmp/fc_test
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=undefined -o /tmp/fc_test_san tests/test_core.cpp
/tmp/fc_test_san quick
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))
