#include "PluginEditor.h"
<<<<<<< HEAD
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

=======
#include "Theme.h"
#include "Presets.h"
#include "Tips.h"

using namespace juce;
using theme::FontKind;

namespace
{
constexpr float SW = 1200.0f, SH = 574.0f;   // stage size of the plugin window
inline float clampf (float v, float a, float b) { return jlimit (a, b, v); }
inline float l2d (float x) { return 20.0f * std::log10 (std::max (x, 1.0e-9f)); }
inline Colour rgba (int r, int g, int b, float a) { return Colour::fromFloatRGBA (r / 255.0f, g / 255.0f, b / 255.0f, a); }

String fDb (float v, int dp = 1) { return String (v >= 0.0f ? "+" : "") + String (v, dp) + " dB"; }

struct Mul32
{
    uint32_t a;
    double next() { a += 0x6D2B79F5u; uint32_t t = a ^ (a >> 15); t *= (1u | a); t = (t + ((t ^ (t >> 7)) * (61u | t))) ^ t; return (double) (t ^ (t >> 14)) / 4294967296.0; }
};
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))
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
<<<<<<< HEAD
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
=======
            const double y0 = r.next() * h, len = h * (0.2 + 0.6 * r.next());
            g.setColour (Colour::fromFloatRGBA (90 / 255.0f, 52 / 255.0f, 25 / 255.0f, al));
            g.fillRect ((float) x, (float) y0, 1.0f, (float) len);
        }
        if (r.next() < 0.12) { g.setColour (Colours::black.withAlpha (0.35f)); g.fillRect (x, 0, 1, h); }
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))
    }
    g.setColour (Colours::black);
    g.fillRect (edgeRight ? 0 : w - 3, 0, 3, h);
    return img;
}
<<<<<<< HEAD

=======
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))
std::unique_ptr<Drawable> makeLogo()
{
    static const char* svg = R"(<svg xmlns="http://www.w3.org/2000/svg" width="64" height="64" viewBox="0 0 64 64"><path d="M8 6 L24 22 H40 L56 6 L60 36 Q60 58 32 60 Q4 58 4 36 Z" fill="#FF7BB1"/><path d="M12 14 L21 23 H14 Z M52 14 L43 23 H50 Z" fill="#FBBDD0"/><ellipse cx="22" cy="36" rx="4" ry="5" fill="#16171a"/><ellipse cx="42" cy="36" rx="4" ry="5" fill="#16171a"/><path d="M29 44 H35 L32 48 Z" fill="#16171a"/><path d="M32 48 Q28 53 24 50 M32 48 Q36 53 40 50" stroke="#16171a" stroke-width="2" fill="none"/><path d="M2 40 H14 M2 46 H14 M62 40 H50 M62 46 H50" stroke="#FBBDD0" stroke-width="1.6"/></svg>)";
    if (auto xml = XmlDocument::parse (svg)) return Drawable::createFromSVG (*xml);
    return {};
}

<<<<<<< HEAD
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
=======
struct KnobCfg { const char* pid; float x, y, size; float mn, mx; bool discrete; std::vector<std::pair<float, const char*>> ticks; };
const std::vector<std::pair<float, const char*>> tickIn { { 0, "0" }, { 6, "6" }, { 12, "12" }, { 18, "18" }, { 24, "24" } };
const std::vector<std::pair<float, const char*>> tickTh { { 0, "0" }, { 2, "2" }, { 4, "4" }, { 6, "6" }, { 8, "8" }, { 10, "10" } };
const std::vector<std::pair<float, const char*>> tickTc { { 0, "1" }, { 1, "2" }, { 2, "3" }, { 3, "4" }, { 4, "5" }, { 5, "6" } };
const std::vector<std::pair<float, const char*>> tickOut { { -12, "-12" }, { 0, "0" }, { 12, "+12" } };
const std::vector<std::pair<float, const char*>> tickMix { { 0, "0" }, { 50, "50" }, { 100, "100" } };
const KnobCfg kKnobs[8] = {
    { "inA", 75, 314, 108, 0, 24, false, tickIn },  { "thA", 193, 314, 108, 0, 10, false, tickTh }, { "tcA", 311, 314, 108, 0, 5, true, tickTc },
    { "out", 471, 429, 76, -12, 12, false, tickOut }, { "mix", 561, 429, 76, 0, 100, false, tickMix },
    { "inB", 689, 314, 108, 0, 24, false, tickIn }, { "thB", 807, 314, 108, 0, 10, false, tickTh }, { "tcB", 925, 314, 108, 0, 5, true, tickTc } };

struct Btn { const char* id; float x, y, w, h; const char* text; };
const Btn kBtns[] = {
    { "pPrev", 382.64f, 10.09f, 35.34f, 24.8f, "\xe2\x97\x80" }, { "pName", 421.98f, 8.91f, 200, 27.19f, "" }, { "pNext", 625.98f, 10.09f, 35.34f, 24.8f, "\xe2\x96\xb6" },
    { "pSave", 665.33f, 10.09f, 47.08f, 24.8f, "Save" }, { "bBypass", 819.83f, 10.09f, 59.36f, 24.8f, "Bypass" }, { "bGuide", 889.19f, 10.09f, 51.8f, 24.8f, "Guide" },
    { "bSettings", 950.98f, 10.09f, 65.81f, 24.8f, "Settings" }, { "bAbout", 1026.8f, 10.09f, 109.2f, 24.8f, "About & Support" },
    { "mLR", 447, 295, 105, 23.59f, "Left / Right" }, { "mLV", 556, 295, 105, 23.59f, "Lat / Vert" }, { "bLink", 447, 323.59f, 105, 24.8f, "Link" },
    { "bLevel", 556, 323.59f, 105, 24.8f, "Level Match" }, { "mtR", 447, 353.39f, 105, 23.59f, "Needle: GR" }, { "mtL", 556, 353.39f, 105, 23.59f, "Needle: Level" } };
struct Cv { const char* id; float x, y, w, h; };
const Cv kCv[] = { { "cvDialA", 62, 70, 212, 138 }, { "cvDialB", 282, 70, 212, 138 }, { "cvCurve", 502, 70, 242, 138 }, { "cvHist", 752, 70, 290, 138 },
                   { "cvTubes", 447, 238, 214, 52 }, { "cvIn", 1062.5f, 70, 38, 300 }, { "cvOut", 1104.5f, 70, 38, 300 } };
const Rectangle<float> kSelHP (568.86f, 381.98f, 61, 24);
const Rectangle<float> kCheekL (0, 0, 52, SH), kCheekR (1148, 0, 52, SH);
const int LAD[12] = { 24, 20, 16, 12, 10, 8, 6, 5, 4, 3, 2, 1 };
constexpr int HN = 288;
} // namespace

// ================================================================================================
struct FurchildEditor::Impl
{
    FurchildEditor& ed; FurchildProcessor& proc;
    Image woodL, woodR;
    std::unique_ptr<Drawable> logo;
    int look = 2; bool tips = true;
    String hot, tipText; int tipTicks = 0; Point<float> tipPos;
    String dragId; float dragStart = 0, dragY = 0; bool dragging = false;
    bool clipIn = false, clipOut = false;
    float histA[HN] {}, histB[HN] {}; int hi = 0; float histAcc = 0;
    float needle[2] { 0, 0 };
    String toastText; int toastTicks = 0;
    String presetName;
    int rate = 30;

