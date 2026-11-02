#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>
#include <vector>
#include "Theme.h"
#include "Layout.h"

namespace ui
{
using namespace juce;

// Settings every widget needs to paint (look and info bubbles).
struct Ctx
{
    char look = 'C';
    bool tips = true;
    bool power = false;
    const theme::Look& t() const { return theme::look (look); }
};

// Anything that can show an info bubble.
struct TipClient
{
    String tip;
    virtual ~TipClient() = default;
};

inline Rectangle<float> rc (const layout::Rc& r) { return { r.x, r.y, r.w, r.h }; }

inline float textWidth (const Font& f, const String& s)
{
    GlyphArrangement ga;
    ga.addLineOfText (f, s, 0.0f, 0.0f);
    return ga.getBoundingBox (0, -1, true).getWidth();
}

// Text centred in a box, never shortened with dots. Letter spacing is in px.
inline void drawText (Graphics& g, const String& s, Rectangle<float> r, Font f, Colour c,
                      Justification j = Justification::centred, float spacingPx = 0.0f)
{
    if (std::abs (spacingPx) > 1.0e-6f) f = f.withExtraKerningFactor (spacingPx / jmax (1.0f, f.getHeight())); // JUCE kerning is a fraction of the font height
    g.setFont (f);
    g.setColour (c);
    Rectangle<float> box = r.expanded (30.0f, 0.0f); // room for font differences, text is never shortened
    if (j.testFlags (Justification::left))       box = r.withWidth (r.getWidth() + 60.0f);
    else if (j.testFlags (Justification::right)) box = r.withLeft (r.getX() - 60.0f);
    g.drawText (s, box, j, false);
}

// ---- Button in the mockup's .btn style -----------------------------------------------------------
class PButton : public Component, public TipClient
{
public:
    PButton (const Ctx& c, String t, float px = 14.0f, theme::FontKind k = theme::FontKind::regular)
        : text (std::move (t)), ctx (c), size (px), kind (k) { setMouseCursor (MouseCursor::PointingHandCursor); }

    std::function<void (const ModifierKeys&)> onClick;
    std::function<void()> onDouble;
    bool on = false;
    bool hot = false;                       // red border and pink text (Auto Trim has trimmed)
    float spacing = 0.5f;
    float radius = 4.0f;
    String text;

    void setText (const String& t) { if (t != text) { text = t; repaint(); } }
    void setOn (bool v) { if (v != on) { on = v; repaint(); } }

    void paint (Graphics& g) override
    {
        auto& t = ctx.t();
        auto r = getLocalBounds().toFloat().reduced (0.5f);
        g.setColour (on ? theme::sel : t.plate);
        g.fillRoundedRectangle (r, radius);
        Colour border = hot ? theme::red : on ? theme::selb : (over ? t.dim : t.line);
        g.setColour (border);
        g.drawRoundedRectangle (r, radius, 1.0f);
        const Colour tc = hot ? theme::pink : on ? theme::lilac : t.ink;
        auto f = theme::font (kind, size);
        if (text.contains ("\n"))
        {
            auto lines = StringArray::fromLines (text);
            const float lh = size * 1.15f;
            float y = r.getCentreY() - lh * (float) lines.size() * 0.5f;
            for (auto& l : lines) { drawText (g, l, { r.getX(), y, r.getWidth(), lh }, f, tc, Justification::centred, spacing); y += lh; }
        }
        else drawText (g, text, r, f, tc, Justification::centred, spacing);
    }
    void mouseEnter (const MouseEvent&) override { over = true; repaint(); }
    void mouseExit (const MouseEvent&) override { over = false; repaint(); }
    void mouseUp (const MouseEvent& e) override
    {
        if (e.mouseWasClicked() && getLocalBounds().contains (e.getPosition()) && onClick) onClick (e.mods);
    }
    void mouseDoubleClick (const MouseEvent&) override { if (onDouble) onDouble(); }

private:
    const Ctx& ctx;
    float size;
    theme::FontKind kind;
    bool over = false;
};

// ---- Faceplate knob, drawn like the mockup's SVG (150 x 150 units, scaled to the cell) ------------------
struct KnobCfg
{
    double lo = 0, hi = 1, def = 0, step = 0;     // step 0 = continuous
    bool discrete = false;                         // one tick per step, drag needs less travel
    std::vector<std::pair<double, String>> ticks;
};

class KnobView : public Component, public TipClient
{
public:
    // shown: the value to draw (a linked B knob shows A). begin/set/end: the editor applies the change (and mirrors it when Link is on).
    KnobView (KnobCfg c, std::function<double()> shownFn, std::function<void()> beginFn,
              std::function<void (double)> setFn, std::function<void()> endFn)
        : cfg (std::move (c)), shown (std::move (shownFn)), begin (std::move (beginFn)), set (std::move (setFn)), end (std::move (endFn))
    {
        setMouseCursor (MouseCursor::UpDownResizeCursor);
        setWantsKeyboardFocus (true);
        last = shown();
    }

