#pragma once
<<<<<<< HEAD
#include <juce_audio_processors/juce_audio_processors.h>
#include "../plugin/PluginProcessor.h"

class Face;

// Plugin window. The faceplate is a fixed 1200 x 574 drawing that is scaled to the window size;
// the window keeps its proportions and can be dragged from any edge or corner.
class FurchildEditor : public juce::AudioProcessorEditor
=======
#include "../plugin/PluginProcessor.h"

// Native port of the approved mockup window (the mockup IS the plugin). The preview rig of the mockup
// is not part of the plugin, so the window is the 1200 x 640 stage minus the 66 px rig strip.
class FurchildEditor : public juce::AudioProcessorEditor, private juce::Timer
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))
{
public:
    explicit FurchildEditor (FurchildProcessor&);
    ~FurchildEditor() override;
<<<<<<< HEAD
    void resized() override;
    void resetSize();
    void openDialogForTesting (const juce::String& name); // used by tools/snapshot.cpp

    static constexpr int kW = 1200, kH = 574;

private:
    FurchildProcessor& proc;
    std::unique_ptr<Face> face;
    juce::ComponentBoundsConstrainer constrainer;
    std::unique_ptr<juce::ResizableBorderComponent> border;
=======

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    bool keyPressed (const juce::KeyPress&) override;

    struct Impl;
private:
    void timerCallback() override;
    std::unique_ptr<Impl> d;
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FurchildEditor)
};