    Impl (FurchildEditor& e, FurchildProcessor& p) : ed (e), proc (p)
    {
        woodL = makeWood (11, true); woodR = makeWood (29, false);
        logo = makeLogo();
        look = (int) proc.getSetting (setting::look, 2);
        tips = (bool) proc.getSetting (setting::tips, true);
        presetName = proc.getSetting (setting::preset, "Start Here").toString();
    }
    const theme::Look& T() const { return theme::look (look); }

    // ---------------------------------------------------------------- parameters
    static const fcp::PDef* defOf (const String& id) { for (const auto& d : fcp::kParams) if (id == d.id) return &d; return nullptr; }
    float get (const String& id) const
    {
        if (auto* p = proc.apvts.getParameter (id)) return p->convertFrom0to1 (p->getValue());
        return 0.0f;
    }
    float getUi (const char* id, float fb) const { return (float) (double) proc.getSetting (id, fb); }
    void begin (const String& id) { if (auto* p = proc.apvts.getParameter (id)) p->beginChangeGesture(); }
    void end (const String& id) { if (auto* p = proc.apvts.getParameter (id)) p->endChangeGesture(); }
    void setRaw (const String& id, float v)
    {
        const auto* d = defOf (id); if (! d) return;
        v = clampf (v, d->mn, d->mx); if (d->isInt) v = std::round (v);
        if (auto* p = proc.apvts.getParameter (id)) p->setValueNotifyingHost (p->convertTo0to1 (v));
    }
    static String sibling (const String& id)
    {
        if (id.length() == 3 && (id.startsWith ("in") || id.startsWith ("th") || id.startsWith ("tc"))) return id.substring (0, 2) + (id[2] == 'A' ? "B" : "A");
        return {};
    }
    void set (const String& id, float v)
    {
        setRaw (id, v);
        if (get ("link") > 0.5f) { const auto s = sibling (id); if (s.isNotEmpty()) { begin (s); setRaw (s, v); end (s); } }
        ed.repaint();
    }
    void setUser (const String& id, float v) { begin (id); set (id, v); end (id); }
    static float fix (const KnobCfg& k, float v) { v = clampf (v, k.mn, k.mx); return std::string (k.pid).rfind ("in", 0) == 0 || k.discrete ? std::round (v) : v; }

    // ---------------------------------------------------------------- text helpers
    void text (Graphics& g, const String& s, FontKind k, float px, float ls, Colour c, Rectangle<float> r, Justification j) const
    {
        g.setColour (c); g.setFont (theme::font (k, px).withExtraKerningFactor (ls / px)); g.drawText (s, r, j, false);
    }
    void textBase (Graphics& g, const String& s, FontKind k, float px, Colour c, float x, float y, Justification j) const
    {
        g.setColour (c); g.setFont (theme::font (k, px)); g.drawSingleLineText (s, roundToInt (x), roundToInt (y), j);
    }

    // ---------------------------------------------------------------- paint
    void paintAll (Graphics& g)
    {
        const auto& t = T();
        g.fillAll (Colour (0xff0b0b0d));
        // cheeks
        g.drawImage (woodL, kCheekL, RectanglePlacement::stretchToFit); // stretched like the mockup canvas (104 -> 52 px wide)
        g.drawImage (woodR, kCheekR, RectanglePlacement::stretchToFit);
        // main plate
        g.setColour (t.plate); g.fillRect (52.0f, 0.0f, 1096.0f, SH);
        // knob section stripes
        g.setColour (t.plate); g.fillRect (52.0f, 230.0f, 1004.0f, 318.0f);
        g.setColour (Colours::white.withAlpha (0.018f));
        for (float x = 52; x < 1056; x += 3.0f) g.fillRect (x, 230.0f, 1.0f, 318.0f);
        // header and display strip
        g.setColour (t.plate2); g.fillRect (52.0f, 0.0f, 1096.0f, 46.0f); g.fillRect (52.0f, 46.0f, 1004.0f, 184.0f);
        g.setColour (t.plate2); g.fillRect (1056.0f, 46.0f, 92.0f, 502.0f);
        g.setColour (t.plate2); g.fillRect (52.0f, 548.0f, 1096.0f, 26.0f);
        g.setColour (t.line);
        g.fillRect (52.0f, 45.0f, 1096.0f, 1.0f);           // under the header
        g.fillRect (1056.0f, 46.0f, 1.0f, 502.0f);          // left edge of the meters
        g.fillRect (52.0f, 548.0f, 1096.0f, 1.0f);          // above the footer
        // header
        if (logo) logo->drawWithin (g, Rectangle<float> (64, 6, 34, 34), RectanglePlacement::centred, 1.0f);
        text (g, "Furchild", FontKind::serif, 27, 0.3f, Colours::white, { 106, 7, 140, 30 }, Justification::centredLeft);
        text (g, "STUDIO 87", FontKind::regular, 12, 2, t.dim, { 215.44f, 15.5f, 80, 14 }, Justification::centredLeft);
        for (const auto& b : kBtns) drawBtn (g, b);
        // display captions
        const char* caps[4] = { "LEFT", "RIGHT", "TRANSFER CURVE", "GAIN REDUCTION, LAST 6 S" };
        const float cx[4] = { 62, 282, 502, 752 };
        for (int i = 0; i < 4; ++i) text (g, caps[i], FontKind::regular, 12, 1.5f, t.title, { cx[i], 54, 200, 14 }, Justification::centredLeft);
        for (const auto& c : kCv) drawCanvas (g, c);
        // sub lines
        auto& e = proc.engine;
        const bool lvl = getUi ("meter", 0) > 0.5f;
        for (int c = 0; c < 2; ++c)
        {
            const float x0 = c ? 282.0f : 62.0f;
            const int tc = (int) std::round (get (c ? "tcB" : "tcA"));
            textBase (g, "TC " + String (tc + 1) + (fc::tcTable()[tc].autoKind ? " auto" : ""), FontKind::mono, 11, t.dim, x0 + 2, 221, Justification::left);
            const String v = lvl ? (e.ocPk[c] > 1e-4f ? String (l2d (e.ocPk[c]), 1) + " dB" : String ("-inf")) : String (e.grNow[c], 1) + " dB";
            textBase (g, String (lvl ? "Out " : "GR ") + v, FontKind::mono, 11, t.dim, x0 + 210, 221, Justification::right);
        }
        textBase (g, "dB in", FontKind::mono, 11, t.dim, 504, 221, Justification::left);
        textBase (g, "dB out", FontKind::mono, 11, t.dim, 742, 221, Justification::right);
        textBase (g, "A " + String (e.grNow[0], 1), FontKind::mono, 11, t.dim, 754, 221, Justification::left);
        textBase (g, "B " + String (e.grNow[1], 1), FontKind::mono, 11, t.dim, 1040, 221, Justification::right);
        // channel groups
        for (int c = 0; c < 2; ++c)
        {
            const float gx = c ? 676.0f : 62.0f;
            g.setColour (Colours::black.withAlpha (0.14f)); g.fillRoundedRectangle (gx, 238, 370, 302, 10);
            g.setColour (t.line); g.drawRoundedRectangle (gx + 0.5f, 238.5f, 369, 301, 10, 1);
            text (g, c ? "CHANNEL TWO \xc2\xb7 RIGHT" : "CHANNEL ONE \xc2\xb7 LEFT", FontKind::bold, 14, 4, t.title, { gx, 245, 370, 17 }, Justification::centred);
            drawLadder (g, c, gx + 11);
        }
        for (int i = 0; i < 8; ++i) drawKnob (g, kKnobs[i]);
        // middle column extras
        text (g, "SIDE-CHAIN HP", FontKind::regular, 12, 2, t.title, { 478.14f, 386.98f, 90, 14 }, Justification::centredLeft);
        drawSelect (g);
        // meters column
        text (g, "IN", FontKind::regular, 12, 1.5f, t.title, { 1062.64f, 54, 30, 14 }, Justification::centredLeft);
        text (g, "OUT", FontKind::regular, 12, 1.5f, t.title, { 1100.3f, 54, 40, 14 }, Justification::centredLeft);
        textBase (g, e.inPk > 1e-4f ? String (l2d (e.inPk), 1) : String ("-inf"), FontKind::mono, 11, t.dim, 1073.09f, 385, Justification::left);
        textBase (g, e.outPk > 1e-4f ? String (l2d (e.outPk), 1) : String ("-inf"), FontKind::mono, 11, t.dim, 1105.5f, 385, Justification::left);
        // footer
        text (g, "Studio 87 \xc2\xb7 Furchild v" JucePlugin_VersionString, FontKind::regular, 12, 1, t.dim, { 64, 554, 300, 14 }, Justification::centredLeft);
        text (g, String (roundToInt (SW)) + " \xc3\x97 " + String (roundToInt (SH)) + " px", FontKind::regular, 12, 1, t.dim, { 1000, 554, 136, 14 }, Justification::centredRight);
        // toast and info bubble
        if (toastTicks > 0)
        {
            auto r = Rectangle<float> (400, 528, 400, 26); g.setColour (Colours::black); g.fillRoundedRectangle (r, 5);
            g.setColour (theme::selb); g.drawRoundedRectangle (r.reduced (0.5f), 5, 1);
            text (g, toastText, FontKind::regular, 14, 0, t.ink, r, Justification::centred);
        }
        if (tips && tipText.isNotEmpty())
        {
            const int tw = 300;
            AttributedString as; as.setWordWrap (AttributedString::byWord); as.append (tipText, theme::font (FontKind::regular, 14.0f), t.ink);
            TextLayout tl; tl.createLayout (as, (float) tw - 20.0f);
            const float th = tl.getHeight() + 14.0f; float x = jlimit (56.0f, 1144.0f - tw, tipPos.x), y = tipPos.y + 22.0f;
            if (y + th > SH - 4) y = tipPos.y - th - 8;
            auto r = Rectangle<float> (x, y, (float) tw, th); g.setColour (Colours::black); g.fillRoundedRectangle (r, 5);
            g.setColour (theme::selb); g.drawRoundedRectangle (r.reduced (0.5f), 5, 1); tl.draw (g, r.reduced (10.0f, 7.0f));
        }
    }

