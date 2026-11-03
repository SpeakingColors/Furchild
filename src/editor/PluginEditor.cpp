#include "PluginEditor.h"
#include "Widgets.h"
#include "Views.h"
#include "Dialogs.h"
#include "Presets.h"

using namespace juce;
using namespace ui;

namespace
{
// ---- the wood cheeks: same three colours and vertical grain as PurrOne ---------------------------
struct Mul32
{
    uint32_t a;
    double next()
    {
        a += 0x6D2B79F5u;
        uint32_t t = a ^ (a >> 15);
        t *= (1u | a);
        t = (t + ((t ^ (t >> 7)) * (61u | t))) ^ t;
        return (double) (t ^ (t >> 14)) / 4294967296.0;
    }
};

Image makeWood (uint32_t seed, bool edgeRight)
{
    const int w = 104, h = 1280;
    Image img (Image::ARGB, w, h, true);
    Graphics g (img);
    Mul32 r { seed };
    g.fillAll (Colour (0xff3d2210));
    for (int x = 0; x < w; ++x)
    {
        const double a = r.next();
        g.setColour (Colour::fromFloatRGBA (123 / 255.0f, 74 / 255.0f, 38 / 255.0f, (float) (0.40 * (0.4 + 0.6 * a))));
        g.fillRect (x, 0, 1, h);
        if (r.next() < 0.35)
        {
            const float al = (float) (0.75 * r.next());
            const double y0 = r.next() * h;
            const double len = h * (0.2 + 0.6 * r.next());
            g.setColour (Colour::fromFloatRGBA (90 / 255.0f, 52 / 255.0f, 25 / 255.0f, al));
            g.fillRect ((float) x, (float) y0, 1.0f, (float) len);
        }
        if (r.next() < 0.12)
        {
            g.setColour (Colours::black.withAlpha (0.35f));
            g.fillRect (x, 0, 1, h);
        }
    }
    g.setColour (Colours::black);
    g.fillRect (edgeRight ? 0 : w - 3, 0, 3, h);
    return img;
}

std::unique_ptr<Drawable> makeLogo()
{
    static const char* svg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="64" height="64" viewBox="0 0 64 64"><path d="M8 6 L24 22 H40 L56 6 L60 36 Q60 58 32 60 Q4 58 4 36 Z" fill="#FF7BB1"/><path d="M12 14 L21 23 H14 Z M52 14 L43 23 H50 Z" fill="#FBBDD0"/><ellipse cx="22" cy="36" rx="4" ry="5" fill="#16171a"/><ellipse cx="42" cy="36" rx="4" ry="5" fill="#16171a"/><path d="M29 44 H35 L32 48 Z" fill="#16171a"/><path d="M32 48 Q28 53 24 50 M32 48 Q36 53 40 50" stroke="#16171a" stroke-width="2" fill="none"/><path d="M2 40 H14 M2 46 H14 M62 40 H50 M62 46 H50" stroke="#FBBDD0" stroke-width="1.6"/></svg>)";
    if (auto xml = XmlDocument::parse (svg)) return Drawable::createFromSVG (*xml);
    return {};
}

class Cheek : public Component, public TipClient
{
public:
    Cheek (Image i, std::function<void()> reset) : img (std::move (i)), onReset (std::move (reset)) {}
    void paint (Graphics& g) override
    {
        g.setImageResamplingQuality (Graphics::highResamplingQuality);
        g.drawImage (img, getLocalBounds().toFloat());
    }
    void mouseDoubleClick (const MouseEvent&) override { if (onReset) onReset(); }
private:
    Image img;
    std::function<void()> onReset;
};

struct Delay : Timer
{
    std::function<void()> fn;
    void start (int ms) { startTimer (ms); }
    void timerCallback() override { stopTimer(); if (fn) fn(); }
};

class Bubble : public Component
{
public:
    Bubble (const Ctx& c) : ctx (c) { setInterceptsMouseClicks (false, false); setVisible (false); }
    void showAt (const String& s, Rectangle<int> target, int parentW, int parentH)
    {
        text = s;
        AttributedString as = build();
        TextLayout tl; tl.createLayout (as, 282.0f);
        const int tw = (int) std::ceil (std::min (282.0f, tl.getWidth())) + 20, th = (int) std::ceil (tl.getHeight()) + 14;
        int x = jlimit (60, std::max (60, parentW - tw - 8), target.getX());
        int y = target.getBottom() + 6;
        if (y + th > parentH - 4) y = target.getY() - th - 6;
        setBounds (x, y, tw, th);
        setVisible (true);
        toFront (false);
    }
    void paint (Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour (Colours::black); g.fillRoundedRectangle (r, 5.0f);
        g.setColour (theme::selb); g.drawRoundedRectangle (r.reduced (0.5f), 5.0f, 1.0f);
        build().draw (g, r.reduced (10.0f, 7.0f));
    }
private:
    AttributedString build() const
    {
        AttributedString as; as.setWordWrap (AttributedString::byWord); as.setLineSpacing (2.0f);
        as.append (text, theme::font (theme::FontKind::regular, 14.0f), ctx.t().ink);
        return as;
    }
    const Ctx& ctx;
    String text;
};

