#pragma once
#include "Widgets.h"
#include "../core/Furchild.h"

namespace ui
{
// Everything the views draw, refreshed by the editor timer (message thread only).
struct Live
{
    float inPk = 0, outPk = 0;               // linear, decaying peak hold
    float grNow[2] {}, ocPk[2] {};           // dB (positive) and linear decaying peak of each channel's output before the ceiling
    float needle[2] {};                      // smoothed needle positions
    float lm = 1.0f;                         // Level Match gain, linear
    double inDb[2] {}, thKnob[2] { 2, 2 }, outDb = 2.0, mix = 100.0;
    int tc[2] { 2, 2 }, mode = 0, hp = 0, meterMode = 0;
    bool link = true, bypass = false, levelMatch = false;
    float histA[288] {}, histB[288] {};
    int hi = 0;
};

inline float l2d (float x) { return 20.0f * std::log10 (std::max (x, 1.0e-9f)); }

// Base of the display canvases: drawn in the mockup's canvas units, scaled into a bordered box.
class CanvasView : public Component, public TipClient
{
public:
    CanvasView (const Ctx& c, const Live& l, float cw, float ch) : ctx (c), live (l), cvW (cw), cvH (ch) {}
    void paint (Graphics& g) override
    {
        auto& t = ctx.t();
        auto r = getLocalBounds().toFloat();
        g.setColour (t.disp);
        g.fillRoundedRectangle (r, 3.0f);
        auto inner = r.reduced (1.0f);
        {
            Graphics::ScopedSaveState ss (g);
            g.reduceClipRegion (inner.toNearestInt());
            g.addTransform (AffineTransform::scale (inner.getWidth() / cvW, inner.getHeight() / cvH).translated (inner.getX(), inner.getY()));
            draw (g, t, cvW, cvH);
        }
        g.setColour (t.line);
        g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 1.0f);
    }
protected:
    virtual void draw (Graphics&, const theme::Look&, float w, float h) = 0;
    static void label (Graphics& g, const String& s, float x, float y, Justification j, Colour c)
    {
        g.setFont (theme::font (theme::FontKind::mono, 9.0f));
        g.setColour (c);
        g.drawSingleLineText (s, roundToInt (x), roundToInt (y), j);
    }
    static void dashedLine (Graphics& g, Line<float> ln, float a, float b, float w)
    {
        const float d[2] = { a, b };
        g.drawDashedLine (ln, d, 2, w);
    }
    const Ctx& ctx;
    const Live& live;
    float cvW, cvH;
};