    bool btnOn (const String& id) const
    {
        if (id == "mLR") return get ("mode") < 0.5f;
        if (id == "mLV") return get ("mode") > 0.5f;
        if (id == "bLink") return get ("link") > 0.5f;
        if (id == "bLevel") return get ("levelMatch") > 0.5f;
        if (id == "mtR") return getUi ("meter", 0) < 0.5f;
        if (id == "mtL") return getUi ("meter", 0) > 0.5f;
        if (id == "bBypass") return get ("bypass") > 0.5f;
        return false;
    }
    void drawBtn (Graphics& g, const Btn& b) const
    {
        const auto& t = T(); const String id (b.id); const bool on = btnOn (id);
        Rectangle<float> r (b.x, b.y, b.w, b.h);
        g.setColour (on ? theme::sel : t.plate); g.fillRoundedRectangle (r, 4);
        g.setColour (on ? theme::selb : (hot == id ? t.dim : t.line)); g.drawRoundedRectangle (r.reduced (0.5f), 4, 1);
        String s = b.text;
        if (id == "pName") s = presetName;
        if (id == "bLevel" && on) { const float lg = l2d (proc.engine.lm); s = "Match " + String (lg >= 0 ? "+" : "") + String (lg, 1); }
        const bool mid = b.y > 280;
        text (g, s, id == "pName" ? FontKind::semi : FontKind::regular, id == "pName" ? 16.0f : mid ? 13.0f : 14.0f, 0.5f, on ? theme::lilac : t.ink, r, Justification::centred);
    }
    void drawSelect (Graphics& g) const
    {
        const auto& t = T(); const auto r = kSelHP;
        g.setColour (t.disp); g.fillRoundedRectangle (r, 4); g.setColour (hot == "selHP" ? t.dim : t.line); g.drawRoundedRectangle (r.reduced (0.5f), 4, 1);
        static const char* o[4] = { "Off", "60 Hz", "120 Hz", "240 Hz" };
        text (g, o[jlimit (0, 3, (int) std::round (get ("hp")))], FontKind::regular, 13, 0, t.ink, r.withTrimmedLeft (6), Justification::centredLeft);
        Path p; const float cx = r.getRight() - 12, cy = r.getCentreY(); p.startNewSubPath (cx - 4, cy - 2); p.lineTo (cx, cy + 2); p.lineTo (cx + 4, cy - 2);
        g.setColour (t.ink); g.strokePath (p, PathStrokeType (1.5f));
    }

