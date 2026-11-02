#!/bin/sh
# Build and run the core tests (no JUCE needed). Add SAN=1 to run with the address and undefined behaviour sanitizers.
set -e
cd "$(dirname "$0")/.."
FLAGS="-std=c++17 -O2 -Wall -Wextra"
[ -n "$SAN" ] && FLAGS="$FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer"
g++ $FLAGS -o /tmp/furchild_test tests/test_core.cpp
/tmp/furchild_test