// Needle meter. Reduce view: 0 to 20 dB of gain reduction. Level view: -30 to +3 dB.
class DialView : public CanvasView
{
public:
    DialView (const Ctx& c, const Live& l, int channel) : CanvasView (c, l, 210, 136), ch (channel) {}
protected:
    void draw (Graphics& g, const theme::Look& t, float, float) override
    {
        const Point<float> piv (105.0f, 130.0f);
        const float DR = 98.0f, SW = 56.0f;
        const int m = live.meterMode;
        const float mn = m ? -30.0f : 0.0f, mx = m ? 3.0f : 20.0f;
        auto ang = [=] (float v) { return -SW + 2.0f * SW * (jlimit (mn, mx, v) - mn) / (mx - mn); };
        auto ptAt = [=] (float a, float r) { return Point<float> (piv.x + std::sin (degreesToRadians (a)) * r, piv.y - std::cos (degreesToRadians (a)) * r); };
        auto arc = [&] (float v0, float v1, float r, Colour col, float lw)
        {
            Path p; p.addCentredArc (piv.x, piv.y, r, r, 0.0f, degreesToRadians (ang (v0)), degreesToRadians (ang (v1)), true);
            g.setColour (col); g.strokePath (p, PathStrokeType (lw));
        };
        if (m) { arc (-30, -12, DR + 5, theme::blue, 4); arc (-12, 0, DR + 5, theme::purple, 4); arc (0, 3, DR + 5, theme::red, 4); }
        else { arc (0, 12, DR + 5, t.accent, 4); arc (12, 20, DR + 5, theme::red, 4); }
        static const float labM[5] = { -30, -20, -10, 0, 3 }, labG[5] = { 0, 5, 10, 15, 20 };
        const float* labs = m ? labM : labG;
        for (float v = mn; v <= mx + 1.0e-4f; v += 1.0f)
        {
            bool major = false; for (int i = 0; i < 5; ++i) if (std::abs (labs[i] - v) < 1.0e-3f) major = true;
            const auto p0 = ptAt (ang (v), DR - (major ? 9.0f : 5.0f)), p1 = ptAt (ang (v), DR);
            g.setColour (t.dim); g.drawLine (p0.x, p0.y, p1.x, p1.y, major ? 1.6f : 1.0f);
        }
        for (int i = 0; i < 5; ++i)
        {
            const auto p = ptAt (ang (labs[i]), DR - 22.0f);
            label (g, String (m && labs[i] > 0 ? "+" : "") + String ((int) labs[i]), p.x, p.y + 3.0f, Justification::horizontallyCentred, t.ink);
        }
        g.setFont (theme::font (theme::FontKind::regular, 11.0f)); g.setColour (t.dim);
        g.drawSingleLineText (m ? "LEVEL dB" : "GAIN REDUCTION dB", 6, 14);
        const auto np = ptAt (ang (live.needle[ch]), DR - 4.0f);
        g.setColour (theme::pink); g.drawLine (piv.x, piv.y, np.x, np.y, 2.0f);
        g.setColour (Colour (0xff2a2c33)); g.fillEllipse (piv.x - 9, piv.y - 9, 18, 18);
        g.setColour (t.dim); g.drawEllipse (piv.x - 9, piv.y - 9, 18, 18, 1.0f);
    }
private:
    int ch;
};

// Input level (across) against output level (up).
class CurveView : public CanvasView
{
public:
    CurveView (const Ctx& c, const Live& l) : CanvasView (c, l, 240, 136) {}
protected:
    void draw (Graphics& g, const theme::Look& t, float w, float h) override
    {
        const float x0 = -60, x1 = 6, y0 = -60, y1 = 18;
        auto X = [=] (float v) { return 26.0f + (v - x0) / (x1 - x0) * (w - 32.0f); };
        auto Y = [=] (float v) { return h - 14.0f - (v - y0) / (y1 - y0) * (h - 22.0f); };
        for (float v : { -60.0f, -48.0f, -36.0f, -24.0f, -12.0f, 0.0f })
        {
            g.setColour (t.line); g.drawLine (X (v), 4, X (v), h - 14, 1.0f);
            label (g, String ((int) v), X (v), h - 4, Justification::horizontallyCentred, t.dim);
        }
        for (float v : { -48.0f, -24.0f, 0.0f, 12.0f })
        {
            g.setColour (t.line); g.drawLine (26, Y (v), w - 6, Y (v), 1.0f);
            label (g, String ((int) v), 23, Y (v) + 3, Justification::right, t.dim);
        }
        g.setColour (Colour (0xff4a4d55));
        dashedLine (g, { X (-60), Y (-60), X (6), Y (6) }, 3, 3, 1.0f);
        for (int k = live.link ? 0 : 1; k >= 0; --k)
        {
            const Colour col = k ? theme::pink : t.accent;
            const int s = live.link ? 0 : k;
            const double gin = live.inDb[s], thr = fur::thresholdDb (live.thKnob[s]);
            const float tx = (float) (thr - gin);
            if (tx > x0 && tx < x1)
            {
                g.setColour (col.withAlpha (0.55f));
                dashedLine (g, { X (tx), 4, X (tx), h - 14 }, 2, 3, 1.0f);
            }
            Path p; bool first = true;
            for (float v = x0; v <= x1 + 0.01f; v += 0.5f)
            {
                const float o = live.bypass ? v : (float) (v + gin - fur::law::grDb (v + gin - thr) + live.outDb);
                const Point<float> pt (X (v), Y (jlimit (y0, y1, o)));
                if (first) p.startNewSubPath (pt); else p.lineTo (pt);
                first = false;
            }
            g.setColour (col);
            g.strokePath (p, PathStrokeType (2.0f));
        }
        const float li = l2d (live.inPk), lo = l2d (live.outPk);
        if (std::isfinite (li) && li > -60.0f)
        {
            g.setColour (Colours::white);
            g.fillEllipse (X (jmin (x1, li)) - 4, Y (jlimit (y0, y1, lo)) - 4, 8, 8);
        }
    }
};