    // ---------------------------------------------------------------- knobs
    void drawKnob (Graphics& g, const KnobCfg& k) const
    {
        const auto& t = T(); const String pid (k.pid);
        const float v = get (pid), pos = (v - k.mn) / (k.mx - k.mn), cxk = k.x + k.size * 0.5f;
        const float ls = k.size / 150.0f, ox = k.x, oy = k.y;
        auto P = [&] (float x, float y) { return Point<float> (ox + x * ls, oy + y * ls); };
        const float a0 = -135.0f, span = 270.0f;
        auto ang = [&] (float val) { return (a0 + span * (val - k.mn) / (k.mx - k.mn)) * MathConstants<float>::pi / 180.0f; };
        // label above
        const String names[8] = { "INPUT", "THRESHOLD", "TIME", "OUTPUT", "MIX", "INPUT", "THRESHOLD", "TIME" };
        int ki = 0; for (int i = 0; i < 8; ++i) if (&kKnobs[i] == &k) ki = i;
        text (g, names[ki], FontKind::bold, 14, 3, t.title, { cxk - 60, k.size > 100 ? 296.0f : 410.98f, 120, 17 }, Justification::centred);
        // ticks
        Graphics::ScopedSaveState ss (g);
        if (k.discrete)
            for (int v2 = (int) k.mn; v2 <= (int) k.mx; ++v2) { const float a = ang ((float) v2); g.setColour (Colour (0xfff4f5f7)); g.drawLine (Line<float> (P (75 + std::sin (a) * 64, 75 - std::cos (a) * 64), P (75 + std::sin (a) * 72, 75 - std::cos (a) * 72)), 2 * ls); }
        else
            for (int i = 0; i <= 20; ++i)
            {
                const float a = (a0 + i * 13.5f) * MathConstants<float>::pi / 180.0f; const bool major = i % 2 == 0; const float r1 = major ? 66.0f : 68.0f;
                g.setColour (major ? Colour (0xfff4f5f7) : Colour (0xff8c8f99));
                g.drawLine (Line<float> (P (75 + std::sin (a) * r1, 75 - std::cos (a) * r1), P (75 + std::sin (a) * 72, 75 - std::cos (a) * 72)), (major ? 1.8f : 1.0f) * ls);
            }
        for (const auto& tk : k.ticks)
        {
            const float a = ang (tk.first); const auto p = P (75 + std::sin (a) * 56, 75 - std::cos (a) * 56 + 4);
            textBase (g, tk.second, FontKind::semi, 13 * ls, Colour (0xffc9cbd3), p.x, p.y, Justification::centred);
        }
        g.setColour (Colour (0xff0b0b0d)); g.fillEllipse (Rectangle<float> (0, 0, 84 * ls, 84 * ls).withCentre (P (75, 75)));
        g.setColour (Colours::black); g.drawEllipse (Rectangle<float> (0, 0, 84 * ls, 84 * ls).withCentre (P (75, 75)), 1 * ls);
        { Path ring; ring.addEllipse (Rectangle<float> (0, 0, 80 * ls, 80 * ls).withCentre (P (75, 75))); Path dashed; const float dl[2] { 1.6f * ls, 1.8f * ls };
          PathStrokeType (4 * ls).createDashedStroke (dashed, ring, dl, 2); g.setColour (Colour (0xff4a4c55)); g.fillPath (dashed); }
        // rotating cap
        const float ra = (a0 + span * pos) * MathConstants<float>::pi / 180.0f;
        const auto c0 = P (75, 75);
        { ColourGradient gr (Colour (0xff5a5c66), P (75 - 33 * 0.2f, 75 - 33 * 0.3f), Colour (0xff101114), P (75 + 33 * 0.9f, 75 + 33 * 0.9f), true);
          gr.addColour (0.55, Colour (0xff2b2c33)); g.setGradientFill (gr); g.fillEllipse (Rectangle<float> (0, 0, 66 * ls, 66 * ls).withCentre (c0)); }
        g.setColour (Colour (0xff5a5c66)); g.drawEllipse (Rectangle<float> (0, 0, 66 * ls, 66 * ls).withCentre (c0), 1 * ls);
        g.setColour (Colours::white.withAlpha (0.08f)); g.drawEllipse (Rectangle<float> (0, 0, 58 * ls, 58 * ls).withCentre (c0), 1 * ls);
        auto rot = [&] (float x, float y) { const float dx = x - 75, dy = y - 75; return P (75 + dx * std::cos (ra) - dy * std::sin (ra), 75 + dx * std::sin (ra) + dy * std::cos (ra)); };
        Path tri; tri.startNewSubPath (rot (71, 46)); tri.lineTo (rot (75, 36)); tri.lineTo (rot (79, 46)); tri.closeSubPath(); g.setColour (theme::pink); g.fillPath (tri);
        g.setColour (Colour (0xfff4f5f7)); g.drawLine (Line<float> (c0, rot (75, 48)), 3.5f * ls);
        // value text
        const float ty = k.size > 100 ? 420.0f : 503.0f;
        if (pid == "inA" || pid == "inB") textBase (g, fDb (v, 0), FontKind::mono, 11, t.ink, cxk, ty + 11, Justification::centred);
        else if (pid == "thA" || pid == "thB")
        {
            textBase (g, String (v, 1), FontKind::mono, 11, t.ink, cxk, ty + 11, Justification::centred);
            textBase (g, fDb (fc::thToDb (v)) + " thr", FontKind::mono, 11, t.dim, cxk, ty + 26, Justification::centred);
        }
        else if (pid == "tcA" || pid == "tcB")
        {
            const int tc = jlimit (0, 5, (int) std::round (v)); const auto& tt = fc::tcTable()[tc];
            text (g, "FAST", FontKind::regular, 11, 2, t.title, { cxk - 56, 420, 60, 13 }, Justification::centredLeft);
            text (g, "SLOW", FontKind::regular, 11, 2, t.title, { cxk - 4, 420, 60, 13 }, Justification::centredRight);
            textBase (g, "TC " + String (tc + 1) + (tt.autoKind ? "  auto" : "  fixed"), FontKind::mono, 11, t.ink, cxk, 442, Justification::centred);
            textBase (g, "Attack " + String (tt.att, 1).replace (".0", "") + " ms", FontKind::mono, 11, t.dim, cxk, 457, Justification::centred);
            textBase (g, String (tt.rtxt), FontKind::mono, 11, t.dim, cxk, 471, Justification::centred);
        }
        else if (pid == "out") textBase (g, fDb (v), FontKind::mono, 11, t.ink, cxk, ty + 11, Justification::centred);
        else if (pid == "mix") textBase (g, String (roundToInt (v)) + "%", FontKind::mono, 11, t.ink, cxk, ty + 11, Justification::centred);
    }

    void drawLadder (Graphics& g, int c, float x0)
    {
        const auto& t = T(); const float gr = proc.engine.grNow[c];
        for (int i = 0; i < 12; ++i)
        {
            const float x = x0 + 29.0f * i + 14.5f;
            textBase (g, String (LAD[i]), FontKind::mono, 10, t.dim, x, 273.0f, Justification::centred);
            const Colour col = LAD[i] >= 16 ? theme::red : LAD[i] >= 10 ? theme::orange : LAD[i] >= 5 ? t.yellow : theme::green;
            const bool on = gr >= (float) LAD[i] - 0.25f; const Rectangle<float> r (x - 6, 280, 12, 12);
            if (on) { for (int k = 4; k >= 1; --k) { g.setColour (col.withAlpha (0.07f)); g.fillEllipse (r.expanded ((float) k * 1.6f)); } g.setColour (col); }
            else g.setColour (Colour (0xff26282c));
            g.fillEllipse (r);
            if (! on) { g.setColour (Colours::black.withAlpha (0.5f)); g.drawEllipse (r.reduced (0.5f), 1.0f); }
        }
    }

