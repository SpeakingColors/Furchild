#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

// Presets use the faceplate units of the mockup: Input dB, Threshold 0..10, Time 0..5 (TC 1 to 6), Output dB, Mix 0..100,
// switches 0/1. Unknown keys are ignored, out of range values refuse the preset. Bypass is stored for compatibility with
// the mockup's text but is never applied: loading a preset does not touch Bypass.
struct Preset
{
    juce::String name, type = "Bus";
    juce::StringArray tags;
    bool factory = false;
    double v[13] { 0, 0, 2, 2, 2, 2, 0, 1, 2, 100, 0, 0, 0 };
};

namespace presets
{
inline constexpr int kCount = 13;
inline const char* const keys[kCount] = { "inA", "inB", "thA", "thB", "tcA", "tcB", "mode", "link", "out", "mix", "hp", "bypass", "levelMatch" };
inline const double lo[kCount] = { 0, 0, 0, 0, 0, 0, 0, 0, -12, 0, 0, 0, 0 };
inline const double hi[kCount] = { 24, 24, 10, 10, 5, 5, 1, 1, 12, 100, 3, 1, 1 };
inline const char* const types[] = { "Vocal", "Bass", "Drum", "Keys", "Lead", "Bus", "Master", "FX" };
inline constexpr int kNumTypes = 8;
inline constexpr int kBypass = 11;

const std::vector<Preset>& factory();
juce::File userFolder();

class Store
{
public:
    Store();
    void reload();                                    // read the user folder
    std::vector<Preset> all() const;                  // factory first, then yours
    const std::vector<Preset>& user() const { return mine; }
    const Preset* find (const juce::String& name) const;
    bool save (const Preset& p, const juce::String& replaceName = {}); // writes the file, replaces a same-named one
    bool remove (const juce::String& name);
    juce::String exportJson (const std::vector<Preset>& list) const;
    int importJson (const juce::String& text);        // returns count, or -1 when invalid
private:
    std::vector<Preset> mine;
};

bool valid (const juce::var& presetObject);
Preset fromVar (const juce::var& o);
juce::var toVar (const Preset& p);
} // namespace presets