    void refresh() { const double v = shown(); if (std::abs (v - last) > 1.0e-9) { last = v; repaint(); } }
    double value() const { return shown(); }

    void paint (Graphics& g) override
    {
        g.addTransform (AffineTransform::scale ((float) getWidth() / 150.0f));
        const float cx = 75.0f, cy = 75.0f;
        const double v = shown();
        auto angOf = [this] (double x) { return degreesToRadians ((float) (-135.0 + 270.0 * (x - cfg.lo) / (cfg.hi - cfg.lo))); };
        if (cfg.discrete)
        {
            for (double x = cfg.lo; x <= cfg.hi + 1.0e-9; x += 1.0)
            {
                const float a = angOf (x);
                g.setColour (Colour (0xfff4f5f7));
                g.drawLine (cx + std::sin (a) * 64.0f, cy - std::cos (a) * 64.0f, cx + std::sin (a) * 72.0f, cy - std::cos (a) * 72.0f, 2.0f);
            }
        }
        else
        {
            for (int i = 0; i <= 20; ++i)
            {
                const float a = degreesToRadians (-135.0f + (float) i * 13.5f);
                const bool major = i % 2 == 0;
                const float r1 = major ? 66.0f : 68.0f;
                g.setColour (major ? Colour (0xfff4f5f7) : Colour (0xff8c8f99));
                g.drawLine (cx + std::sin (a) * r1, cy - std::cos (a) * r1, cx + std::sin (a) * 72.0f, cy - std::cos (a) * 72.0f, major ? 1.8f : 1.0f);
            }
        }
        g.setFont (theme::font (theme::FontKind::semi, 13.0f));
        g.setColour (Colour (0xffc9cbd3));
        for (auto& tk : cfg.ticks)
        {
            const float a = angOf (tk.first);
            g.drawSingleLineText (tk.second, roundToInt (cx + std::sin (a) * 56.0f), roundToInt (cy - std::cos (a) * 56.0f + 4.0f), Justification::horizontallyCentred);
        }
        g.setColour (Colour (0xff0b0b0d));
        g.fillEllipse (cx - 42, cy - 42, 84, 84);
        g.setColour (Colours::black);
        g.drawEllipse (cx - 42, cy - 42, 84, 84, 1.0f);
        {
            Path ring; ring.addEllipse (cx - 40, cy - 40, 80, 80);
            Path dashes; const float d[2] = { 1.6f, 1.8f };
            PathStrokeType (4.0f, PathStrokeType::mitered, PathStrokeType::butt).createDashedStroke (dashes, ring, d, 2);
            g.setColour (Colour (0xff4a4c55));
            g.fillPath (dashes);
        }
        {
            Graphics::ScopedSaveState ss (g);
            g.addTransform (AffineTransform::rotation (angOf (v), cx, cy));
            ColourGradient grad (Colour (0xff5a5c66), 68.4f, 65.1f, Colour (0xff101114), 68.4f + 46.2f, 65.1f, true);
            grad.addColour (0.55, Colour (0xff2b2c33));
            g.setGradientFill (grad);
            g.fillEllipse (cx - 33, cy - 33, 66, 66);
            g.setColour (Colour (0xff5a5c66));
            g.drawEllipse (cx - 33, cy - 33, 66, 66, 1.0f);
            g.setColour (Colours::white.withAlpha (0.08f));
            g.drawEllipse (cx - 29, cy - 29, 58, 58, 1.0f);
            Path tri; tri.startNewSubPath (71, 46); tri.lineTo (75, 36); tri.lineTo (79, 46); tri.closeSubPath();
            g.setColour (theme::pink);
            g.fillPath (tri);
            g.setColour (Colour (0xfff4f5f7));
            g.drawLine (75, 75, 75, 48, 3.5f);
            g.fillEllipse (73.25f, 46.25f, 3.5f, 3.5f);
        }
        if (hasKeyboardFocus (false))
        {
            g.setColour (theme::selb);
            g.drawEllipse (1.0f, 1.0f, 148.0f, 148.0f, 2.0f);
        }
    }