    // ---------------------------------------------------------------- canvases
    void drawCanvas (Graphics& g, const Cv& c)
    {
        const auto& t = T(); const String id (c.id);
        Rectangle<float> r (c.x, c.y, c.w, c.h);
        const bool bordered = id == "cvDialA" || id == "cvDialB" || id == "cvCurve" || id == "cvHist";
        Rectangle<float> in = bordered ? r.reduced (1) : r;
        if (bordered) { g.setColour (t.disp); g.fillRoundedRectangle (r, 3); g.setColour (t.line); g.drawRoundedRectangle (r.reduced (0.5f), 3, 1); }
        Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (in.toNearestInt());
        g.addTransform (AffineTransform::translation (in.getX(), in.getY()));
        if (id == "cvDialA") drawDial (g, 0, in.getWidth(), in.getHeight());
        else if (id == "cvDialB") drawDial (g, 1, in.getWidth(), in.getHeight());
        else if (id == "cvCurve") drawCurve (g, in.getWidth(), in.getHeight());
        else if (id == "cvHist") drawHist (g, in.getWidth(), in.getHeight());
        else if (id == "cvTubes") drawTubes (g, in.getWidth(), in.getHeight());
        else drawMeter (g, id == "cvIn", in.getWidth(), in.getHeight());
    }
    void drawMeter (Graphics& g, bool isIn, float w, float h)
    {
        const auto& t = T(); auto& e = proc.engine; const float mn = -60, mx = 6;
        auto y = [&] (float db) { return h - 6 - (clampf (db, mn, mx) - mn) / (mx - mn) * (h - 12); };
        g.setColour (t.disp); g.fillRect (0.0f, 0.0f, w, h);
        const float db = l2d (isIn ? e.inPk : e.outPk), top = y (db);
        ColourGradient gr (theme::blue, 0, y (mn), theme::red, 0, y (mx), false);
        auto f = [&] (float d) { return (double) ((d - mn) / (mx - mn)); };
        gr.addColour (f (-12), theme::blue); gr.addColour (f (-12) + 0.001, theme::purple); gr.addColour (f (0), theme::purple); gr.addColour (f (0) + 0.001, theme::red);
        g.setGradientFill (gr); g.fillRect (2.0f, top, 12.0f, y (mn) - top);
        g.setColour (t.dim);
        for (int tv : { 0, -6, -12, -24, -36, -48 }) { const float yy = y ((float) tv); g.fillRect (14.0f, yy, 3.0f, 1.0f); textBase (g, String (tv), FontKind::mono, 9, t.dim, w - 1, yy + 3, Justification::right); }
        bool& cl = isIn ? clipIn : clipOut; if (db >= 0) cl = true;
        g.setColour (cl ? theme::red : Colour (0xff2a2c31)); g.fillRect (2.0f, 0.0f, 12.0f, 4.0f);
    }
    void drawDial (Graphics& g, int c, float w, float h)
    {
        const auto& t = T(); const bool lvl = getUi ("meter", 0) > 0.5f; auto& e = proc.engine;
        g.setColour (t.disp); g.fillRect (0.0f, 0.0f, w, h);
        const Point<float> piv (105, 130); const float DR = 98, SWd = 56;
        const float mn = lvl ? -30.0f : 0.0f, mx = lvl ? 3.0f : 20.0f;
        auto ang = [&] (float v) { return -SWd + 2 * SWd * (clampf (v, mn, mx) - mn) / (mx - mn); };
        auto ptAt = [&] (float a, float r) { return Point<float> (piv.x + std::sin (a * MathConstants<float>::pi / 180) * r, piv.y - std::cos (a * MathConstants<float>::pi / 180) * r); };
        auto arc = [&] (float v0, float v1, float r, Colour col, float lw)
        {
            Path p; p.addCentredArc (piv.x, piv.y, r, r, 0, ang (v0) * MathConstants<float>::pi / 180, ang (v1) * MathConstants<float>::pi / 180, true);
            g.setColour (col); g.strokePath (p, PathStrokeType (lw));
        };
        if (lvl) { arc (-30, -12, DR + 5, theme::blue, 4); arc (-12, 0, DR + 5, theme::purple, 4); arc (0, 3, DR + 5, theme::red, 4); }
        else { arc (0, 12, DR + 5, t.accent, 4); arc (12, 20, DR + 5, theme::red, 4); }
        std::vector<float> labs = lvl ? std::vector<float> { -30, -20, -10, 0, 3 } : std::vector<float> { 0, 5, 10, 15, 20 };
        for (float v = mn; v <= mx + 1e-6f; v += 1.0f)
        {
            bool major = false; for (float l : labs) if (std::fabs (l - v) < 1e-4f) major = true;
            const float a = ang (v); const auto p0 = ptAt (a, DR - (major ? 9.0f : 5.0f)), p1 = ptAt (a, DR);
            g.setColour (t.dim); g.drawLine (Line<float> (p0, p1), major ? 1.6f : 1.0f);
        }
        for (float v : labs) { const auto p = ptAt (ang (v), DR - 22); textBase (g, String (lvl && v > 0 ? "+" : "") + String (roundToInt (v)), FontKind::mono, 10, t.ink, p.x, p.y + 3, Justification::centred); }
        textBase (g, lvl ? "LEVEL dB" : "GAIN REDUCTION dB", FontKind::regular, 11, t.dim, 6, 14, Justification::left);
        const float val = needle[c]; const auto np = ptAt (ang (val), DR - 4);
        g.setColour (theme::pink); g.drawLine (Line<float> (piv, np), 2.0f);
        g.setColour (Colour (0xff2a2c33)); g.fillEllipse (Rectangle<float> (18, 18).withCentre (piv));
        g.setColour (t.dim); g.drawEllipse (Rectangle<float> (18, 18).withCentre (piv), 1.0f);
        (void) e;
    }
    void dashed (Graphics& g, Point<float> a, Point<float> b, float on, float off, float th) const
    {
        Path p; p.startNewSubPath (a); p.lineTo (b); Path d; const float dl[2] { on, off }; PathStrokeType (th).createDashedStroke (d, p, dl, 2); g.fillPath (d);
    }
    void drawCurve (Graphics& g, float w, float h)
    {
        const auto& t = T(); auto& e = proc.engine; const auto& p = e.p;
        g.setColour (t.disp); g.fillRect (0.0f, 0.0f, w, h);
        const float x0 = -60, x1 = 6, y0 = -60, y1 = 18;
        auto X = [&] (float v) { return 26 + (v - x0) / (x1 - x0) * (w - 32); };
        auto Y = [&] (float v) { return h - 14 - (v - y0) / (y1 - y0) * (h - 22); };
        for (int v : { -60, -48, -36, -24, -12, 0 }) { g.setColour (t.line); g.fillRect (X ((float) v), 4.0f, 1.0f, h - 18); textBase (g, String (v), FontKind::mono, 9, t.dim, X ((float) v), h - 4, Justification::centred); }
        for (int v : { -48, -24, 0, 12 }) { g.setColour (t.line); g.fillRect (26.0f, Y ((float) v), w - 32, 1.0f); textBase (g, String (v), FontKind::mono, 9, t.dim, 23, Y ((float) v) + 3, Justification::right); }
        g.setColour (Colour (0xff4a4d55)); dashed (g, { X (-60), Y (-60) }, { X (6), Y (6) }, 3, 3, 1);
        for (int ci = p.link ? 0 : 1; ci >= 0; --ci)
        {
            const Colour col = ci ? theme::pink : t.accent;
            const float tx = (p.link ? p.T[0] : p.T[ci]) - (p.link ? p.gIn[0] : p.gIn[ci]);
            if (tx > x0 && tx < x1) { g.setColour (col.withAlpha (0.55f)); dashed (g, { X (tx), 4 }, { X (tx), h - 14 }, 2, 3, 1); }
            Path path;
            for (float v = x0; v <= x1 + 1e-4f; v += 0.5f) { const float o = p.bypass ? v : e.curveOut (v, ci); const Point<float> pt (X (v), Y (clampf (o, y0, y1))); if (v == x0) path.startNewSubPath (pt); else path.lineTo (pt); }
            g.setColour (col); g.strokePath (path, PathStrokeType (2));
        }
        const float li = l2d (e.inPk), lo = l2d (e.outPk);
        if (std::isfinite (li) && li > -60) { g.setColour (Colours::white); g.fillEllipse (Rectangle<float> (8, 8).withCentre ({ X (std::min (x1, li)), Y (clampf (lo, y0, y1)) })); }
    }
    void drawHist (Graphics& g, float w, float h)
    {
        const auto& t = T();
        g.setColour (t.disp); g.fillRect (0.0f, 0.0f, w, h);
        auto Y = [&] (float d) { return 8 + d / 30 * (h - 22); };
        for (int d : { 0, 6, 12, 18, 24, 30 }) { g.setColour (t.line); g.fillRect (26.0f, Y ((float) d), w - 30, 1.0f); textBase (g, "-" + String (d), FontKind::mono, 9, t.dim, 23, Y ((float) d) + 3, Justification::right); }
        for (int k = 0; k < 2; ++k)
        {
            if (k == 1 && proc.engine.p.link) continue;
            const float* hs = k ? histB : histA; const Colour col = k ? theme::pink : t.accent;
            Path path; path.startNewSubPath (28, Y (0));
            for (int i = 0; i < HN; ++i) path.lineTo (28 + (float) i / 287.0f * (w - 32), Y (std::min (30.0f, hs[(hi + i) % HN])));
            Path fill (path); fill.lineTo (w - 4, Y (0)); fill.closeSubPath();
            g.setColour (col.withAlpha (0.22f)); g.fillPath (fill);
            g.setColour (col); g.strokePath (path, PathStrokeType (1.6f));
        }
    }
    void drawTubes (Graphics& g, float w, float h)
    {
        const auto& t = T(); auto& e = proc.engine;
        g.setColour (t.disp); g.fillRect (0.0f, 0.0f, w, h);
        for (int i = 0; i < 4; ++i)
        {
            const int c = i < 2 ? 0 : 1; const float x = 27 + i * 53, gr = e.grNow[e.p.link ? 0 : c], glow = 0.18f + 0.82f * std::min (1.0f, gr / 18.0f);
            ColourGradient rg (rgba (255, 166, 0, glow * 0.9f), x, 30, rgba (255, 166, 0, 0), x + 24, 30, true); g.setGradientFill (rg); g.fillRect (x - 26, 2.0f, 52.0f, 48.0f);
            Path o; o.startNewSubPath (x - 10, 46); o.lineTo (x - 10, 18); o.quadraticTo (x - 10, 4, x, 4); o.quadraticTo (x + 10, 4, x + 10, 18); o.lineTo (x + 10, 46); o.closeSubPath();
            g.setColour (t.dim); g.strokePath (o, PathStrokeType (1.4f));
            g.setColour (rgba (255, 166, 0, 0.35f + 0.65f * glow)); g.drawLine (x - 4, 42, x - 4, 16, 2); g.drawLine (x + 4, 42, x + 4, 16, 2);
            g.setColour (rgba (255, 210, 120, 0.3f + 0.7f * glow)); g.fillRect (x - 6, 26.0f, 12.0f, 3.0f);
            g.setColour (Colour (0xff2a2c33)); g.fillRect (x - 12, 46.0f, 24.0f, 5.0f);
        }
    }

