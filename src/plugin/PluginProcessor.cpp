#include "PluginProcessor.h"
#include "../editor/PluginEditor.h"

using namespace juce;

AudioProcessorValueTreeState::ParameterLayout FurchildProcessor::createLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    const int v = 1; // parameter version
    const auto flt = [] (float lo, float hi, float step) { return NormalisableRange<float> (lo, hi, step); };

    for (const char* ch : { "A", "B" })
    {
        const String c (ch), n = c == "A" ? "Channel one " : "Channel two ";
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "in" + c, v }, n + "Input", flt (0.0f, 24.0f, 1.0f), 0.0f,
                                                            AudioParameterFloatAttributes().withLabel ("dB")));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "th" + c, v }, n + "Threshold", flt (0.0f, 10.0f, 0.01f), 2.0f));
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "tc" + c, v }, n + "Time",
                                                             StringArray { "TC 1", "TC 2", "TC 3", "TC 4", "TC 5", "TC 6" }, 2));
    }
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ids::mode, v }, "Mode", StringArray { "Left / Right", "Lat / Vert" }, 0));
    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { ids::link, v }, "Link", true));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ids::out, v }, "Output", flt (-12.0f, 12.0f, 0.1f), 2.0f,
                                                        AudioParameterFloatAttributes().withLabel ("dB")));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ids::mix, v }, "Mix", flt (0.0f, 100.0f, 0.1f), 100.0f,
                                                        AudioParameterFloatAttributes().withLabel ("%")));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ids::hp, v }, "Side-chain HP", StringArray { "Off", "60 Hz", "120 Hz", "240 Hz" }, 0));
    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { ids::levelMatch, v }, "Level Match", false));
    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { ids::bypass, v }, "Bypass", false));
    return { p.begin(), p.end() };
}

FurchildProcessor::FurchildProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  AudioChannelSet::stereo(), true)
                          .withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "FURCHILD", createLayout())
{
    pIn[0] = apvts.getRawParameterValue (ids::inA); pIn[1] = apvts.getRawParameterValue (ids::inB);
    pTh[0] = apvts.getRawParameterValue (ids::thA); pTh[1] = apvts.getRawParameterValue (ids::thB);
    pTc[0] = apvts.getRawParameterValue (ids::tcA); pTc[1] = apvts.getRawParameterValue (ids::tcB);
    pMode = apvts.getRawParameterValue (ids::mode); pLink = apvts.getRawParameterValue (ids::link);
    pOut = apvts.getRawParameterValue (ids::out);   pMix = apvts.getRawParameterValue (ids::mix);
    pHp = apvts.getRawParameterValue (ids::hp);     pLevel = apvts.getRawParameterValue (ids::levelMatch);
    pBypass = apvts.getRawParameterValue (ids::bypass);
}

fur::Params FurchildProcessor::readParams() const
{
    fur::Params q;
    for (int c = 0; c < 2; ++c)
    {
        q.inDb[c] = pIn[c]->load();
        q.thKnob[c] = pTh[c]->load();
        q.tc[c] = (int) (pTc[c]->load() + 0.5f);
    }
    q.mode = (int) (pMode->load() + 0.5f);
    q.link = pLink->load() > 0.5f;
    q.outDb = pOut->load();
    q.mix = pMix->load() * 0.01;
    q.hp = (int) (pHp->load() + 0.5f);
    q.levelMatch = pLevel->load() > 0.5f;
    q.bypass = pBypass->load() > 0.5f;
    return q;
}

bool FurchildProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto out = l.getMainOutputChannelSet();
    return (out == AudioChannelSet::mono() || out == AudioChannelSet::stereo()) && out == l.getMainInputChannelSet();
}

var FurchildProcessor::getSetting (const char* key, const var& fallback) const
{
    return apvts.state.getProperty (key, fallback);
}

void FurchildProcessor::setSetting (const char* key, const var& v)
{
    apvts.state.setProperty (key, v, nullptr);
}

void FurchildProcessor::prepareToPlay (double sampleRate, int)
{
    core.setParams (readParams());
    core.prepare (sampleRate);        // the 2x oversampled stages report their delay below
    core.setParams (readParams());
    setLatencySamples (core.latencySamples());
}

void FurchildProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;

    const int numCh = jmin (2, buffer.getNumChannels());
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());
    if (numCh == 0 || buffer.getNumSamples() == 0) return;

    core.setParams (readParams());
    core.process (buffer.getWritePointer (0), numCh > 1 ? buffer.getWritePointer (1) : nullptr, buffer.getNumSamples());

    const auto pk = core.takePeaks();
    raiseTo (inPeak, (float) pk.in);
    raiseTo (outPeak, (float) pk.out);
    for (int c = 0; c < 2; ++c)
    {
        grNow[c].store ((float) core.grNow (c));
        raiseTo (grPeak[c], (float) pk.gr[c]);
        raiseTo (ocPeak[c], (float) pk.oc[c]);
    }
    matchGain.store ((float) core.levelMatchGain());
}

AudioProcessorEditor* FurchildProcessor::createEditor()
{
    return new FurchildEditor (*this);
}

void FurchildProcessor::getStateInformation (MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty ("stateVersion", 1, nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void FurchildProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (ValueTree::fromXml (*xml)); // unknown keys are ignored by APVTS
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new FurchildProcessor(); }