class Toast : public Component
{
public:
    Toast (const Ctx&) { setInterceptsMouseClicks (false, false); setVisible (false); done.fn = [this] { setVisible (false); }; }
    void say (const String& s, int parentW, int parentH)
    {
        text = s;
        const int w = (int) textWidth (theme::font (theme::FontKind::regular, 15.0f), s) + 30;
        setBounds ((parentW - w) / 2, parentH - 34 - 28, w, 28);
        setVisible (true); toFront (false); repaint();
        done.start (1600);
    }
    void paint (Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour (Colours::black); g.fillRoundedRectangle (r, 5.0f);
        g.setColour (theme::selb); g.drawRoundedRectangle (r.reduced (0.5f), 5.0f, 1.0f);
        drawText (g, text, r, theme::font (theme::FontKind::regular, 15.0f), Colours::white);
    }
private:
    String text;
    Delay done;
};


const char* const kTcTip[6] = {
    "Time constant 1: fastest. Attack 0.2 ms, release 0.3 s. Catches peaks and lets go quickly.",
    "Time constant 2: attack 0.2 ms, release 0.8 s. A good start for vocals and drums.",
    "Time constant 3: attack 0.4 ms, release 2 s. Smooth levelling for most things.",
    "Time constant 4: attack 0.8 ms, release 5 s. Slow and gentle for held notes and pads.",
    "Time constant 5: automatic. Release is about 2 s after a single peak and stretches toward 10 s when peaks keep coming.",
    "Time constant 6: automatic. Release is about 0.3 s after a single peak and stretches toward 25 s when the level stays high. Good for a master." };
const char* const kTcAtt[6] = { "0.2", "0.2", "0.4", "0.8", "0.4", "0.2" };
const char* const kTcRel[6] = { "Release 0.3 s", "Release 0.8 s", "Release 2 s", "Release 5 s", "Release 2 to 10 s", "Release 0.3 to 25 s" };

String fmtDb (double v, int d) { return String (v >= 0.0 ? "+" : "") + String (v, d) + " dB"; }

// Drop-down list drawn on the faceplate (no native menus, so it scales with the window).
class SelectButton : public Component, public TipClient
{
public:
    SelectButton (const Ctx& c) : ctx (c) { setMouseCursor (MouseCursor::PointingHandCursor); }
    std::function<void()> onClick;
    void setText (const String& s) { if (s != text) { text = s; repaint(); } }
    void paint (Graphics& g) override
    {
        auto& t = ctx.t();
        auto r = getLocalBounds().toFloat().reduced (0.5f);
        g.setColour (t.disp); g.fillRoundedRectangle (r, 4.0f);
        g.setColour (over ? t.dim : t.line); g.drawRoundedRectangle (r, 4.0f, 1.0f);
        drawText (g, text, r.withTrimmedLeft (4.0f).withTrimmedRight (16.0f), theme::font (theme::FontKind::regular, 13.0f), t.ink, Justification::centredLeft);
        Path tri;
        const float cx = r.getRight() - 9.0f, cy = r.getCentreY();
        tri.addTriangle (cx - 3.5f, cy - 2.0f, cx + 3.5f, cy - 2.0f, cx, cy + 2.5f);
        g.setColour (t.ink); g.fillPath (tri);
    }
    void mouseEnter (const MouseEvent&) override { over = true; repaint(); }
    void mouseExit (const MouseEvent&) override { over = false; repaint(); }
    void mouseUp (const MouseEvent& e) override { if (e.mouseWasClicked() && onClick) onClick(); }
private:
    const Ctx& ctx;
    String text;
    bool over = false;
};

class DropMenu : public Component
{
public:
    DropMenu (const Ctx& c) : ctx (c) { setVisible (false); }
    std::function<void (int)> onPick;
    void open (StringArray its, int selected, Rectangle<int> anchor)
    {
        items = std::move (its); sel = selected; hover = -1;
        const int w = jmax (anchor.getWidth(), 84), h = items.size() * 26 + 6;
        box = { anchor.getX(), anchor.getBottom() + 2, w, h };
        if (box.getBottom() > getHeight() - 4) box.setY (anchor.getY() - h - 2);
        setVisible (true); toFront (false); repaint();
    }
    void paint (Graphics& g) override
    {
        auto& t = ctx.t();
        auto r = box.toFloat();
        g.setColour (Colours::black.withAlpha (0.5f)); g.fillRoundedRectangle (r.translated (0, 3).expanded (3), 8.0f);
        g.setColour (t.disp); g.fillRoundedRectangle (r, 5.0f);
        g.setColour (theme::selb); g.drawRoundedRectangle (r.reduced (0.5f), 5.0f, 1.0f);
        for (int i = 0; i < items.size(); ++i)
        {
            auto ir = Rectangle<float> ((float) box.getX() + 3, (float) box.getY() + 3 + (float) i * 26, (float) box.getWidth() - 6, 26);
            if (i == hover) { g.setColour (theme::sel); g.fillRoundedRectangle (ir, 3.0f); }
            drawText (g, items[i], ir.withTrimmedLeft (8.0f), theme::font (theme::FontKind::regular, 14.0f), i == sel ? theme::lilac : t.ink, Justification::centredLeft);
        }
    }
    int indexAt (Point<int> p) const { return box.contains (p) ? (p.y - box.getY() - 3) / 26 : -1; }
    void mouseMove (const MouseEvent& e) override { const int i = indexAt (e.getPosition()); if (i != hover) { hover = i; repaint(); } }
    void mouseDown (const MouseEvent& e) override
    {
        const int i = indexAt (e.getPosition());
        setVisible (false);
        if (i >= 0 && i < items.size() && onPick) onPick (i);
    }
    bool hitTest (int, int) override { return isVisible(); }
private:
    const Ctx& ctx;
    StringArray items;
    int sel = 0, hover = -1;
    Rectangle<int> box;
};
} // namespace