    // ---------------------------------------------------------------- presets
    File userDir() const { return File::getSpecialLocation (File::userApplicationDataDirectory).getChildFile ("Studio 87").getChildFile ("Furchild").getChildFile ("presets"); }
    static const char* key (int i) { return fcp::kParams[i].id; }
    void applyValues (const float* v)
    {
        proc.engine.resetLM();
        for (int i = 0; i < fcp::kNumParams; ++i) { if (String (key (i)) == "bypass") continue; begin (key (i)); setRaw (key (i), v[i]); end (key (i)); }
        ed.repaint();
    }
    void setName (const String& n) { presetName = n; proc.setSetting (setting::preset, n); ed.repaint(); }
    void toast (const String& s) { toastText = s; toastTicks = 60; ed.repaint(); }
    void applyFactory (int idx) { applyValues (fcp::kFactory[idx].v); setName (fcp::kFactory[idx].name); toast (String ("Loaded ") + fcp::kFactory[idx].name); }
    StringArray userNames() const
    {
        StringArray s; for (auto& f : userDir().findChildFiles (File::findFiles, false, "*.fcp")) s.add (f.getFileNameWithoutExtension()); s.sort (true); return s;
    }
    bool isUser (const String& n) const { return userDir().getChildFile (n + ".fcp").existsAsFile(); }
    void applyUser (const String& name)
    {
        if (auto xml = XmlDocument::parse (userDir().getChildFile (name + ".fcp")))
        {
            float v[fcp::kNumParams]; for (int i = 0; i < fcp::kNumParams; ++i) v[i] = fcp::kParams[i].df;
            for (auto* e : xml->getChildIterator()) for (int i = 0; i < fcp::kNumParams; ++i) if (e->getStringAttribute ("id") == key (i)) v[i] = clampf ((float) e->getDoubleAttribute ("v"), fcp::kParams[i].mn, fcp::kParams[i].mx);
            applyValues (v); setName (name); toast ("Loaded " + name);
        }
    }
    void saveUser (const String& rawName)
    {
        const String name = rawName.removeCharacters ("\\/:*?\"<>|").trim().substring (0, 40); if (name.isEmpty()) return;
        auto f = userDir().getChildFile (name + ".fcp"); f.getParentDirectory().createDirectory();
        XmlElement x ("FurchildPreset");
        for (int i = 0; i < fcp::kNumParams; ++i) { if (String (key (i)) == "bypass") continue; auto* e = x.createNewChildElement ("p"); e->setAttribute ("id", key (i)); e->setAttribute ("v", (double) get (key (i))); }
        if (x.writeTo (f)) { setName (name); toast ("Saved " + name); } else toast ("Could not save the preset");
    }
    void askSave()
    {
        auto* w = new AlertWindow ("Save preset", "Name for your preset. Saving with an existing name replaces it.", MessageBoxIconType::NoIcon, &ed);
        w->addTextEditor ("n", isUser (presetName) ? presetName : String(), "Name"); w->addButton ("Save", 1); w->addButton ("Cancel", 0);
        w->enterModalState (true, ModalCallbackFunction::create ([this, w] (int r)
        {
            const String n = w->getTextEditorContents ("n");
            if (r == 1) { if (isUser (n.removeCharacters ("\\/:*?\"<>|").trim())) askOverwrite (n); else saveUser (n); }
        }), true);
    }
    void askOverwrite (const String& n)
    {
        auto opts = MessageBoxOptions().withIconType (MessageBoxIconType::QuestionIcon).withTitle ("Replace preset?").withMessage ("A preset called \"" + n + "\" exists. Replace it?").withButton ("Replace").withButton ("Cancel");
        AlertWindow::showAsync (opts, [this, n] (int r) { if (r == 1) saveUser (n); });
    }
    void askRename()
    {
        if (! isUser (presetName)) { toast ("Only your own presets can be renamed"); return; }
        auto* w = new AlertWindow ("Rename preset", "New name for your preset.", MessageBoxIconType::NoIcon, &ed);
        w->addTextEditor ("n", presetName, "Name"); w->addButton ("Rename", 1); w->addButton ("Cancel", 0);
        w->enterModalState (true, ModalCallbackFunction::create ([this, w] (int r)
        {
            const String n = w->getTextEditorContents ("n").removeCharacters ("\\/:*?\"<>|").trim().substring (0, 40);
            if (r == 1 && n.isNotEmpty() && n != presetName && ! isUser (n)) { if (userDir().getChildFile (presetName + ".fcp").moveFileTo (userDir().getChildFile (n + ".fcp"))) { setName (n); toast ("Renamed to " + n); } }
        }), true);
    }
    void stepPreset (int dir)
    {
        const auto un = userNames(); const int nf = fcp::kNumFactory, tot = nf + un.size(); int cur = -1;
        for (int i = 0; i < nf; ++i) if (presetName == fcp::kFactory[i].name) cur = i;
        for (int i = 0; i < un.size(); ++i) if (presetName == un[i]) cur = nf + i;
        cur = cur < 0 ? (dir > 0 ? 0 : tot - 1) : (cur + dir + tot) % tot;
        if (cur < nf) applyFactory (cur); else applyUser (un[cur - nf]);
    }
    void presetMenu (Point<int> where)
    {
        PopupMenu m; StringArray types;
        for (int i = 0; i < fcp::kNumFactory; ++i) types.addIfNotAlreadyThere (fcp::kFactory[i].type);
        for (const auto& ty : types)
        {
            PopupMenu sub;
            for (int i = 0; i < fcp::kNumFactory; ++i) if (String (fcp::kFactory[i].type) == ty) sub.addItem (i + 1, fcp::kFactory[i].name, true, presetName == fcp::kFactory[i].name);
            m.addSubMenu (ty, sub);
        }
        const auto un = userNames();
        if (un.size() > 0) { PopupMenu sub; for (int i = 0; i < un.size(); ++i) sub.addItem (1000 + i, un[i], true, presetName == un[i]); m.addSubMenu ("My presets", sub); }
        m.showMenuAsync (PopupMenu::Options().withTargetScreenArea (Rectangle<int> (where.x, where.y, 1, 1)), [this, un] (int r)
        { if (r >= 1000) applyUser (un[r - 1000]); else if (r > 0) applyFactory (r - 1); });
    }
    void settingsMenu (Point<int> where)
    {
        PopupMenu m; m.addSectionHeader ("Settings");
        m.addItem (1, "Info bubbles", true, tips);
        m.addItem (2, "Power saving (slower redraw)", true, (bool) proc.getSetting (setting::power, false));
        PopupMenu lk; lk.addItem (11, "A  neon navy", true, look == 0); lk.addItem (12, "B  purple soda pop", true, look == 1); lk.addItem (13, "C  charcoal", true, look == 2);
        m.addSubMenu ("Colour look", lk);
        m.showMenuAsync (PopupMenu::Options().withTargetScreenArea (Rectangle<int> (where.x, where.y, 1, 1)), [this] (int r)
        {
            if (r == 1) { tips = ! tips; proc.setSetting (setting::tips, tips); }
            else if (r == 2) { const bool v = ! (bool) proc.getSetting (setting::power, false); proc.setSetting (setting::power, v); rate = v ? 12 : 30; ed.startTimerHz (rate); }
            else if (r >= 11 && r <= 13) { look = r - 11; proc.setSetting (setting::look, look); }
            ed.repaint();
        });
    }
    void aboutBox (bool guide)
    {
        String msg = guide
            ? "Furchild is a variable-mu style compressor: the harder the signal pushes, the more it turns down, with a soft knee. Input sets how hard the signal hits the box, in 1 dB steps. Threshold is where squeezing starts. Time picks one of six attack and release pairs; 5 and 6 are automatic. Left / Right treats the channels separately; Lat / Vert squeezes the middle and the sides on their own. Link turns both channels down together. Level Match keeps the processed sound as loud as the original so you judge tone, not volume. Side-chain HP keeps deep bass from triggering the squeeze. Hover any control for an info bubble. Double-click a knob to reset it; Shift while dragging for fine steps. Keys: B bypass, [ and ] previous and next preset."
            : "Furchild by Studio 87. Free and open source (AGPL-3.0-or-later). No telemetry, no data collection, no network access. Your own presets are stored in your user data folder under Studio 87/Furchild/presets. https://github.com/SpeakingColors/Furchild";
        AlertWindow::showAsync (MessageBoxOptions().withIconType (MessageBoxIconType::NoIcon).withTitle ("Furchild").withMessage (msg).withButton ("OK"), nullptr);
    }

