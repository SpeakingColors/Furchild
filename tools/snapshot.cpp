<<<<<<< HEAD
// Renders the real editor offscreen to a PNG (screenshots, manual tour) and runs plugin self tests that need the JUCE classes.
// usage: FurSnapshot out.png [scale] [look A|B|C] [dialog none|settings|guide|about|presets|save|export|import|hp|tip] [frames] [preset name] [meter 0|1]
//        FurSnapshot --selftest [fixture.xml]
//        FurSnapshot --write-fixture out.xml
#include "../src/plugin/PluginProcessor.h"
#include "../src/editor/PluginEditor.h"
#include "../src/editor/Presets.h"

static void setFixtureValues (FurchildProcessor& a)
{
    auto set = [&a] (const char* id, float real) { auto* p = a.apvts.getParameter (id); p->setValueNotifyingHost (p->convertTo0to1 (real)); };
    set ("inA", 7.0f); set ("inB", 3.0f); set ("thA", 4.5f); set ("thB", 1.5f); set ("tcA", 5); set ("tcB", 1);
    set ("mode", 1); set ("link", 0); set ("out", -3.5f); set ("mix", 40.0f); set ("hp", 3); set ("levelMatch", 1);
    a.setSetting (setting::look, "B"); a.setSetting (setting::meter, 1); a.setSetting (setting::preset, "Mid Side Master");
}

// Saved state of state version 1 must keep loading (tests/fixtures/state-v1.xml).
static int fixtureTest (const juce::File& f)
{
    int bad = 0;
    auto check = [&] (bool ok, const char* what) { std::printf ("%s: %s\n", ok ? "ok  " : "FAIL", what); if (! ok) ++bad; };
    FurchildProcessor p;
    auto xml = juce::XmlDocument::parse (f);
    check (xml != nullptr, "fixture parses");
    if (xml == nullptr) return 1;
    juce::MemoryBlock mb;
    juce::AudioProcessor::copyXmlToBinary (*xml, mb);
    p.setStateInformation (mb.getData(), (int) mb.getSize());
    auto real = [&p] (const char* id) { return p.apvts.getRawParameterValue (id)->load(); };
    check (std::abs (real ("inA") - 7.0f) < 0.01f && std::abs (real ("inB") - 3.0f) < 0.01f && std::abs (real ("thA") - 4.5f) < 0.01f
               && std::abs (real ("thB") - 1.5f) < 0.01f && (int) real ("tcA") == 5 && (int) real ("tcB") == 1, "channel values from the v1 fixture");
    check ((int) real ("mode") == 1 && real ("link") < 0.5f && std::abs (real ("out") + 3.5f) < 0.06f && std::abs (real ("mix") - 40.0f) < 0.1f
               && (int) real ("hp") == 3 && real ("levelMatch") > 0.5f, "mode, link, output, mix, side-chain HP and Level Match from the v1 fixture");
    check (p.getSetting (setting::look, "C").toString() == "B" && (int) p.getSetting (setting::meter, 0) == 1
               && p.getSetting (setting::preset, "").toString() == "Mid Side Master", "settings from the v1 fixture");
    return bad;
}