// Gain reduction of each channel over the last six seconds.
class HistView : public CanvasView
{
public:
    HistView (const Ctx& c, const Live& l) : CanvasView (c, l, 288, 136) {}
protected:
    void draw (Graphics& g, const theme::Look& t, float w, float h) override
    {
        auto Y = [=] (float d) { return 8.0f + d / 30.0f * (h - 22.0f); };
        for (int d = 0; d <= 30; d += 6)
        {
            g.setColour (t.line); g.drawLine (26, Y ((float) d), w - 4, Y ((float) d), 1.0f);
            label (g, "-" + String (d), 23, Y ((float) d) + 3, Justification::right, t.dim);
        }
        for (int k = 0; k < 2; ++k)
        {
            if (k == 1 && live.link) continue;
            const float* hs = k ? live.histB : live.histA;
            const Colour col = k ? theme::pink : t.accent;
            Path p;
            p.startNewSubPath (28, Y (0));
            for (int i = 0; i < 288; ++i)
                p.lineTo (28.0f + (float) i / 287.0f * (w - 32.0f), Y (jmin (30.0f, hs[(live.hi + i) % 288])));
            p.lineTo (w - 4, Y (0));
            p.closeSubPath();
            g.setColour (col.withAlpha (0.22f)); g.fillPath (p);
            g.setColour (col); g.strokePath (p, PathStrokeType (1.6f));
        }
    }
};

// Four valves that glow with the gain reduction.
class TubeView : public Component, public TipClient
{
public:
    TubeView (const Ctx& c, const Live& l) : ctx (c), live (l) {}
    void paint (Graphics& g) override
    {
        auto& t = ctx.t();
        g.fillAll (t.disp);
        for (int i = 0; i < 4; ++i)
        {
            const int c = i < 2 ? 0 : 1;
            const float x = 27.0f + (float) i * 53.0f, gr = live.grNow[live.link ? 0 : c];
            const float glow = 0.18f + 0.82f * jmin (1.0f, gr / 18.0f);
            ColourGradient rg (theme::amber.withAlpha (glow * 0.9f), x, 30, theme::amber.withAlpha (0.0f), x + 24.0f, 30, true);
            g.setGradientFill (rg);
            g.fillRect (x - 26, 2.0f, 52.0f, 48.0f);
            Path glass;
            glass.startNewSubPath (x - 10, 46); glass.lineTo (x - 10, 18); glass.quadraticTo (x - 10, 4, x, 4);
            glass.quadraticTo (x + 10, 4, x + 10, 18); glass.lineTo (x + 10, 46); glass.closeSubPath();
            g.setColour (t.dim); g.strokePath (glass, PathStrokeType (1.4f));
            g.setColour (theme::amber.withAlpha (0.35f + 0.65f * glow));
            g.drawLine (x - 4, 42, x - 4, 16, 2.0f); g.drawLine (x + 4, 42, x + 4, 16, 2.0f);
            g.setColour (Colour::fromFloatRGBA (1.0f, 210.0f / 255.0f, 120.0f / 255.0f, 0.3f + 0.7f * glow));
            g.fillRect (x - 6, 26.0f, 12.0f, 3.0f);
            g.setColour (Colour (0xff2a2c33));
            g.fillRect (x - 12, 46.0f, 24.0f, 5.0f);
        }
    }
private:
    const Ctx& ctx;
    const Live& live;
};