    void mouseDown (const MouseEvent&) override { dragStart = shown(); acc = dragStart; begin(); grabKeyboardFocus(); dragging = true; }
    void mouseDrag (const MouseEvent& e) override
    {
        if (! dragging) return;
        const double rng = cfg.hi - cfg.lo;
        const double d = -(double) e.getDistanceFromDragStartY();
        acc = dragStart + d * (rng / (cfg.discrete ? 150.0 : 220.0)) * (e.mods.isShiftDown() ? 0.1 : 1.0);
        apply (acc);
    }
    void mouseUp (const MouseEvent&) override { if (dragging) { end(); dragging = false; } }
    void mouseDoubleClick (const MouseEvent&) override { begin(); apply (cfg.def); end(); }
    void mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& w) override
    {
        const float dy = w.isReversed ? -w.deltaY : w.deltaY;
        if (std::abs (dy) < 1.0e-6f) return;
        nudge ((dy > 0 ? 1 : -1) * stepSize (e.mods.isShiftDown(), 0.02));
    }
    bool keyPressed (const KeyPress& k) override
    {
        const bool shift = k.getModifiers().isShiftDown();
        if (k.isKeyCode (KeyPress::upKey) || k.isKeyCode (KeyPress::rightKey)) nudge (stepSize (shift, 0.01));
        else if (k.isKeyCode (KeyPress::downKey) || k.isKeyCode (KeyPress::leftKey)) nudge (-stepSize (shift, 0.01));
        else if (k.isKeyCode (KeyPress::homeKey)) { begin(); apply (cfg.lo); end(); }
        else if (k.isKeyCode (KeyPress::endKey)) { begin(); apply (cfg.hi); end(); }
        else return false;
        return true;
    }
    void focusGained (FocusChangeType) override { repaint(); }
    void focusLost (FocusChangeType) override { repaint(); }

private:
    double stepSize (bool fine, double frac) const
    {
        if (cfg.discrete) return cfg.step > 0 ? cfg.step : 1.0;
        const double s = (cfg.hi - cfg.lo) * (fine ? frac * 0.1 : frac);
        return cfg.step > 0 ? std::max (cfg.step, s) : s;
    }
    void nudge (double delta) { begin(); apply (shown() + delta); end(); }
    void apply (double v)
    {
        v = jlimit (cfg.lo, cfg.hi, v);
        if (cfg.step > 0) v = std::round (v / cfg.step) * cfg.step;
        set (jlimit (cfg.lo, cfg.hi, v));
        refresh();
    }

    KnobCfg cfg;
    std::function<double()> shown;
    std::function<void()> begin, end;
    std::function<void (double)> set;
    double last = 0, dragStart = 0, acc = 0;
    bool dragging = false;
};

// Value text rows under a knob, in the mono font: a bold part in ink and the rest dim. Not clickable.
class KnobValue : public Component
{
public:
    struct Line { String bold, rest; };
    KnobValue (const Ctx& c, std::vector<float> rowY) : ctx (c), ys (std::move (rowY)) { setInterceptsMouseClicks (false, false); }
    void setLines (std::vector<Line> l)
    {
        bool same = l.size() == lines.size();
        for (size_t i = 0; same && i < l.size(); ++i) same = l[i].bold == lines[i].bold && l[i].rest == lines[i].rest;
        if (! same) { lines = std::move (l); repaint(); }
    }
    void paint (Graphics& g) override
    {
        auto& t = ctx.t();
        const auto fb = theme::font (theme::FontKind::monoBold, 11.0f), fr = theme::font (theme::FontKind::mono, 11.0f);
        for (size_t i = 0; i < lines.size() && i < ys.size(); ++i)
        {
            const float wb = lines[i].bold.isEmpty() ? 0.0f : textWidth (fb, lines[i].bold);
            const float wr = lines[i].rest.isEmpty() ? 0.0f : textWidth (fr, lines[i].rest);
            const float x0 = (float) getWidth() * 0.5f - (wb + wr) * 0.5f;
            const int y = roundToInt (ys[i] + 14.0f * 0.5f + fr.getAscent() * 0.5f - 1.0f);
            g.setFont (fb); g.setColour (t.ink);
            if (wb > 0) g.drawSingleLineText (lines[i].bold, roundToInt (x0), y);
            g.setFont (fr); g.setColour (t.dim);
            if (wr > 0) g.drawSingleLineText (lines[i].rest, roundToInt (x0 + wb), y);
        }
    }
private:
    const Ctx& ctx;
    std::vector<float> ys;
    std::vector<Line> lines;
};

// A line of text that the editor updates (readouts, captions).
class TextView : public Component, public TipClient
{
public:
    enum class Tone { dim, title, ink };
    TextView (const Ctx& c, theme::FontKind k, float px, Tone tn, Justification j, float spacing = 0.0f, bool upperCase = false)
        : ctx (c), kind (k), size (px), tone (tn), just (j), spacingPx (spacing), upper (upperCase) { setInterceptsMouseClicks (false, false); }
    void setText (const String& s) { if (s != text) { text = s; repaint(); } }
    void paint (Graphics& g) override
    {
        auto& t = ctx.t();
        drawText (g, upper ? text.toUpperCase() : text, getLocalBounds().toFloat(), theme::font (kind, size),
                  tone == Tone::dim ? t.dim : tone == Tone::title ? t.title : t.ink, just, spacingPx);
    }
private:
    const Ctx& ctx;
    theme::FontKind kind;
    float size;
    Tone tone;
    Justification just;
    float spacingPx;
    bool upper;
    String text;
};

// Invisible area that only shows an info bubble (or reacts to a click).
class HitArea : public Component, public TipClient
{
public:
    std::function<void (const ModifierKeys&)> onClick;
    HitArea() { setMouseCursor (MouseCursor::PointingHandCursor); }
    void mouseUp (const MouseEvent& e) override { if (e.mouseWasClicked() && onClick) onClick (e.mods); }
};
} // namespace ui