static int selfTest (const juce::File& fixture)
{
    int bad = 0;
    auto check = [&] (bool ok, const char* what) { std::printf ("%s: %s\n", ok ? "ok  " : "FAIL", what); if (! ok) ++bad; };
    FurchildProcessor a, b;
    auto set = [] (FurchildProcessor& p, const char* id, float real)
    {
        auto* prm = p.apvts.getParameter (id);
        prm->setValueNotifyingHost (prm->convertTo0to1 (real));
    };
    auto real = [] (FurchildProcessor& p, const char* id) { return p.apvts.getRawParameterValue (id)->load(); };
    // defaults are the mockup's defaults
    check (real (a, "inA") == 0.0f && real (a, "inB") == 0.0f && std::abs (real (a, "thA") - 2.0f) < 1.0e-3f && (int) real (a, "tcA") == 2
               && std::abs (real (a, "out") - 2.0f) < 1.0e-3f && std::abs (real (a, "mix") - 100.0f) < 1.0e-3f
               && (int) real (a, "mode") == 0 && real (a, "link") > 0.5f && (int) real (a, "hp") == 0 && real (a, "bypass") < 0.5f && real (a, "levelMatch") < 0.5f,
          "defaults match the mockup");
    bool all = true;
    for (int i = 0; i < 6; ++i) { set (a, "tcA", (float) i); set (a, "tcB", (float) i); all = all && (int) real (a, "tcA") == i && (int) real (a, "tcB") == i; }
    check (all, "all 6 Time items, last item included");
    all = true;
    for (int i = 0; i < 4; ++i) { set (a, "hp", (float) i); all = all && (int) real (a, "hp") == i; }
    check (all, "all 4 side-chain HP items, last item included");
    all = true;
    for (int i = 0; i < 2; ++i) { set (a, "mode", (float) i); all = all && (int) real (a, "mode") == i; }
    check (all, "both modes");
    set (a, "inA", 24.0f); check (std::abs (real (a, "inA") - 24.0f) < 1.0e-3f, "Input reaches +24 dB");
    set (a, "out", -12.0f); check (std::abs (real (a, "out") + 12.0f) < 1.0e-3f, "Output reaches -12 dB");
    set (a, "out", 12.0f); check (std::abs (real (a, "out") - 12.0f) < 1.0e-3f, "Output reaches +12 dB");
    // state round trip including plugin settings
    setFixtureValues (a);
    juce::MemoryBlock mb; a.getStateInformation (mb);
    b.setStateInformation (mb.getData(), (int) mb.getSize());
    check (std::abs (real (b, "inA") - 7.0f) < 0.01f && (int) real (b, "tcA") == 5 && (int) real (b, "mode") == 1 && (int) real (b, "hp") == 3
               && std::abs (real (b, "mix") - 40.0f) < 0.1f && real (b, "link") < 0.5f, "parameters survive a state round trip");
    check (b.getSetting (setting::look, "C").toString() == "B" && (int) b.getSetting (setting::meter, 0) == 1
               && b.getSetting (setting::preset, "").toString() == "Mid Side Master", "settings survive a state round trip");
    // every factory preset is valid and inside the parameter ranges
    {
        bool ok = ! presets::factory().empty();
        for (auto& p : presets::factory())
            for (int i = 0; i < presets::kCount; ++i) ok = ok && p.v[i] >= presets::lo[i] && p.v[i] <= presets::hi[i];
        check (ok, "factory presets are inside the parameter ranges");
        check (presets::factory().size() == 16, "16 factory presets");
    }
    // the processor makes sound and reports its latency
    {
        FurchildProcessor c;
        c.prepareToPlay (48000.0, 512);
        check (c.getLatencySamples() == 16, "reported latency is 16 samples at 48 kHz");
        juce::AudioBuffer<float> buf (2, 512); juce::MidiBuffer midi;
        for (int i = 0; i < 512; ++i) { buf.setSample (0, i, 0.5f * std::sin (0.05f * (float) i)); buf.setSample (1, i, 0.5f * std::sin (0.05f * (float) i)); }
        for (int k = 0; k < 20; ++k) c.processBlock (buf, midi);
        bool finite = true; for (int i = 0; i < 512; ++i) finite = finite && std::isfinite (buf.getSample (0, i)) && std::isfinite (buf.getSample (1, i));
        check (finite, "processing stays finite");
        juce::AudioBuffer<float> mono (1, 256);
        mono.clear();
        c.processBlock (mono, midi);
        check (true, "a mono block is processed");
    }
    // an editor can be made, resized and deleted
    {
        std::unique_ptr<juce::AudioProcessorEditor> ed (b.createEditor());
        ed->setSize (600, 287);
        ed->setSize (1800, 861);
        check (ed->getWidth() == 1800, "editor creates and resizes");
    }
    if (fixture != juce::File()) bad += fixtureTest (fixture);
    std::printf (bad == 0 ? "ALL SELF TESTS PASSED\n" : "SELF TESTS FAILED\n");
    return bad;
}
=======
// Renders the real editor offscreen to a PNG so it can be compared with the mockup.
// usage: FurchildSnapshot out.png [unused] [scale] [look 0|1|2] [frames]
#include "../src/plugin/PluginProcessor.h"
#include "../src/editor/PluginEditor.h"
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;
<<<<<<< HEAD
    if (argc > 2 && juce::String (argv[1]) == "--write-fixture")
    {
        FurchildProcessor p; setFixtureValues (p);
        auto state = p.apvts.copyState(); state.setProperty ("stateVersion", 1, nullptr);
        if (auto xml = state.createXml()) juce::File (argv[2]).replaceWithText (xml->toString());
        return 0;
    }
    if (argc > 1 && juce::String (argv[1]) == "--selftest")
        return selfTest (argc > 2 ? juce::File (argv[2]) : juce::File()) == 0 ? 0 : 1;
    FurchildProcessor proc;
    proc.prepareToPlay (48000.0, 800);
    if (argc > 3) proc.setSetting (setting::look, juce::String (argv[3]));
    if (argc > 7) proc.setSetting (setting::meter, juce::String (argv[7]).getIntValue());
    // optional preset by name
    if (argc > 6 && juce::String (argv[6]).isNotEmpty())
        for (auto& pr : presets::factory())
            if (pr.name == juce::String (argv[6]))
            {
                for (int i = 0; i < presets::kCount; ++i)
                    if (i != presets::kBypass)
                        if (auto* prm = proc.apvts.getParameter (presets::keys[i])) prm->setValueNotifyingHost (prm->convertTo0to1 ((float) pr.v[i]));
                proc.setSetting (setting::preset, pr.name);
            }
    std::unique_ptr<juce::AudioProcessorEditor> ed (proc.createEditor());
    const double sc = argc > 2 ? juce::String (argv[2]).getDoubleValue() : 1.0;
    ed->setSize (juce::roundToInt (1200 * sc), juce::roundToInt (574 * sc));
    const juce::String dialog = argc > 4 ? juce::String (argv[4]) : juce::String ("none");
    const int frames = argc > 5 ? juce::String (argv[5]).getIntValue() : 60;
    // Feed a drum, bass and pad loop in real time so the meters, curve dot, tubes and history show something.
    juce::AudioBuffer<float> buf (2, 800);
    juce::MidiBuffer midi;