// =====================================================================================================
class Face : public Component, public DialogHost, private Timer
{
public:
    Face (FurchildProcessor& p, std::function<void()> resetSize)
        : proc (p), onResetSize (std::move (resetSize)), logo (makeLogo()),
          dialA (ctx, live, 0), dialB (ctx, live, 1), curve (ctx, live), hist (ctx, live), tubes (ctx, live),
          ladA (ctx, live, 0, layout::ladderBoxA, layout::ladderA, layout::ladderNumA),
          ladB (ctx, live, 1, layout::ladderBoxB, layout::ladderB, layout::ladderNumB),
          meterIn (ctx), meterOut (ctx), hpSelect (ctx), hpMenu (ctx), bubble (ctx), toastView (ctx), modal (*this)
    {
        const String lk = proc.getSetting (setting::look, "C").toString();
        ctx.look = lk == "A" ? 'A' : lk == "B" ? 'B' : 'C';
        ctx.tips = (bool) proc.getSetting (setting::tips, true);
        ctx.power = (bool) proc.getSetting (setting::power, false);
        setSize (layout::W, layout::H);
        setWantsKeyboardFocus (true);

        addAndMakeVisible (cheekL = new Cheek (makeWood (11, true), onResetSize));
        addAndMakeVisible (cheekR = new Cheek (makeWood (29, false), onResetSize));
        cheekL->setBounds (roundToInt (layout::cheekL.x), 0, roundToInt (layout::cheekL.w), layout::H);
        cheekR->setBounds (roundToInt (layout::cheekR.x), 0, roundToInt (layout::cheekR.w), layout::H);
        cheekL->tip = cheekR->tip = "Double-click the wood to reset the window to 100%.";

        buildTexts();
        buildHeader();
        buildDisplays();
        buildMid();
        buildKnobs();
        buildMeters();

        hpMenu.setBounds (getLocalBounds());
        hpMenu.onPick = [this] (int i) { setReal (ids::hp, (float) i); syncControls(); };
        addChildComponent (hpMenu);
        addAndMakeVisible (bubble);
        addAndMakeVisible (toastView);
        addChildComponent (modal);
        addMouseListener (this, true);
        tipTimer.fn = [this] { showTip(); };
        refreshLive();
        syncControls();
        startTimerHz (ctx.power ? 16 : 60);
    }
    ~Face() override { stopTimer(); removeMouseListener (this); }

    void openForTesting (const String& n)
    {
        if (n == "settings") modal.openSettings();
        else if (n == "guide") modal.openGuide();
        else if (n == "about") modal.openAbout();
        else if (n == "presets") modal.openBrowser();
        else if (n == "save") modal.openSave (nullptr);
        else if (n == "export") modal.openExport (nullptr);
        else if (n == "import") modal.openImport();
        else if (n == "hp") hpSelect.onClick();
        else if (n == "tip") { tipHost = &meterIn; tipText = "Info bubble check."; showTip(); }
    }

    // ---- DialogHost --------------------------------------------------------------------------
    FurchildProcessor& processor() override { return proc; }
    presets::Store& store() override { return presetStore; }
    Ctx& context() override { return ctx; }
    String presetName() override { return proc.getSetting (setting::preset, presets::factory()[0].name).toString(); }
    void setPresetName (const String& n) override { proc.setSetting (setting::preset, n); syncControls(); }
    void toast (const String& s) override { toastView.say (s, getWidth(), getHeight()); }
    void resetWindow() override { if (onResetSize) onResetSize(); }
    void settingsChanged() override
    {
        startTimerHz (ctx.power ? 16 : 60);
        repaint();
        for (auto* c : getChildren()) c->repaint();
    }

    Preset currentValues() override
    {
        Preset p;
        for (int i = 0; i < presets::kCount; ++i) p.v[i] = shownReal (presets::keys[i]);
        for (int i : { 2, 3, 8, 9 }) p.v[i] = std::round (p.v[i] * 100.0) / 100.0;
        return p;
    }

    void loadPreset (const Preset& p) override
    {
        for (int i = 0; i < presets::kCount; ++i)
            if (i != presets::kBypass) setReal (presets::keys[i], (float) p.v[i]);
        setPresetName (p.name);
        toast ("Loaded " + p.name);
        refreshLive();
        syncControls();
    }

    bool dirty() override
    {
        const Preset* base = presetStore.find (presetName());
        if (base == nullptr) base = &presets::factory()[0];
        const Preset now = currentValues();
        for (int i = 0; i < presets::kCount; ++i)
        {
            if (i == presets::kBypass) continue; // bypass is not part of a preset
            if (std::abs (now.v[i] - base->v[i]) > 0.06) return true;
        }
        return false;
    }