// Gain reduction ladder of one channel. Bounds are the mockup's ladder box; drawing uses faceplate coordinates.
class LadderView : public Component, public TipClient
{
public:
    LadderView (const Ctx& c, const Live& l, int channel, const layout::Rc& box, const layout::Led* leds, const layout::Rc* nums)
        : ctx (c), live (l), ch (channel), origin (box.x, box.y), led (leds), num (nums) {}
    void paint (Graphics& g) override
    {
        auto& t = ctx.t();
        g.addTransform (AffineTransform::translation (-origin.x, -origin.y));
        const float gr = live.grNow[ch];
        for (int i = 0; i < 12; ++i)
        {
            drawText (g, String (led[i].value), rc (num[i]), theme::font (theme::FontKind::mono, 10.0f), t.dim);
            const int v = led[i].value;
            const Colour c = v >= 16 ? theme::red : v >= 10 ? theme::orange : v >= 5 ? t.yellow : theme::green;
            drawLed (g, led[i].cx, led[i].cy, c, gr >= (float) v - 0.25f);
        }
    }
private:
    static void drawLed (Graphics& g, float cx, float cy, Colour c, bool on)
    {
        if (on)
        {
            ColourGradient glow (c.withAlpha (0.6f), cx, cy, c.withAlpha (0.0f), cx + 11.0f, cy, true);
            glow.addColour (6.0 / 11.0, c.withAlpha (0.45f));
            g.setGradientFill (glow);
            g.fillEllipse (cx - 11.0f, cy - 11.0f, 22.0f, 22.0f);
            g.setColour (c);
            g.fillEllipse (cx - 6.0f, cy - 6.0f, 12.0f, 12.0f);
            g.setColour (Colours::white.withAlpha (0.35f));
            g.drawEllipse (cx - 5.0f, cy - 5.0f, 10.0f, 10.0f, 1.0f);
        }
        else
        {
            g.setColour (Colour (0xff26282c));
            g.fillEllipse (cx - 6.0f, cy - 6.0f, 12.0f, 12.0f);
            g.setColour (Colours::black.withAlpha (0.55f));
            g.drawEllipse (cx - 5.5f, cy - 5.5f, 11.0f, 11.0f, 1.0f);
        }
    }
    const Ctx& ctx;
    const Live& live;
    int ch;
    Point<float> origin;
    const layout::Led* led;
    const layout::Rc* num;
};

// Input or output level strip, 38 x 300. Click to clear the clip marker. dB scale, neon blue, purple and pink.
class MeterView : public Component, public TipClient
{
public:
    MeterView (const Ctx& c) : ctx (c) {}
    float lin = 0.0f;
    bool latch = false;
    void update (float l)
    {
        const bool ch = std::abs (l - lin) > 1.0e-7f;
        lin = l;
        if (l2d (l) >= 0.0f && ! latch) { latch = true; repaint(); }
        if (ch) repaint();
    }
    void paint (Graphics& g) override
    {
        auto& t = ctx.t();
        const float w = (float) getWidth(), h = (float) getHeight(), mn = -60, mx = 6;
        auto y = [=] (float db) { return h - 6.0f - (jlimit (mn, mx, db) - mn) / (mx - mn) * (h - 12.0f); };
        g.fillAll (t.disp);
        const float db = l2d (lin), top = y (db);
        auto f = [=] (float d) { return (d - mn) / (mx - mn); };
        ColourGradient grad (theme::blue, 0.0f, y (mn), theme::red, 0.0f, y (mx), false);
        grad.addColour (f (-12.0f), theme::blue);
        grad.addColour (f (-12.0f) + 0.001, theme::purple);
        grad.addColour (f (0.0f), theme::purple);
        grad.addColour (f (0.0f) + 0.001, theme::red);
        g.setGradientFill (grad);
        g.fillRect (2.0f, top, 12.0f, y (mn) - top);
        g.setFont (theme::font (theme::FontKind::mono, 9.0f));
        for (float tk : { 0.0f, -6.0f, -12.0f, -24.0f, -36.0f, -48.0f })
        {
            const float yy = y (tk);
            g.setColour (t.dim);
            g.fillRect (14.0f, yy, 3.0f, 1.0f);
            g.drawSingleLineText (String ((int) tk), roundToInt (w - 1.0f), roundToInt (yy + 3.0f), Justification::right);
        }
        g.setColour (latch ? theme::red : Colour (0xff2a2c31));
        g.fillRect (2.0f, 0.0f, 12.0f, 4.0f);
    }
    void mouseDown (const MouseEvent&) override { latch = false; repaint(); }
private:
    const Ctx& ctx;
};
} // namespace ui