    // ---------------------------------------------------------------- hit testing
    struct Hit { String id; Rectangle<float> r; };
    std::vector<Hit> hits() const
    {
        std::vector<Hit> h;
        for (const auto& b : kBtns) h.push_back ({ b.id, { b.x, b.y, b.w, b.h } });
        for (const auto& c : kCv) h.push_back ({ c.id, { c.x, c.y, c.w, c.h } });
        for (const auto& k : kKnobs) h.push_back ({ k.pid, { k.x, k.y, k.size, k.size } });
        h.push_back ({ "selHP", kSelHP });
        h.push_back ({ "ladA", { 73, 265, 348, 27 } }); h.push_back ({ "ladB", { 687, 265, 348, 27 } });
        h.push_back ({ "pkIn", { 1073, 374, 28, 14 } }); h.push_back ({ "pkOut", { 1105, 374, 28, 14 } });
        h.push_back ({ "cheek", kCheekL }); h.push_back ({ "cheek", kCheekR });
        return h;
    }
    String hitId (Point<float> p) const { auto h = hits(); for (int i = (int) h.size() - 1; i >= 0; --i) if (h[(size_t) i].r.contains (p)) return h[(size_t) i].id; return {}; }
    Point<float> toStage (Point<float> p) const { const float s = (float) ed.getWidth() / SW; return { p.x / s, p.y / s }; }
    const KnobCfg* knobOf (const String& id) const { for (const auto& k : kKnobs) if (id == k.pid) return &k; return nullptr; }