    // ---- painting ----------------------------------------------------------------------------
    void paint (Graphics& g) override
    {
        auto& t = ctx.t();
        g.fillAll (Colours::black);
        g.setColour (t.plate);
        g.fillRect (rc (layout::body).withTop (0).withHeight ((float) layout::H));
        // header
        g.setColour (t.plate2); g.fillRect (rc (layout::hdr));
        g.setColour (t.line); g.fillRect (layout::hdr.x, layout::hdr.h - 1.0f, layout::hdr.w, 1.0f);
        if (logo) logo->drawWithin (g, rc (layout::logo), RectanglePlacement::centred, 1.0f);
        drawText (g, layout::name.s, rc (layout::name.r), theme::font (theme::FontKind::brand, layout::name.size), Colours::white, Justification::centred, layout::name.spacing);
        drawText (g, String (layout::s87.s).toUpperCase(), rc (layout::s87.r), theme::font (theme::FontKind::regular, layout::s87.size), t.dim, Justification::centred, layout::s87.spacing);
        // display strip
        g.setColour (t.plate2); g.fillRect (rc (layout::disp));
        g.setColour (t.line); g.fillRect (layout::disp.x, layout::disp.y + layout::disp.h - 1.0f, layout::disp.w, 1.0f);
        for (auto& c : layout::dispCaps)
            drawText (g, String (c.s).toUpperCase(), rc (c.r), theme::font (theme::FontKind::regular, c.size), t.title, Justification::centredLeft, c.spacing);
        drawText (g, layout::dispSubText[0], rc (layout::dispSubs[0]), theme::font (theme::FontKind::mono, 11.0f), t.dim, Justification::centredLeft);
        drawText (g, layout::dispSubText[1], rc (layout::dispSubs[1]), theme::font (theme::FontKind::mono, 11.0f), t.dim, Justification::centredRight);
        // controls section with its fine vertical grain, then the two channel groups
        g.setColour (t.plate); g.fillRect (rc (layout::knobs));
        g.setColour (Colours::white.withAlpha (0.018f));
        for (float x = layout::knobs.x; x < layout::knobs.x + layout::knobs.w; x += 3.0f) g.fillRect (x, layout::knobs.y, 1.0f, layout::knobs.h);
        for (auto* grp : { &layout::grpA, &layout::grpB })
        {
            g.setColour (Colours::black.withAlpha (0.14f)); g.fillRoundedRectangle (rc (*grp), 10.0f);
            g.setColour (t.line); g.drawRoundedRectangle (rc (*grp).reduced (0.5f), 10.0f, 1.0f);
        }
        for (int i = 0; i < 8; ++i)
        {
            const auto& c = layout::cells[i];
            drawText (g, String (layout::cellLabels[i]).toUpperCase(), rc (c.label), theme::font (theme::FontKind::bold, c.labelSize), t.title, Justification::centred, c.labelSpacing);
            if (c.hasEnds)
            {
                drawText (g, "FAST", rc (c.ends[0]), theme::font (theme::FontKind::regular, 11.0f), t.title, Justification::centred, 2.0f);
                drawText (g, "SLOW", rc (c.ends[1]), theme::font (theme::FontKind::regular, 11.0f), t.title, Justification::centred, 2.0f);
            }
        }
        drawText (g, String (layout::hpCap.s).toUpperCase(), rc (layout::hpCap.r), theme::font (theme::FontKind::regular, layout::hpCap.size), t.title, Justification::centred, layout::hpCap.spacing);
        // meter column
        g.setColour (t.plate2); g.fillRect (rc (layout::meters));
        g.setColour (t.line); g.fillRect (layout::meters.x, layout::meters.y, 1.0f, layout::meters.h);
        for (auto& c : layout::meterCaps)
            drawText (g, String (c.s).toUpperCase(), rc (c.r), theme::font (theme::FontKind::regular, c.size), t.title, Justification::centred, c.spacing);
        // footer
        g.setColour (t.plate2); g.fillRect (rc (layout::foot));
        g.setColour (t.line); g.fillRect (layout::foot.x, layout::foot.y, layout::foot.w, 1.0f);
        drawText (g, String ("Studio 87 ") + String::fromUTF8 ("\xc2\xb7") + " Furchild v" + JucePlugin_VersionString,
                  { layout::foot.x + 12.0f, layout::foot.y, 400.0f, layout::foot.h }, theme::font (theme::FontKind::regular, 12.0f), t.dim, Justification::centredLeft, 1.0f);
    }

    // ---- keyboard ----------------------------------------------------------------------------
    bool keyPressed (const KeyPress& k) override
    {
        if (modal.isOpen() || k.getModifiers().isCommandDown() || k.getModifiers().isCtrlDown() || k.getModifiers().isAltDown()) return false;
        const juce_wchar c = CharacterFunctions::toLowerCase (k.getTextCharacter());
        if (c == 'b') toggle (ids::bypass);
        else if (c == 'm') toggle (ids::mode);
        else if (c == 'l') toggleLink();
        else return false;
        return true;
    }

    // ---- info bubbles ------------------------------------------------------------------------
    void mouseMove (const MouseEvent& e) override { trackTip (e); }
    void mouseEnter (const MouseEvent& e) override { trackTip (e); }
    void mouseExit (const MouseEvent&) override { hideTip(); }
    void mouseDown (const MouseEvent& e) override
    {
        hideTip();
        if (dynamic_cast<TextEditor*> (e.eventComponent) == nullptr && ! modal.isOpen()) grabKeyboardFocus();
    }

private:
    void timerCallback() override
    {
        if (! isShowing() && getPeer() != nullptr) return;   // paused while the window is hidden (an offscreen render has no peer and keeps running)
        refreshLive();
        dialA.repaint(); dialB.repaint(); curve.repaint(); hist.repaint(); tubes.repaint(); ladA.repaint(); ladB.repaint();
        meterIn.update (live.inPk); meterOut.update (live.outPk);
        syncControls();
    }

