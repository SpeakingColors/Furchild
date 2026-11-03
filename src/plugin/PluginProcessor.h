#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
<<<<<<< HEAD
#include "../core/Furchild.h"
#include <atomic>

// Stable parameter IDs. Never rename or reuse these (saved projects and automation use them).
namespace ids
{
    inline constexpr const char* inA = "inA";
    inline constexpr const char* inB = "inB";
    inline constexpr const char* thA = "thA";
    inline constexpr const char* thB = "thB";
    inline constexpr const char* tcA = "tcA";
    inline constexpr const char* tcB = "tcB";
    inline constexpr const char* mode = "mode";
    inline constexpr const char* link = "link";
    inline constexpr const char* out = "out";
    inline constexpr const char* mix = "mix";
    inline constexpr const char* hp = "hp";
    inline constexpr const char* levelMatch = "levelMatch";
    inline constexpr const char* bypass = "bypass";
}

// Plugin-wide settings kept in the saved state (not presets, not automatable).
namespace setting
{
    inline constexpr const char* look = "look";       // "A", "B", "C"
    inline constexpr const char* tips = "tips";       // info bubbles
    inline constexpr const char* power = "power";     // power saving redraw
    inline constexpr const char* scale = "scale";     // window scale, 1.0 = 100 %
    inline constexpr const char* preset = "preset";   // name of the loaded preset
    inline constexpr const char* meter = "meter";     // needle shows 0 gain reduction, 1 output level
=======
#include "../core/FurchildCore.h"
#include "ParamTable.h"
#include <atomic>
#include <cstring>

// Parameter ids are the ones of the approved mockup (inA, thA, tcA, mode, link, out, mix, hp, bypass, levelMatch). Never rename them.
namespace setting
{
    inline constexpr const char* look = "look";       // 0 = A, 1 = B, 2 = C
    inline constexpr const char* tips = "tips";       // info bubbles
    inline constexpr const char* power = "power";     // power saving redraw
    inline constexpr const char* meter = "meter";     // needle shows 0 gain reduction, 1 level
    inline constexpr const char* preset = "preset";   // name of the loaded preset
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))
}

class FurchildProcessor : public juce::AudioProcessor
{
public:
    FurchildProcessor();
    ~FurchildProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    using juce::AudioProcessor::processBlock;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Furchild"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
<<<<<<< HEAD

    // Meter data for the editor (written on the audio thread, read by the editor timer).
    // Peaks are linear and collected since the editor last read them (exchange to 0).
    std::atomic<float> inPeak { 0.0f }, outPeak { 0.0f };
    std::atomic<float> grNow[2] { { 0.0f }, { 0.0f } };    // dB, positive
    std::atomic<float> grPeak[2] { { 0.0f }, { 0.0f } };   // largest gain reduction since the last read
    std::atomic<float> ocPeak[2] { { 0.0f }, { 0.0f } };   // output level of each channel before the ceiling (linear peak)
    std::atomic<float> matchGain { 1.0f };                 // Level Match gain, linear

    // Settings stored in the state tree.
    juce::var getSetting (const char* key, const juce::var& fallback) const;
    void setSetting (const char* key, const juce::var& v);
    static void raiseTo (std::atomic<float>& a, float v) { float c = a.load(); while (v > c && ! a.compare_exchange_weak (c, v)) {} }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    fur::Params readParams() const;

    fur::Compressor core;
    std::atomic<float>* pIn[2]; std::atomic<float>* pTh[2]; std::atomic<float>* pTc[2];
    std::atomic<float>* pMode; std::atomic<float>* pLink; std::atomic<float>* pOut; std::atomic<float>* pMix;
    std::atomic<float>* pHp; std::atomic<float>* pLevel; std::atomic<float>* pBypass;
=======
    fc::Engine engine;      // the editor reads its meters (plain floats, benign races)

    juce::var getSetting (const char* key, const juce::var& fallback) const;
    void setSetting (const char* key, const juce::var& v);

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    std::array<std::atomic<float>*, fcp::kNumParams> raw {};
    float vals[fcp::kNumParams] {}, lastVals[fcp::kNumParams] {};
    bool haveLast = false;
    bool lastLM = false;
    void apply();
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FurchildProcessor)
};