=======
    if (argc < 2) return 1;
    FurchildProcessor proc;
    proc.setPlayConfigDetails (2, 2, 48000.0, 512);
    proc.prepareToPlay (48000.0, 512);
    if (argc > 4) proc.setSetting (setting::look, juce::String (argv[4]).getIntValue());
    auto setp = [&] (const char* id, float v) { if (auto* pr = proc.apvts.getParameter (id)) pr->setValueNotifyingHost (pr->convertTo0to1 (v)); };
    setp ("out", 2.0f);
    std::unique_ptr<juce::AudioProcessorEditor> ed (proc.createEditor());
    const double sc = argc > 3 ? juce::String (argv[3]).getDoubleValue() : 1.0;
    ed->setSize (juce::roundToInt (1200 * sc), juce::roundToInt (574 * sc));
    const int frames = argc > 5 ? juce::String (argv[5]).getIntValue() : 60;
    juce::AudioBuffer<float> buf (2, 512);
    juce::MidiBuffer midi;
    proc.prepareToPlay (48000.0, 512);
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))
    juce::Random rnd (7);
    long pos = 0;
    for (int f = 0; f < frames; ++f)
    {
<<<<<<< HEAD
        for (int i = 0; i < 800; ++i, ++pos)
        {
            const double t = (double) pos / 48000.0, tk = std::fmod (t, 0.5), th = std::fmod (t + 0.25, 0.5);
            float v = (float) (0.9 * std::exp (-tk * 12.0) * std::sin (6.2831853 * (52.0 + 90.0 * std::exp (-tk * 40.0)) * tk));
            v += (float) (0.22 * std::exp (-th * 45.0) * (rnd.nextFloat() * 2.0f - 1.0f));
            v += 0.16f * (float) std::sin (6.2831853 * 110.0 * t) * (float) (0.6 + 0.4 * std::sin (6.2831853 * 0.25 * t));
=======
        for (int i = 0; i < 512; ++i, ++pos)
        {
            const double t = (double) pos / 48000.0, tk = std::fmod (t, 0.5), th = std::fmod (t + 0.25, 0.5);
            float v = (float) (0.85 * std::exp (-tk * 14.0) * std::sin (6.2831853 * (52.0 + 90.0 * std::exp (-tk * 40.0)) * tk));
            v += (float) (0.22 * std::exp (-th * 45.0) * (rnd.nextFloat() * 2.0f - 1.0f));
            v += 0.12f * (float) std::sin (6.2831853 * 220.0 * t);
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))
            buf.setSample (0, i, v); buf.setSample (1, i, v * 0.97f);
        }
        proc.processBlock (buf, midi);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (16);
    }
<<<<<<< HEAD
    if (dialog != "none") { if (auto* pe = dynamic_cast<FurchildEditor*> (ed.get())) pe->openDialogForTesting (dialog); }
=======
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))
    juce::MessageManager::getInstance()->runDispatchLoopUntil (150);
    juce::Image img = ed->createComponentSnapshot (ed->getLocalBounds(), false, 1.0f);
    juce::File out (argv[1]);
    out.deleteFile();
    juce::FileOutputStream os (out);
    juce::PNGImageFormat().writeImageToStream (img, os);
    ed.reset();
    return 0;
}