    // ---- parameter access --------------------------------------------------------------------
    RangedAudioParameter* param (const String& id) const { return dynamic_cast<RangedAudioParameter*> (proc.apvts.getParameter (id)); }
    float real (const String& id) const { return proc.apvts.getRawParameterValue (id)->load(); }
    bool linked() const { return real (ids::link) > 0.5f; }
    // what the faceplate shows: with Link on, channel two shows channel one
    float shownReal (const String& id) const
    {
        if (linked() && id.endsWithChar ('B') && (id.startsWith ("in") || id.startsWith ("th") || id.startsWith ("tc")))
            return real (id.dropLastCharacters (1) + "A");
        return real (id);
    }
    static String mirrorOf (const String& id)
    {
        if (id.length() == 3 && (id.startsWith ("in") || id.startsWith ("th") || id.startsWith ("tc")))
            return id.dropLastCharacters (1) + (id.endsWithChar ('A') ? "B" : "A");
        return {};
    }
    void setReal (const String& id, float v)
    {
        auto* p = param (id);
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (v));
        p->endChangeGesture();
    }
    void toggle (const char* id) { setReal (id, real (id) > 0.5f ? 0.0f : 1.0f); syncControls(); }
    void toggleLink()
    {
        const bool on = ! linked();
        setReal (ids::link, on ? 1.0f : 0.0f);
        if (on) for (const char* k : { "in", "th", "tc" }) setReal (String (k) + "B", real (String (k) + "A"));
        syncControls();
    }
    void beginKnob (const String& id)
    {
        param (id)->beginChangeGesture();
        if (linked() && mirrorOf (id).isNotEmpty()) param (mirrorOf (id))->beginChangeGesture();
    }
    void setKnob (const String& id, double v)
    {
        auto* p = param (id);
        p->setValueNotifyingHost (p->convertTo0to1 ((float) v));
        if (linked() && mirrorOf (id).isNotEmpty())
        {
            auto* m = param (mirrorOf (id));
            m->setValueNotifyingHost (m->convertTo0to1 ((float) v));
        }
    }
    void endKnob (const String& id)
    {
        param (id)->endChangeGesture();
        if (linked() && mirrorOf (id).isNotEmpty()) param (mirrorOf (id))->endChangeGesture();
    }

    void refreshLive()
    {
        const double now = Time::getMillisecondCounterHiRes();
        const double dt = lastMs > 0 ? jlimit (0.001, 0.25, (now - lastMs) * 0.001) : 1.0 / 60.0;
        lastMs = now;
        const float dec = (float) std::exp (-dt / 0.1);
        live.inPk = std::max (proc.inPeak.exchange (0.0f), live.inPk * dec);
        live.outPk = std::max (proc.outPeak.exchange (0.0f), live.outPk * dec);
        live.meterMode = (int) proc.getSetting (setting::meter, 0) != 0 ? 1 : 0;
        const float k = 1.0f - std::pow (0.6f, (float) (dt * 60.0));
        for (int c = 0; c < 2; ++c)
        {
            live.grNow[c] = proc.grNow[c].load();
            live.ocPk[c] = std::max (proc.ocPeak[c].exchange (0.0f), live.ocPk[c] * dec);
            pend[c] = std::max (pend[c], proc.grPeak[c].exchange (0.0f));
            const float tgt = live.meterMode ? std::max (-30.0f, l2d (live.ocPk[c])) : live.grNow[c];
            live.needle[c] += (tgt - live.needle[c]) * k;
        }
        histAcc += dt;
        for (int guard = 0; histAcc >= 6.0 / 288.0 && guard < 300; ++guard)
        {
            histAcc -= 6.0 / 288.0;
            live.histA[live.hi] = pend[0]; live.histB[live.hi] = pend[1];
            live.hi = (live.hi + 1) % 288;
            pend[0] = live.grNow[0]; pend[1] = live.grNow[1];
        }
        live.lm = proc.matchGain.load();
        for (int c = 0; c < 2; ++c)
        {
            const String ch = c ? "B" : "A";
            live.inDb[c] = shownReal ("in" + ch); live.thKnob[c] = shownReal ("th" + ch); live.tc[c] = (int) (shownReal ("tc" + ch) + 0.5f);
        }
        live.outDb = real (ids::out); live.mix = real (ids::mix);
        live.mode = real (ids::mode) > 0.5f ? 1 : 0; live.hp = (int) (real (ids::hp) + 0.5f);
        live.link = linked(); live.bypass = real (ids::bypass) > 0.5f; live.levelMatch = real (ids::levelMatch) > 0.5f;
    }

    String dot() const { return String::fromUTF8 ("\xc2\xb7"); }

    void syncControls()
    {
        const bool lv = live.meterMode != 0;
        bBypass->setOn (live.bypass);
        mLR->setOn (live.mode == 0); mLV->setOn (live.mode == 1);
        bLink->setOn (live.link);
        bLevel->setOn (live.levelMatch);
        {
            const float lg = l2d (live.lm);
            bLevel->setText (live.levelMatch ? String ("Match ") + (lg >= 0 ? "+" : "") + String (lg, 1) : String ("Level Match"));
        }
        mtR->setOn (! lv); mtL->setOn (lv);
        hpSelect.setText (layout::hpItems[jlimit (0, 3, live.hp)]);
        const bool lat = live.mode == 1;
        ghA->setText (String ("Channel one ") + dot() + " " + (lat ? "Lat" : "Left"));
        ghB->setText (String ("Channel two ") + dot() + " " + (lat ? "Vert" : "Right"));
        capA->setText (lat ? "Lat (sum)" : "Left");
        capB->setText (lat ? "Vert (difference)" : "Right");
        for (int c = 0; c < 2; ++c)
        {
            const int tc = jlimit (0, 5, live.tc[c]);
            (c ? dsB : dsA)->setText ("TC " + String (tc + 1) + (tc >= 4 ? " auto" : ""));
            String txt;
            if (lv) txt = live.ocPk[c] > 1.0e-4f ? String (l2d (live.ocPk[c]), 1) + " dB" : String ("-inf");
            else txt = String (live.grNow[c], 1) + " dB";
            (c ? dtB : dtA)->setText ((lv ? "Out " : "GR ") + txt);
        }
        hsA->setText ("A " + String (live.grNow[0], 1)); hsB->setText ("B " + String (live.grNow[1], 1));
        pkIn->setText (live.inPk > 1.0e-4f ? String (l2d (live.inPk), 1) : String ("-inf"));
        pkOut->setText (live.outPk > 1.0e-4f ? String (l2d (live.outPk), 1) : String ("-inf"));
        if (auto* ed = findParentComponentOfClass<FurchildEditor>())
            winsz->setText (String (ed->getWidth()) + String::fromUTF8 (" \xc3\x97 ") + String (ed->getHeight()) + " px");
        // knobs and the value rows under them
        for (auto* kv : knobs) kv->refresh();
        static const char* const cellId[8] = { "inA", "thA", "tcA", "out", "mix", "inB", "thB", "tcB" };
        for (int i = 0; i < 8; ++i)
        {
            const double v = shownReal (cellId[i]);
            std::vector<KnobValue::Line> ln;
            if (i == 0 || i == 5) ln = { { fmtDb (v, 0), {} } };
            else if (i == 1 || i == 6) ln = { { String (v, 1), {} }, { {}, fmtDb (0.0 - v * 3.2, 1) + " thr" } };
            else if (i == 2 || i == 7)
            {
                const int tc = jlimit (0, 5, (int) (v + 0.5));
                ln = { { "TC " + String (tc + 1), tc >= 4 ? " auto" : " fixed" }, { {}, String ("Attack ") + kTcAtt[tc] + " ms" }, { {}, kTcRel[tc] } };
                knobs[i]->tip = String (layout::cellTips[i]) + " " + kTcTip[tc];
            }
            else if (i == 3) ln = { { fmtDb (v, 1), {} } };
            else ln = { { String (roundToInt (v)) + "%", {} } };
            values[i]->setLines (std::move (ln));
        }
        pName->setText (presetName() + (dirty() ? " *" : ""));
    }

    // ---- building ----------------------------------------------------------------------------
    PButton* addButton (const String& text, const layout::Ctl& c, float px = 14.0f, theme::FontKind k = theme::FontKind::regular)
    {
        auto* b = new PButton (ctx, text, px, k);
        owned.add (b);
        addAndMakeVisible (b);
        b->setBounds (roundToInt (c.r.x), roundToInt (c.r.y), roundToInt (c.r.w), roundToInt (c.r.h));
        b->tip = c.tip;
        return b;
    }
    TextView* addText (const layout::Rc& r, theme::FontKind kind, float px, TextView::Tone tone, Justification j, float spacing, bool upper)
    {
        auto* t = new TextView (ctx, kind, px, tone, j, spacing, upper);
        owned.add (t);
        addAndMakeVisible (t);
        // room for text that is longer than the sample text the layout was measured with
        float x = r.x - 40.0f, w = r.w + 80.0f;
        if (j.testFlags (Justification::left)) { x = r.x; w = r.w + 80.0f; }
        else if (j.testFlags (Justification::right)) { x = r.x - 80.0f; w = r.w + 80.0f; }
        t->setBounds (roundToInt (x), roundToInt (r.y), roundToInt (w), roundToInt (r.h));
        return t;
    }

    void buildTexts()
    {
        using TK = theme::FontKind; using Tn = TextView::Tone;
        ghA = addText (layout::ghA.r, TK::bold, layout::ghA.size, Tn::title, Justification::centred, layout::ghA.spacing, true);
        ghB = addText (layout::ghB.r, TK::bold, layout::ghB.size, Tn::title, Justification::centred, layout::ghB.spacing, true);
        capA = addText (layout::capA, TK::regular, 12.0f, Tn::title, Justification::centredLeft, 1.5f, true);
        capB = addText (layout::capB, TK::regular, 12.0f, Tn::title, Justification::centredLeft, 1.5f, true);
        dsA = addText (layout::dsA, TK::mono, 11.0f, Tn::dim, Justification::centredLeft, 0.0f, false);
        dtA = addText (layout::dtA, TK::mono, 11.0f, Tn::dim, Justification::centredRight, 0.0f, false);
        dsB = addText (layout::dsB, TK::mono, 11.0f, Tn::dim, Justification::centredLeft, 0.0f, false);
        dtB = addText (layout::dtB, TK::mono, 11.0f, Tn::dim, Justification::centredRight, 0.0f, false);
        hsA = addText (layout::hsA, TK::mono, 11.0f, Tn::dim, Justification::centredLeft, 0.0f, false);
        hsB = addText (layout::hsB, TK::mono, 11.0f, Tn::dim, Justification::centredRight, 0.0f, false);
        pkIn = addText (layout::pkIn.r, TK::mono, 11.0f, Tn::dim, Justification::centred, 0.0f, false);
        pkOut = addText (layout::pkOut.r, TK::mono, 11.0f, Tn::dim, Justification::centred, 0.0f, false);
        winsz = addText (layout::winsz, TK::regular, 12.0f, Tn::dim, Justification::centredRight, 1.0f, false);
        pkIn->tip = layout::pkIn.tip; pkOut->tip = layout::pkOut.tip;
        for (auto* t : { pkIn, pkOut }) t->setInterceptsMouseClicks (true, false);
    }

    void buildHeader()
    {
        pPrev = addButton (String::fromUTF8 ("\xe2\x97\x80"), layout::pPrev);
        pName = addButton (presetName(), layout::pName, 16.0f, theme::FontKind::semi);
        pNext = addButton (String::fromUTF8 ("\xe2\x96\xb6"), layout::pNext);
        pSave = addButton ("Save", layout::pSave);
        bBypass = addButton ("Bypass", layout::bBypass);
        bGuide = addButton ("Guide", layout::bGuide);
        bSettings = addButton ("Settings", layout::bSettings);
        bAbout = addButton ("About & Support", layout::bAbout);
        pPrev->onClick = [this] (const ModifierKeys&) { step (-1); };
        pNext->onClick = [this] (const ModifierKeys&) { step (1); };
        pSave->onClick = [this] (const ModifierKeys&) { modal.openSave (nullptr); };
        bBypass->onClick = [this] (const ModifierKeys&) { toggle (ids::bypass); };
        bGuide->onClick = [this] (const ModifierKeys&) { modal.openGuide(); };
        bSettings->onClick = [this] (const ModifierKeys&) { modal.openSettings(); };
        bAbout->onClick = [this] (const ModifierKeys&) { modal.openAbout(); };
        pName->onClick = [this] (const ModifierKeys&) { nameDelay.start (260); };
        nameDelay.fn = [this] { modal.openBrowser(); };
        pName->onDouble = [this]
        {
            nameDelay.stopTimer();
            const Preset* p = presetStore.find (presetName());
            if (p == nullptr || p->factory) { toast ("Save a copy first, factory presets cannot be renamed"); return; }
            Preset q = *p; modal.openSave (&q);
        };
    }

    void step (int d)
    {
        auto all = presetStore.all();
        int i = 0;
        for (size_t k = 0; k < all.size(); ++k) if (all[k].name == presetName()) i = (int) k;
        i = (i + d + (int) all.size()) % (int) all.size();
        loadPreset (all[(size_t) i]);
    }

    template <class V> void place (V& c, const layout::Rc& r) { c.setBounds (roundToInt (r.x), roundToInt (r.y), roundToInt (r.w), roundToInt (r.h)); }

    void buildDisplays()
    {
        CanvasView* views[4] = { &dialA, &dialB, &curve, &hist };
        const layout::Ctl* ctls[4] = { &layout::cvDialA, &layout::cvDialB, &layout::cvCurve, &layout::cvHist };
        for (int i = 0; i < 4; ++i) { addAndMakeVisible (*views[i]); place (*views[i], ctls[i]->r); views[i]->tip = ctls[i]->tip; }
        addAndMakeVisible (tubes); place (tubes, layout::cvTubes.r); tubes.tip = layout::cvTubes.tip;
        addAndMakeVisible (ladA); place (ladA, layout::ladderBoxA); ladA.tip = layout::ladderTip;
        addAndMakeVisible (ladB); place (ladB, layout::ladderBoxB); ladB.tip = layout::ladderTip;
    }

    void buildMid()
    {
        mLR = addButton ("Left / Right", layout::mLR, 13.0f);
        mLV = addButton ("Lat / Vert", layout::mLV, 13.0f);
        bLink = addButton ("Link", layout::bLink);
        bLevel = addButton ("Level Match", layout::bLevel);
        mtR = addButton ("Needle: GR", layout::mtR, 13.0f);
        mtL = addButton ("Needle: Level", layout::mtL, 13.0f);
        mLR->onClick = [this] (const ModifierKeys&) { setReal (ids::mode, 0.0f); syncControls(); };
        mLV->onClick = [this] (const ModifierKeys&) { setReal (ids::mode, 1.0f); syncControls(); };
        bLink->onClick = [this] (const ModifierKeys&) { toggleLink(); };
        bLevel->onClick = [this] (const ModifierKeys&) { toggle (ids::levelMatch); };
        mtR->onClick = [this] (const ModifierKeys&) { proc.setSetting (setting::meter, 0); syncControls(); };
        mtL->onClick = [this] (const ModifierKeys&) { proc.setSetting (setting::meter, 1); syncControls(); };
        addAndMakeVisible (hpSelect);
        place (hpSelect, layout::selHP.r);
        hpSelect.tip = layout::selHP.tip;
        hpSelect.onClick = [this]
        {
            StringArray its; for (auto* s : layout::hpItems) its.add (s);
            hpMenu.setBounds (getLocalBounds());
            hpMenu.open (its, jlimit (0, 3, live.hp), hpSelect.getBounds());
        };
    }

    void buildKnobs()
    {
        static const char* const cellId[8] = { "inA", "thA", "tcA", "out", "mix", "inB", "thB", "tcB" };
        for (int i = 0; i < 8; ++i)
        {
            KnobCfg cfg;
            const bool isIn = i == 0 || i == 5, isTh = i == 1 || i == 6, isTc = i == 2 || i == 7, isOut = i == 3;
            if (isIn) { cfg.lo = 0; cfg.hi = 24; cfg.def = 0; cfg.step = 1; for (int v = 0; v <= 24; v += 6) cfg.ticks.push_back ({ (double) v, String (v) }); }
            else if (isTh) { cfg.lo = 0; cfg.hi = 10; cfg.def = 2; for (int v = 0; v <= 10; v += 2) cfg.ticks.push_back ({ (double) v, String (v) }); }
            else if (isTc) { cfg.lo = 0; cfg.hi = 5; cfg.def = 2; cfg.step = 1; cfg.discrete = true; for (int v = 0; v <= 5; ++v) cfg.ticks.push_back ({ (double) v, String (v + 1) }); }
            else if (isOut) { cfg.lo = -12; cfg.hi = 12; cfg.def = 2; cfg.ticks = { { -12, "-12" }, { 0, "0" }, { 12, "+12" } }; }
            else { cfg.lo = 0; cfg.hi = 100; cfg.def = 100; cfg.ticks = { { 0, "0" }, { 50, "50" }, { 100, "100" } }; }
            const String id (cellId[i]);
            auto* k = new KnobView (cfg, [this, id] { return (double) shownReal (id); }, [this, id] { beginKnob (id); },
                                    [this, id] (double v) { setKnob (id, v); }, [this, id] { endKnob (id); });
            knobs.add (k); addAndMakeVisible (k);
            const auto& c = layout::cells[i];
            k->setBounds (roundToInt (c.knob.x), roundToInt (c.knob.y), roundToInt (c.knob.w), roundToInt (c.knob.h));
            k->tip = layout::cellTips[i];
            std::vector<float> ys;
            for (int r = 0; r < layout::valRowCount[i]; ++r) ys.push_back (layout::valRowY[i][r] - c.value.y);
            auto* v = new KnobValue (ctx, ys);
            values.add (v); addAndMakeVisible (v); place (*v, c.value);
        }
    }

    void buildMeters()
    {
        addAndMakeVisible (meterIn); addAndMakeVisible (meterOut);
        place (meterIn, layout::cvIn.r); place (meterOut, layout::cvOut.r);
        meterIn.tip = layout::cvIn.tip; meterOut.tip = layout::cvOut.tip;
    }

    // ---- tips --------------------------------------------------------------------------------
    void trackTip (const MouseEvent& e)
    {
        if (modal.isOpen() || hpMenu.isVisible() || ! ctx.tips) { hideTip(); return; }
        Component* c = e.eventComponent;
        String text; Component* host = nullptr;
        while (c != nullptr && c != this)
        {
            if (auto* tc = dynamic_cast<TipClient*> (c); tc != nullptr && tc->tip.isNotEmpty())
            {
                text = tc->tip; host = c; break;
            }
            c = c->getParentComponent();
        }
        if (host == nullptr || text.isEmpty()) { hideTip(); return; }
        if (host == tipHost && text == tipText) return;
        hideTip();
        tipHost = host; tipText = text;
        tipTimer.start (1000);
    }
    void showTip()
    {
        if (tipHost == nullptr || modal.isOpen()) return;
        bubble.showAt (tipText, getLocalArea (tipHost, tipHost->getLocalBounds()), getWidth(), getHeight());
    }
    void hideTip() { tipTimer.stopTimer(); bubble.setVisible (false); tipHost = nullptr; tipText = {}; }

    // ---- members -----------------------------------------------------------------------------
    FurchildProcessor& proc;
    std::function<void()> onResetSize;
    Ctx ctx;
    Live live;
    presets::Store presetStore;
    std::unique_ptr<Drawable> logo;
    Cheek* cheekL = nullptr; Cheek* cheekR = nullptr;
    OwnedArray<Component> owned;
    PButton *pPrev = nullptr, *pName = nullptr, *pNext = nullptr, *pSave = nullptr, *bBypass = nullptr, *bGuide = nullptr, *bSettings = nullptr, *bAbout = nullptr;
    PButton *mLR = nullptr, *mLV = nullptr, *bLink = nullptr, *bLevel = nullptr, *mtR = nullptr, *mtL = nullptr;
    TextView *ghA = nullptr, *ghB = nullptr, *capA = nullptr, *capB = nullptr, *dsA = nullptr, *dtA = nullptr, *dsB = nullptr, *dtB = nullptr,
             *hsA = nullptr, *hsB = nullptr, *pkIn = nullptr, *pkOut = nullptr, *winsz = nullptr;
    DialView dialA, dialB; CurveView curve; HistView hist; TubeView tubes;
    LadderView ladA, ladB;
    OwnedArray<KnobView> knobs;
    OwnedArray<KnobValue> values;
    MeterView meterIn, meterOut;
    SelectButton hpSelect;
    DropMenu hpMenu;
    Bubble bubble; Toast toastView;
    Modal modal;
    Delay nameDelay, tipTimer;
    Component* tipHost = nullptr;
    String tipText;
    double lastMs = 0, histAcc = 0;
    float pend[2] { 0, 0 };
};