    void doButton (const String& id, Point<int> scr)
    {
        if (id == "mLR") setUser ("mode", 0); else if (id == "mLV") setUser ("mode", 1);
        else if (id == "bLink") setUser ("link", get ("link") > 0.5f ? 0.0f : 1.0f);
        else if (id == "bLevel") setUser ("levelMatch", get ("levelMatch") > 0.5f ? 0.0f : 1.0f);
        else if (id == "mtR") { proc.setSetting ("ui.meter", 0); proc.setSetting (setting::meter, 0); }
        else if (id == "mtL") { proc.setSetting (setting::meter, 1); }
        else if (id == "bBypass") setUser ("bypass", get ("bypass") > 0.5f ? 0.0f : 1.0f);
        else if (id == "pPrev") stepPreset (-1); else if (id == "pNext") stepPreset (1);
        else if (id == "pName") presetMenu (scr); else if (id == "pSave") askSave();
        else if (id == "bSettings") settingsMenu (scr); else if (id == "bGuide") aboutBox (true); else if (id == "bAbout") aboutBox (false);
        ed.repaint();
    }
    void down (const MouseEvent& e)
    {
        const auto p = toStage (e.position); const String id = hitId (p); dragId = {}; dragging = false;
        if (id.isEmpty()) return;
        if (const auto* k = knobOf (id)) { dragId = id; dragStart = get (id); dragY = e.position.y; dragging = true; begin (id); (void) k; return; }
        if (id == "cvIn") { clipIn = false; return; }
        if (id == "cvOut") { clipOut = false; return; }
        if (id == "selHP")
        {
            PopupMenu m; const int cur = (int) std::round (get ("hp")); const char* o[4] = { "Off", "60 Hz", "120 Hz", "240 Hz" };
            for (int i = 0; i < 4; ++i) m.addItem (i + 1, o[i], true, i == cur);
            const float s = (float) ed.getWidth() / SW;
            m.showMenuAsync (PopupMenu::Options().withTargetComponent (&ed).withTargetScreenArea (ed.localAreaToGlobal (Rectangle<float> (kSelHP.getX() * s, kSelHP.getBottom() * s, 1, 1).toNearestInt())),
                             [this] (int r) { if (r > 0) setUser ("hp", (float) (r - 1)); });
            return;
        }
        for (const auto& b : kBtns) if (id == b.id) { doButton (id, ed.localPointToGlobal (e.getPosition())); return; }
    }
    void drag (const MouseEvent& e)
    {
        if (! dragging) return;
        const auto* k = knobOf (dragId); if (! k) return;
        const float s = (float) ed.getWidth() / SW, dy = -(e.position.y - dragY) / s, rng = k->mx - k->mn;
        const float v = dragStart + dy * (rng / (k->discrete ? 150.0f : 220.0f)) * (e.mods.isShiftDown() ? 0.1f : 1.0f);
        set (dragId, fix (*k, v));
    }
    void up() { if (dragging) end (dragId); dragging = false; dragId = {}; }
    void dbl (const MouseEvent& e)
    {
        const auto p = toStage (e.position); const String id = hitId (p);
        if (id == "cheek") { ed.setSize ((int) SW, (int) SH); return; }
        if (id == "pName") { askRename(); return; }
        if (const auto* k = knobOf (id)) { const auto* d = defOf (id); if (d) { setUser (id, d->df); (void) k; } }
    }
    void wheel (const MouseEvent& e, const MouseWheelDetails& w)
    {
        const String id = hitId (toStage (e.position)); const auto* k = knobOf (id); if (! k) return;
        const float dy = w.isReversed ? -w.deltaY : w.deltaY; if (std::fabs (dy) < 1.0e-6f) return;
        const float step = k->discrete || id.startsWith ("in") ? 1.0f : (k->mx - k->mn) * (e.mods.isShiftDown() ? 0.002f : 0.02f);
        begin (id); set (id, fix (*k, get (id) + (dy > 0 ? 1 : -1) * step)); end (id);
    }
    void move (const MouseEvent& e)
    {
        const auto p = toStage (e.position); const String id = hitId (p);
        if (id != hot) { hot = id; tipTicks = 0; tipText = {}; ed.repaint(); }
        tipPos = p;
        ed.setMouseCursor (knobOf (id) ? MouseCursor::UpDownResizeCursor : id.isNotEmpty() && id != "cheek" ? MouseCursor::PointingHandCursor : MouseCursor::NormalCursor);
    }
    void tick()
    {
        auto& e = proc.engine; const bool lvl = getUi ("meter", 0) > 0.5f;
        const float k = 1.0f - std::pow (0.6f, 60.0f / (float) rate);
        for (int c = 0; c < 2; ++c) { const float tgt = lvl ? std::max (-30.0f, l2d (e.ocPk[c])) : e.grNow[c]; needle[c] += (tgt - needle[c]) * k; }
        histAcc += 48.0f / (float) rate;
        while (histAcc >= 1.0f) { histAcc -= 1.0f; histA[hi] = e.grPk[0]; histB[hi] = e.grPk[1]; hi = (hi + 1) % HN; }
        if (tips && hot.isNotEmpty() && tipText.isEmpty() && ++tipTicks > rate) tipText = String::fromUTF8 (fcp::tipFor (hot.toRawUTF8()));
        if (toastTicks > 0) --toastTicks;
        ed.repaint();
    }
};

// ================================================================================================
FurchildEditor::FurchildEditor (FurchildProcessor& p) : AudioProcessorEditor (p), d (std::make_unique<Impl> (*this, p))
{
    setOpaque (true);
    setSize ((int) SW, (int) SH);
    setResizable (true, true);
    setResizeLimits (600, 287, 2400, 1148);
    getConstrainer()->setFixedAspectRatio ((double) SW / SH);
    setWantsKeyboardFocus (true);
    d->rate = (bool) p.getSetting (setting::power, false) ? 12 : 30;
    startTimerHz (d->rate);
}
FurchildEditor::~FurchildEditor() { stopTimer(); }
void FurchildEditor::paint (Graphics& g)
{
    const float s = (float) getWidth() / SW;
    g.addTransform (AffineTransform::scale (s));
    g.setImageResamplingQuality (Graphics::highResamplingQuality);
    d->paintAll (g);
}
void FurchildEditor::timerCallback() { d->tick(); }
void FurchildEditor::mouseDown (const MouseEvent& e) { d->down (e); }
void FurchildEditor::mouseDrag (const MouseEvent& e) { d->drag (e); }
void FurchildEditor::mouseUp (const MouseEvent&) { d->up(); }
void FurchildEditor::mouseMove (const MouseEvent& e) { d->move (e); }
void FurchildEditor::mouseExit (const MouseEvent&) { d->hot = {}; d->tipText = {}; repaint(); }
void FurchildEditor::mouseDoubleClick (const MouseEvent& e) { d->dbl (e); }
void FurchildEditor::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& w) { d->wheel (e, w); }
bool FurchildEditor::keyPressed (const KeyPress& k)
{
    if (k.getTextCharacter() == 'b' || k.getTextCharacter() == 'B') { d->setUser ("bypass", d->get ("bypass") > 0.5f ? 0.0f : 1.0f); return true; }
    if (k.getTextCharacter() == '[') { d->stepPreset (-1); return true; }
    if (k.getTextCharacter() == ']') { d->stepPreset (1); return true; }
    return false;
}
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))
