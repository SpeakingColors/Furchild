// Furchild core: variable-mu style compressor engine. No JUCE needed (tests build it alone).
// Ported from the approved mockup engine (Furchild-v1.1.html), same law, same time constants.
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace fc
{
inline float d2l (float d) { return std::pow (10.0f, d * 0.05f); }
inline float l2d (float l) { return 20.0f * std::log10 (std::max (l, 1.0e-9f)); }
inline float clampf (float v, float a, float b) { return v < a ? a : v > b ? b : v; }

// Gain reduction law: ratio r(x) = 1 + 19 x/(x+24), softplus knee 5 dB wide, table of 361 points.
struct GrTable
{
    float t[361];
    GrTable()
    {
        for (int i = 0; i <= 360; ++i)
        {
            const double x = -30.0 + i * 0.25, s = 5.0 * std::log (1.0 + std::exp (x / 5.0));
            t[i] = (float) std::min (50.0, 0.95 * (s - 1.2 * std::log ((s + 1.2) / 1.2)));
        }
    }
};
inline float grFn (float over)
{
    static const GrTable g;
    if (over <= -30.0f) return 0.0f;
    if (over >= 60.0f) return g.t[360];
    const float f = (over + 30.0f) * 4.0f;
    const int i = (int) f;
    return g.t[i] + (g.t[i + 1] - g.t[i]) * (f - (float) i);
}

struct Tc { float att, rel; int autoKind; const char* txt; const char* rtxt; };
inline const Tc* tcTable()
{
    static const Tc t[6] = { { 0.2f, 0.3f, 0, "rel 0.3 s", "Release 0.3 s" },
                             { 0.2f, 0.8f, 0, "rel 0.8 s", "Release 0.8 s" },
                             { 0.4f, 2.0f, 0, "rel 2 s", "Release 2 s" },
                             { 0.8f, 5.0f, 0, "rel 5 s", "Release 5 s" },
                             { 0.4f, 2.0f, 5, "rel 2 to 10 s", "Release 2 to 10 s" },
                             { 0.2f, 0.3f, 6, "rel 0.3 to 25 s", "Release 0.3 to 25 s" } };
    return t;
}
inline float thToDb (float t) { return -t * 3.2f; }

struct P
{
    float gIn[2] { 0, 0 };   // input gain dB per channel
    float T[2] { -6.4f, -6.4f }; // threshold dB per channel
    int tc[2] { 2, 2 };
    int mode = 0;            // 0 left/right, 1 lat/vert
    bool link = true;
    float outDb = 0.0f, mix = 1.0f;
    int hp = 0;              // 0 off, 1 60 Hz, 2 120 Hz, 3 240 Hz
    bool bypass = false, levelMatch = false;
};

class Engine
{
public:
    Engine() { prepare (48000.0); }

    void prepare (double sampleRate)
    {
        fs = (float) sampleRate;
        sc = 1.0f - std::exp (-1.0f / (0.010f * fs));
        dcR = 1.0f - 6.283185f * 10.0f / fs;
        lmK = 1.0f - std::exp (-1.0f / (1.0f * fs));
        lmS = 1.0f - std::exp (-1.0f / (2.0f * fs));
        susUp = 1.0f - std::exp (-1.0f / (0.8f * fs));
        susDn = 1.0f - std::exp (-1.0f / (3.0f * fs));
        for (int i = 0; i < 6; ++i)
        {
            attK[i] = 1.0f - std::exp (-1.0f / (tcTable()[i].att * 0.001f * fs));
            relK[i] = 1.0f - std::exp (-1.0f / (tcTable()[i].rel * fs));
        }
        setP (p);
        reset();
        curIn[0] = tIn[0]; curIn[1] = tIn[1]; curOut = tOut;
    }
    void setP (const P& np)
    {
        p = np;
        tIn[0] = d2l (p.gIn[0]); tIn[1] = d2l (p.gIn[1]); tOut = d2l (p.outDb);
        static const float fcs[4] = { 0, 60, 120, 240 };
        const float fc = fcs[std::clamp (p.hp, 0, 3)];
        hpA = fc > 0 ? 1.0f - std::exp (-6.283185f * fc / fs) : 0.0f;
    }
    void resetLM() { lm = 1.0f; eD = eW = 0.0f; }
    void reset() { gr[0] = gr[1] = 0; sus[0] = sus[1] = 0; dcX[0] = dcX[1] = dcY[0] = dcY[1] = 0; lp[0] = lp[1] = 0; resetLM(); }

    int tcIndex (int c) const { return std::clamp (p.link ? p.tc[0] : p.tc[c], 0, 5); }
    float relCoef (int c) const
    {
        const int i = tcIndex (c);
        const int a = tcTable()[i].autoKind;
        if (! a) return relK[i];
        const float s = sus[c];
        float rt;
        if (a == 5) rt = 2.0f + 8.0f * s;
        else rt = s < 0.5f ? 0.3f + 9.7f * (s / 0.5f) : 10.0f + 15.0f * ((s - 0.5f) / 0.5f);
        return 1.0f - std::exp (-1.0f / (rt * fs));
    }
    void stepGR (int c, float target)
    {
        const float cur = gr[c];
        const float v = target > cur ? cur + (target - cur) * attK[tcIndex (c)] : cur + (target - cur) * relCoef (c);
        const bool up = cur > 2.0f;
        const float k = up ? susUp : susDn;
        sus[c] += ((up ? 1.0f : 0.0f) - sus[c]) * (up ? k : k * (cur < 0.5f ? 1.0f : 0.0f));
        gr[c] = v;
    }

    // In place. Stereo only.
    void process (float* L, float* R, int n)
    {
        float lv[2], x[2], y[2];
        float ip = 0, op = 0, g0 = 0, g1 = 0, oc[2] = { 0, 0 };
        for (int i = 0; i < n; ++i)
        {
            for (int c = 0; c < 2; ++c)
            {
                const float d = tIn[c] - curIn[c];
                curIn[c] += std::fabs (d) < 1e-7f ? d : d * sc;
            }
            { const float d = tOut - curOut; curOut += std::fabs (d) < 1e-7f ? d : d * sc; }
            const float d0 = L[i], d1 = R[i];
            ip = std::max (ip, std::max (std::fabs (d0), std::fabs (d1)));
            float s0 = d0, s1 = d1;
            if (p.mode == 1) { s0 = (d0 + d1) * 0.5f; s1 = (d0 - d1) * 0.5f; }
            x[0] = s0 * curIn[0]; x[1] = s1 * curIn[1];
            for (int c = 0; c < 2; ++c)
            {
                float d = x[c];
                if (hpA > 0) { lp[c] += hpA * (d - lp[c]); d -= lp[c]; }
                lv[c] = l2d (std::fabs (d));
            }
            if (p.link)
            {
                const float t = grFn (std::max (lv[0], lv[1]) - p.T[0]);
                stepGR (0, t); stepGR (1, t);
            }
            else { stepGR (0, grFn (lv[0] - p.T[0])); stepGR (1, grFn (lv[1] - p.T[1])); }
            g0 = std::max (g0, gr[0]); g1 = std::max (g1, gr[1]);
            for (int c = 0; c < 2; ++c)
            {
                float v = x[c] * d2l (-gr[c]);
                v = v + 0.04f * v * std::fabs (v);
                const float o = v - dcX[c] + dcR * dcY[c];
                dcX[c] = v; dcY[c] = o;
                y[c] = o * curOut;
                oc[c] = std::max (oc[c], std::fabs (y[c]));
            }
            float w0 = y[0], w1 = y[1];
            if (p.mode == 1) { w0 = y[0] + y[1]; w1 = y[0] - y[1]; }
            if (p.levelMatch)
            {
                eD += ((d0 * d0 + d1 * d1) * 0.5f - eD) * lmK;
                eW += ((w0 * w0 + w1 * w1) * 0.5f - eW) * lmK;
                if (eD > 1e-9f && eW > 1e-9f)
                    lm += (clampf (std::sqrt (eD / eW), 0.0625f, 16.0f) - lm) * lmS;
                w0 *= lm; w1 *= lm;
            }
            float o0 = p.mix >= 1.0f ? w0 : d0 * (1.0f - p.mix) + w0 * p.mix;
            float o1 = p.mix >= 1.0f ? w1 : d1 * (1.0f - p.mix) + w1 * p.mix;
            if (! std::isfinite (o0) || ! std::isfinite (o1)) { reset(); o0 = o1 = 0; }
            if (p.bypass) { o0 = d0; o1 = d1; }
            o0 = softClip (o0); o1 = softClip (o1);
            L[i] = o0; R[i] = o1;
            op = std::max (op, std::max (std::fabs (o0), std::fabs (o1)));
        }
        const float dec = std::pow (0.9f, (float) n / 512.0f);
        grPk[0] = g0; grPk[1] = g1; grNow[0] = gr[0]; grNow[1] = gr[1];
        inPk = std::max (ip, inPk * dec); outPk = std::max (op, outPk * dec);
        ocPk[0] = std::max (oc[0], ocPk[0] * dec); ocPk[1] = std::max (oc[1], ocPk[1] * dec);
    }

    static float softClip (float v)
    {
        return v > 0.9f ? 0.9f + 0.1f * std::tanh ((v - 0.9f) / 0.1f) : v < -0.9f ? -(0.9f + 0.1f * std::tanh ((-v - 0.9f) / 0.1f)) : v;
    }
    // Static transfer curve for the display (dB in -> dB out), same as the mockup.
    float curveOut (float v, int c) const
    {
        const float gi = p.gIn[c], T0 = p.link ? p.T[0] : p.T[c], gin = p.link ? p.gIn[0] : gi;
        return v + gin - grFn (v + gin - T0) + p.outDb;
    }

    P p;
    float grNow[2] {}, grPk[2] {}, inPk = 0, outPk = 0, ocPk[2] {}, lm = 1.0f;

private:
    float fs = 48000, sc = 0, dcR = 0, lmK = 0, lmS = 0, susUp = 0, susDn = 0, hpA = 0;
    float attK[6] {}, relK[6] {};
    float tIn[2] { 1, 1 }, tOut = 1, curIn[2] { 1, 1 }, curOut = 1;
    float gr[2] {}, sus[2] {}, dcX[2] {}, dcY[2] {}, lp[2] {}, eD = 0, eW = 0;
};
} // namespace fc