// =====================================================================================================
FurchildEditor::FurchildEditor (FurchildProcessor& p) : AudioProcessorEditor (p), proc (p)
{
    face = std::make_unique<Face> (p, [this] { resetSize(); });
    addAndMakeVisible (*face);
    constrainer.setFixedAspectRatio ((double) kW / (double) kH);
    constrainer.setMinimumSize (480, 230);
    constrainer.setMaximumSize (1800, 861);
    setConstrainer (&constrainer);
    setResizable (true, false);
    border = std::make_unique<ResizableBorderComponent> (this, &constrainer);
    border->setBorderThickness (BorderSize<int> (5));
    addAndMakeVisible (*border);
    const double sc = jlimit (0.4, 1.5, (double) proc.getSetting (setting::scale, 1.0));
    setSize (roundToInt (kW * sc), roundToInt (kH * sc));
}

FurchildEditor::~FurchildEditor() = default;

void FurchildEditor::resized()
{
    const double s = (double) getWidth() / (double) kW;
    face->setTransform (AffineTransform::scale ((float) s));
    face->setBounds (0, 0, kW, kH);
    border->setBounds (getLocalBounds());
    proc.setSetting (setting::scale, s);
}

void FurchildEditor::resetSize() { setSize (kW, kH); }
void FurchildEditor::openDialogForTesting (const juce::String& n) { face->openForTesting (n); }
