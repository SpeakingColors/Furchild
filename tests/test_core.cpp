// Furchild core tests. Build and run with: sh tests/run.sh
// Everything below is measured on the compiled core (no JUCE). Hardware comparisons are not possible here, so the
// targets are the design values of the approved mockup (ratio law, time constants, ranges); see the manual.
#include "../src/core/Furchild.h"
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

using namespace fur;
static int fails = 0;
#define CHECK(cond, ...) do { if (! (cond)) { ++fails; std::printf ("FAIL: "); std::printf (__VA_ARGS__); std::printf ("\n"); } else { std::printf ("ok:   "); std::printf (__VA_ARGS__); std::printf ("\n"); } } while (0)

static const double PI = 3.14159265358979323846;
static double FS = 48000.0;

static Compressor make (const Params& p, double fs = 48000.0, bool os = true)
{
    FS = fs; Compressor c; c.setParams (p); c.prepare (fs, os); c.setParams (p); return c;
}
static std::vector<float> sine (double f, double amp, int n, double fs = FS)
{
    std::vector<float> v ((size_t) n);
    for (int i = 0; i < n; ++i) v[(size_t) i] = (float) (amp * std::sin (2.0 * PI * f * i / fs));
    return v;
}
static double rms (const std::vector<float>& x, size_t a, size_t b)
{
    double s = 0; for (size_t i = a; i < b; ++i) s += (double) x[i] * x[i];
    return std::sqrt (s / (double) (b - a));
}
static double db (double x) { return 20.0 * std::log10 (std::max (x, 1e-12)); }

// steady-state output level (dB, RMS of a sine) for an input level in dB RMS-equivalent peak
static double steadyOutDb (Params p, double inPeakDb, double f = 1000.0, double seconds = 6.0)
{
    auto c = make (p);
    const int n = (int) (seconds * FS);
    auto x = sine (f, std::pow (10.0, inPeakDb / 20.0), n);
    std::vector<float> y = x;
    c.process (y.data(), nullptr, n);
    return db (rms (y, (size_t) (n * 3 / 4), (size_t) n) * std::sqrt (2.0));
}

// radix-2 FFT magnitude spectrum (power) of a real signal
static std::vector<double> power (const std::vector<float>& x, size_t start, size_t n)
{
    std::vector<std::complex<double>> a (n);
    for (size_t i = 0; i < n; ++i) a[i] = (double) x[start + i] * (0.5 - 0.5 * std::cos (2 * PI * (double) i / (double) n)); // Hann
    for (size_t i = 1, j = 0; i < n; ++i)
    {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap (a[i], a[j]);
    }
    for (size_t len = 2; len <= n; len <<= 1)
    {
        const auto w = std::polar (1.0, -2 * PI / (double) len);
        for (size_t i = 0; i < n; i += len)
        {
            std::complex<double> wk = 1;
            for (size_t k = 0; k < len / 2; ++k)
            {
                const auto u = a[i + k], v = a[i + k + len / 2] * wk;
                a[i + k] = u + v; a[i + k + len / 2] = u - v; wk *= w;
            }
        }
    }
    std::vector<double> p (n / 2);
    for (size_t i = 0; i < n / 2; ++i) p[i] = std::norm (a[i]);
    return p;
}

// Energy not on a true harmonic of f0 (harmonics below Nyquist only, so folded ones count as alias), relative to the fundamental, in dB.
static double aliasDb (const std::vector<float>& y, double f0, double fs, size_t start, size_t n)
{
    const auto p = power (y, start, n);
    const double binHz = fs / (double) n;
    std::vector<char> harm (p.size(), 0);
    for (int h = 1; h < 400; ++h)
    {
        const double f = h * f0;
        if (f >= fs / 2) break;   // a harmonic above Nyquist is not a harmonic any more: whatever lands below is alias
        const long b = std::lround (f / binHz);
        for (long k = b - 6; k <= b + 6; ++k) if (k >= 0 && k < (long) p.size()) harm[(size_t) k] = 1;
    }
    double fund = 0, al = 0;
    const long b1 = std::lround (f0 / binHz);
    for (long k = b1 - 6; k <= b1 + 6; ++k) fund += p[(size_t) k];
    for (size_t k = 8; k < p.size(); ++k) if (! harm[k]) al += p[k];
    return 10.0 * std::log10 (std::max (al, 1e-30) / fund);
}

int main()
{
    // ---------------------------------------------------------------- ranges and clamping
    {
        Params p; p.inDb[0] = 99; p.thKnob[0] = -5; p.tc[0] = 77; p.outDb = 1e9; p.mix = 5; p.hp = 9; p.link = true;
        auto c = make (p);
        std::vector<float> x = sine (1000, 0.5, 4800), y = x;
        c.process (y.data(), nullptr, (int) y.size());
        bool ok = true; float mx = 0; for (float v : y) { ok = ok && std::isfinite (v) && std::fabs (v) <= 1.5f; mx = std::max (mx, std::fabs (v)); }
        CHECK (ok, "out of range parameters are clamped, output stays finite (peak %.2f)", mx);
        const auto& q = c.params();
        CHECK (q.inDb[0] == 24 && q.thKnob[0] == 0 && q.tc[0] == 5 && q.outDb == 12 && q.mix == 1 && q.hp == 3, "clamped values: in 24, th 0, tc 6, out +12, mix 1, hp 240");
        Params nanp; nanp.inDb[0] = NAN; nanp.outDb = NAN; nanp.mix = NAN; c.setParams (nanp);
        CHECK (c.params().inDb[0] == 0 && c.params().outDb == 0 && c.params().mix == 1, "NaN parameters fall back to safe values");
    }
    // ---------------------------------------------------------------- level calibration
    {
        Params p; p.outDb = 0; p.thKnob[0] = p.thKnob[1] = 10;  // threshold -32 dB, far above a -60 dB signal
        const double o = steadyOutDb (p, -60.0);
        CHECK (std::fabs (o + 60.0) < 0.1, "unity: a -60 dB sine comes out at %.2f dB with Input 0, Output 0 (no reduction far below threshold)", o);
        p.inDb[0] = p.inDb[1] = 12; p.link = false; p.inDb[1] = 12;
        const double o2 = steadyOutDb (p, -60.0);
        CHECK (std::fabs (o2 + 48.0) < 0.15, "Input +12 dB adds 12 dB below threshold (%.2f dB out)", o2);
        Params q; q.outDb = 6; q.thKnob[0] = q.thKnob[1] = 10;
        CHECK (std::fabs (steadyOutDb (q, -60.0) + 54.0) < 0.1, "Output +6 dB adds 6 dB");
    }
    // ---------------------------------------------------------------- variable mu: the ratio rises with level
    {
        Params p; p.outDb = 0; p.thKnob[0] = p.thKnob[1] = 5;   // threshold -16 dB
        const double T = thresholdDb (5.0);
        double prev = 0; bool rising = true;
        std::printf ("      measured local ratio (input level step 3 dB) against level over threshold:\n");
        for (double over : { -12.0, -3.0, 3.0, 12.0, 24.0, 36.0 })
        {
            const double a = steadyOutDb (p, T + over - 1.5, 1000.0, 5.0), b = steadyOutDb (p, T + over + 1.5, 1000.0, 5.0);
            const double ratio = 3.0 / (b - a);
            std::printf ("      over %+5.0f dB: ratio %5.2f : 1 (design %5.2f)\n", over, ratio, law::ratioAt (over));
            if (ratio < prev - 0.15) rising = false;
            prev = ratio;
            if (over <= -12) CHECK (ratio < 1.35, "ratio near 1:1 well below threshold (%.2f)", ratio);
            if (over == 24) CHECK (ratio > 7.0 && ratio < 13.0, "about 10:1 at 24 dB over (%.2f)", ratio);
            if (over == 36) CHECK (ratio > 9.0 && ratio < 17.0, "ratio keeps rising at 36 dB over (%.2f)", ratio);
        }
        CHECK (rising, "the compression ratio rises with level (variable mu)");
        // gain reduction reading: the needle value follows the law
        auto c = make (p);
        std::vector<float> x = sine (1000, std::pow (10.0, (T + 12.0) / 20.0), 96000), y = x;
        c.process (y.data(), nullptr, (int) y.size());
        const double expected = law::grDb (12.0);
        CHECK (std::fabs (c.grNow (0) - expected) < 0.6, "gain reduction meter %.2f dB against the law %.2f dB at 12 dB over", c.grNow (0), expected);
    }
    // ---------------------------------------------------------------- time constants (measured)
    {
        // DC step at the detector: attack is the time to 63 % of the final reduction
        const double want[4] = { 0.2, 0.2, 0.4, 0.8 };
        for (int tc = 0; tc < 4; ++tc)
        {
            Params p; p.tc[0] = p.tc[1] = tc; p.thKnob[0] = p.thKnob[1] = 5; p.inDb[0] = 0; p.outDb = 0;
            const double fs = 192000.0;
            auto c = make (p, fs);
            float v = (float) std::pow (10.0, (thresholdDb (5.0) + 20.0) / 20.0);
            double final = 0; { auto d = make (p, fs); for (int i = 0; i < 400000; ++i) { float a = v; d.process (&a, nullptr, 1); } final = d.grNow (0); }
            int t63 = -1; for (int i = 0; i < 400000 && t63 < 0; ++i) { float a = v; c.process (&a, nullptr, 1); if (c.grNow (0) >= 0.632 * final) t63 = i; }
            const double ms = 1000.0 * t63 / fs;
            CHECK (std::fabs (ms - want[tc]) / want[tc] < 0.12, "TC %d attack (time to 63%%): %.3f ms, design %.2f ms", tc + 1, ms, want[tc]);
        }
        // release: time constant from the first 100 ms of the decay
        const double wantR[4] = { 0.3, 0.8, 2.0, 5.0 };
        for (int tc = 0; tc < 4; ++tc)
        {
            Params p; p.tc[0] = p.tc[1] = tc; p.thKnob[0] = p.thKnob[1] = 5;
            auto c = make (p);
            float v = (float) std::pow (10.0, (thresholdDb (5.0) + 20.0) / 20.0);
            for (int i = 0; i < 48000 * 2; ++i) { float a = v; c.process (&a, nullptr, 1); }
            const double g0 = c.grNow (0);
            for (int i = 0; i < 4800; ++i) { float a = 0.0f; c.process (&a, nullptr, 1); }
            const double tau = -0.1 / std::log (c.grNow (0) / g0);
            CHECK (std::fabs (tau - wantR[tc]) / wantR[tc] < 0.06, "TC %d release: %.2f s, design %.1f s", tc + 1, tau, wantR[tc]);
        }
        // program dependent release: short peak against long work
        for (int tcIdx : { 4, 5 })
        {
            double tauShort = 0, tauLong = 0;
            for (int longRun = 0; longRun < 2; ++longRun)
            {
                Params p; p.tc[0] = p.tc[1] = tcIdx; p.thKnob[0] = p.thKnob[1] = 5;
                auto c = make (p);
                float v = (float) std::pow (10.0, (thresholdDb (5.0) + 20.0) / 20.0);
                const int hold = longRun ? 48000 * 12 : 480;   // 10 ms against 12 s
                const int pre = longRun ? 0 : 0;
                (void) pre;
                for (int i = 0; i < hold; ++i) { float a = v; c.process (&a, nullptr, 1); }
                const double g0 = c.grNow (0);
                for (int i = 0; i < 240; ++i) { float a = 0.0f; c.process (&a, nullptr, 1); }
                const double tau = -0.005 / std::log (c.grNow (0) / g0);
                (longRun ? tauLong : tauShort) = tau;
            }
            std::printf ("      TC %d release: at the start of the release after a 10 ms peak %.2f s, after 12 s of work %.2f s\n", tcIdx + 1, tauShort, tauLong);
            CHECK (tauLong > tauShort * 2.0, "TC %d release stretches with sustained reduction (%.2f s to %.2f s)", tcIdx + 1, tauShort, tauLong);
            if (tcIdx == 4) CHECK (tauShort > 1.9 && tauShort < 2.6 && tauLong > 7.0 && tauLong < 10.5, "TC 5 runs about 2 s to about 10 s");
            if (tcIdx == 5) CHECK (tauShort > 0.25 && tauShort < 0.6 && tauLong > 6.0 && tauLong < 27.0, "TC 6 runs from a fraction of a second towards 25 s");
        }
    }
    // ---------------------------------------------------------------- link and Lat / Vert
    {
        Params p; p.thKnob[0] = p.thKnob[1] = 5; p.outDb = 0;
        std::vector<float> l = sine (1000, 0.7, 48000), r = sine (1000, 0.02, 48000);
        { auto c = make (p); auto a = l, b = r; c.process (a.data(), b.data(), 48000);
          CHECK (std::fabs (c.grNow (0) - c.grNow (1)) < 1e-9 && c.grNow (0) > 3.0, "Link: both channels reduce together (%.2f, %.2f dB)", c.grNow (0), c.grNow (1)); }
        { p.link = false; auto c = make (p); auto a = l, b = r; c.process (a.data(), b.data(), 48000);
          CHECK (c.grNow (0) > 3.0 && c.grNow (1) < 0.01, "Link off: channels are independent (%.2f, %.2f dB)", c.grNow (0), c.grNow (1)); }
        { p.link = false; p.mode = 1; auto c = make (p); auto a = l, b = l; c.process (a.data(), b.data(), 48000);
          CHECK (c.grNow (0) > 3.0 && c.grNow (1) < 0.01, "Lat / Vert: a mono signal is all middle (reduction %.2f dB, side %.2f dB)", c.grNow (0), c.grNow (1));
          double dmax = 0; for (size_t i = 24000; i < 48000; ++i) dmax = std::max (dmax, (double) std::fabs (a[i] - b[i]));
          CHECK (dmax < 1e-6, "Lat / Vert: the sides stay empty, left equals right"); }
        { p.link = false; p.mode = 1; auto c = make (p); std::vector<float> a = l, b = l; for (auto& v : b) v = -v; c.process (a.data(), b.data(), 48000);
          CHECK (c.grNow (1) > 3.0 && c.grNow (0) < 0.01, "Lat / Vert: an out-of-phase signal is all side (%.2f dB on the side channel)", c.grNow (1)); }
    }
    // ---------------------------------------------------------------- Level Match
    {
        Params p; p.thKnob[0] = p.thKnob[1] = 2; p.inDb[0] = 12; p.outDb = -6; p.levelMatch = true; p.tc[0] = 1;
        auto c = make (p);
        std::mt19937 rng (3); std::normal_distribution<float> nd (0.f, 0.05f);
        const int n = 48000 * 20; std::vector<float> l ((size_t) n), r;
        for (auto& v : l) v = nd (rng);
        r = l; auto in = l;
        c.process (l.data(), r.data(), n);
        const double dIn = db (rms (in, (size_t) n - 96000, (size_t) n)), dOut = db (rms (l, (size_t) n - 96000, (size_t) n));
        CHECK (std::fabs (dIn - dOut) < 0.6, "Level Match: output %.2f dB against input %.2f dB after settling", dOut, dIn);
        p.levelMatch = false; auto c2 = make (p); auto l2 = in, r2 = in; c2.process (l2.data(), r2.data(), n);
        std::printf ("      without Level Match the same settings give %.2f dB\n", db (rms (l2, (size_t) n - 96000, (size_t) n)));
    }
    // ---------------------------------------------------------------- Mix, dry path, latency, responsiveness
    {
        Params p; p.mix = 0.0; p.thKnob[0] = p.thKnob[1] = 0;
        auto c = make (p);
        const int L = c.latencySamples();
        auto x = sine (440, 1.0, 9600), y = x; c.process (y.data(), nullptr, 9600);
        bool exact = true; for (size_t i = (size_t) L; i < x.size(); ++i) exact = exact && y[i] == x[i - (size_t) L];
        CHECK (exact, "Mix 0 returns the input bit for bit (delayed by the %d samples of latency), even above the ceiling", L);
        CHECK (L == 16, "reported latency is 16 samples at 48 kHz");
        CHECK (make (p, 96000.0).latencySamples() == 0 && make (p, 192000.0).latencySamples() == 0 && make (p, 44100.0).latencySamples() == 16, "latency is 16 samples at 44.1 and 48 kHz and 0 at 88.2 kHz and above");
        Params pb; pb.thKnob[0] = pb.thKnob[1] = 0; pb.bypass = true; auto cb = make (pb); cb.process (x.data(), nullptr, 0);
        { auto z = sine (333, 1.0, 9600), zr = z; auto in = z; auto b2 = make (pb); b2.process (z.data(), zr.data(), 9600);
          bool bx = true; for (size_t i = (size_t) L + 1000; i < z.size(); ++i) bx = bx && z[i] == in[i - (size_t) L] && zr[i] == in[i - (size_t) L];
          CHECK (bx, "bypass is the input bit for bit, delayed by the same latency"); }
        { Params pa = pb; pa.bypass = false; auto z = sine (333, 0.9, 24000), zr = z; auto cc = make (pa); cc.process (z.data(), zr.data(), 12000);
          Params pc = pa; pc.bypass = true; cc.setParams (pc); auto z2 = sine (333, 0.9, 24000); std::vector<float> a (z2.begin() + 12000, z2.end()), ar = a; cc.process (a.data(), ar.data(), 12000);
          double mxStep = 0; for (size_t i = 1; i < a.size(); ++i) mxStep = std::max (mxStep, (double) std::fabs (a[i] - a[i - 1]));
          CHECK (mxStep < 0.1, "switching bypass on is a smooth crossfade (largest step between samples %.3f)", mxStep); }
        Params q; q.thKnob[0] = q.thKnob[1] = 10; q.outDb = 0; auto d = make (q);
        std::vector<float> imp (256, 0.0f); imp[100] = 0.001f; d.process (imp.data(), nullptr, 256);
        size_t pk = 0; for (size_t i = 0; i < imp.size(); ++i) if (std::fabs (imp[i]) > std::fabs (imp[pk])) pk = i;
        CHECK ((int) pk == 100 + d.latencySamples(), "measured latency: an impulse at 100 comes out at %zu (reported %d)", pk, d.latencySamples());
        // knob to sound: step Input from 0 to +12 dB under a steady -50 dB sine; when does the output change?
        Params a; a.thKnob[0] = a.thKnob[1] = 10; a.outDb = 0; auto e = make (a);
        auto s = sine (1000, 0.003, 9600); auto ref = s; auto f = make (a);
        e.process (ref.data(), nullptr, 4800);
        f.process (s.data(), nullptr, 4800);
        Params b = a; b.inDb[0] = b.inDb[1] = 12; e.setParams (b); f.setParams (b);
        std::vector<float> s2 (s.begin() + 4800, s.end()), r2 = s2; // continue the same sine on both
        std::vector<float> keep = sine (1000, 0.003, 9600); std::vector<float> t1 (keep.begin() + 4800, keep.end()), t0 = t1;
        Params a0 = a; auto g = make (a0); { std::vector<float> w (keep.begin(), keep.begin() + 4800); g.process (w.data(), nullptr, 4800); }
        g.process (t0.data(), nullptr, 4800);        // never changed
        { std::vector<float> w (keep.begin(), keep.begin() + 4800); auto h = make (a0); h.process (w.data(), nullptr, 4800); h.setParams (b); h.process (t1.data(), nullptr, 4800); }
        int first = -1; for (int i = 0; i < 4800 && first < 0; ++i) if (std::fabs (t1[(size_t) i] - t0[(size_t) i]) > 1e-6) first = i;
        const double fin = rms (t1, 4000, 4800), start = rms (t0, 4000, 4800);
        int t90 = -1; { auto h = make (a0); std::vector<float> w (keep.begin(), keep.begin() + 4800); h.process (w.data(), nullptr, 4800); h.setParams (b);
            std::vector<float> u (keep.begin() + 4800, keep.end());
            for (int blk = 0; blk < 48 && t90 < 0; ++blk) { h.process (u.data() + blk * 100, nullptr, 100); const double lv = rms (std::vector<float> (u.begin() + blk * 100, u.begin() + blk * 100 + 100), 0, 100);
                if (lv >= start + 0.9 * (fin - start)) t90 = (blk + 1) * 100; } }
        CHECK (first >= 0 && first <= make (a).latencySamples() + 1, "knob to sound: the output starts to change %d samples after the knob moves (latency %d)", first, make (a).latencySamples());
        std::printf ("      Input step 0 to +12 dB reaches 90%% of its end level after about %d samples (%.1f ms); smoothing time 10 ms\n", t90, 1000.0 * t90 / 48000);
        CHECK (t90 > 0 && 1000.0 * t90 / 48000 < 30.0, "knob to sound: 90%% of the change within 30 ms");
    }
    // ---------------------------------------------------------------- aliasing of the nonlinear stages
    {
        const size_t N = 32768;
        std::printf ("      alias energy (not on a true harmonic) relative to the fundamental, oversampling off and on:\n");
        struct Case { const char* what; double amp; double outDb; double limit; };
        for (auto cs : { Case { "tube stage, 0.9 peak", 0.9, 0.0, -60.0 }, Case { "ceiling just working (peak 1.3 out)", 0.5, 8.5, -20.0 }, Case { "ceiling hard driven (0.5 peak into +12 dB)", 0.5, 12.0, -19.0 } })
            for (double f0k : { 3001.0, 7001.0, 11003.0, 15013.0 })
            {
                double f0 = std::round (f0k / (FS / (double) N)) * (FS / (double) N);
                double res[2];
                for (int os = 0; os < 2; ++os)
                {
                    Params p; p.outDb = cs.outDb; p.inDb[0] = p.inDb[1] = 0; p.thKnob[0] = p.thKnob[1] = 0; p.tc[0] = p.tc[1] = 3;
                    auto c = make (p, 48000.0, os == 1);
                    auto y = sine (f0, cs.amp, (int) (N + 16384), FS);
                    c.process (y.data(), nullptr, (int) y.size());
                    res[os] = aliasDb (y, f0, FS, 16384, N);
                }
                std::printf ("      %s, %.0f Hz: %.1f dB without, %.1f dB with 2x oversampling\n", cs.what, f0, res[0], res[1]);
                CHECK (res[1] < cs.limit && (res[0] < -70.0 ? res[1] <= res[0] + 0.5 : res[1] <= res[0] - 3.0), "alias below %.0f dB and clearly better than without oversampling (the floor near -75 dB is gain-reduction ripple, not aliasing) (%s, %.0f Hz): %.1f dB", cs.limit, cs.what, f0, res[1]);
            }
    }
    // ---------------------------------------------------------------- tube colour amount (measured)
    {
        Params p; p.outDb = 0; p.thKnob[0] = p.thKnob[1] = 0;
        auto c = make (p);
        const size_t N = 32768; const double f0 = std::round (1000.0 / (FS / (double) N)) * (FS / (double) N);
        auto y = sine (f0, 0.7, (int) (N + 16384)); c.process (y.data(), nullptr, (int) y.size());
        const auto P = power (y, 16384, N); const double bin = FS / (double) N;
        auto at = [&] (double f) { double s = 0; const long b = std::lround (f / bin); for (long k = b - 4; k <= b + 4; ++k) s += P[(size_t) k]; return s; };
        const double h2 = 10 * std::log10 (at (2 * f0) / at (f0)), h3 = 10 * std::log10 (at (3 * f0) / at (f0));
        std::printf ("      tube colour at 0.7 peak (threshold 0, reduction %.1f dB): 2nd harmonic %.1f dB, 3rd harmonic %.1f dB\n", c.grNow (0), h2, h3);
        CHECK (h3 < -35.0 && h3 > -70.0 && h2 < h3 - 20.0, "tube colour is a gentle odd harmonic (3rd at %.1f dB, 2nd far below)", h3);
    }
    // ---------------------------------------------------------------- stability
    {
        for (double fs : { 44100.0, 48000.0, 88200.0, 96000.0, 192000.0 })
        {
            Params p; p.thKnob[0] = p.thKnob[1] = 5; p.inDb[0] = 24; p.tc[0] = 5;
            auto c = make (p, fs);
            std::vector<float> z ((size_t) fs, 0.0f), zr = z; c.process (z.data(), zr.data(), (int) fs);
            bool zero = true; for (size_t i = 0; i < z.size(); ++i) zero = zero && z[i] == 0.0f && zr[i] == 0.0f;
            CHECK (zero, "%.0f Hz: silence stays exactly silent", fs);
            std::vector<float> dc ((size_t) fs * 2, 1.0f), dcr = dc; c.process (dc.data(), dcr.data(), (int) dc.size());
            bool ok = true; for (float v : dc) ok = ok && std::isfinite (v) && std::fabs (v) < 1.5f;
            CHECK (ok && std::fabs (dc.back()) < 0.01f, "%.0f Hz: full scale DC is finite and the DC blocker brings it to %.4f", fs, dc.back());
            std::vector<float> sq ((size_t) fs); for (size_t i = 0; i < sq.size(); ++i) sq[i] = (i / 24) % 2 ? 1.0f : -1.0f;
            auto sr = sq; c.process (sq.data(), sr.data(), (int) sq.size());
            ok = true; for (float v : sq) ok = ok && std::isfinite (v) && std::fabs (v) < 1.5f;
            CHECK (ok, "%.0f Hz: full scale square wave stays finite and bounded", fs);
        }
        Params p; auto c = make (p);
        std::vector<float> x (4800, 0.2f); x[100] = NAN; x[200] = INFINITY; x[300] = -INFINITY;
        auto xr = x; c.process (x.data(), xr.data(), (int) x.size());
        bool ok = true; for (size_t i = 0; i < x.size(); ++i) ok = ok && std::isfinite (x[i]) && std::isfinite (xr[i]);
        CHECK (ok, "NaN and infinity in the input never reach the output");
        std::vector<float> t = sine (1000, 0.1, 4800), tr = t; c.process (t.data(), tr.data(), 4800);
        CHECK (rms (t, 2400, 4800) > 0.01, "the compressor keeps working after bad input");
        // random parameters for a long time, hot noise
        std::mt19937 rng (11); std::uniform_real_distribution<float> u (-1.f, 1.f);
        auto q = make (p); bool fine = true; std::vector<float> a (512), b (512);
        for (int blk = 0; blk < 3000 && fine; ++blk)
        {
            Params r; for (int k = 0; k < 2; ++k) { r.inDb[k] = std::floor (24 * std::fabs (u (rng))); r.thKnob[k] = 10 * std::fabs (u (rng)); r.tc[k] = (int) (6 * std::fabs (u (rng))); }
            r.mode = u (rng) > 0; r.link = u (rng) > 0; r.outDb = 12 * u (rng); r.mix = std::fabs (u (rng)); r.hp = (int) (4 * std::fabs (u (rng))); r.levelMatch = u (rng) > 0.3;
            q.setParams (r);
            for (auto& v : a) v = u (rng) * (blk % 7 == 0 ? 1.0f : 0.3f);
            b = a; q.process (a.data(), b.data(), 512);
            for (size_t i = 0; i < 512; ++i) fine = fine && std::isfinite (a[i]) && std::isfinite (b[i]) && std::fabs (a[i]) < 4.0f && std::fabs (b[i]) < 4.0f;
        }
        CHECK (fine, "3000 blocks of hot noise with random parameters (modes, links, time constants) stay finite and bounded");
    }
    std::printf (fails == 0 ? "ALL CORE TESTS PASSED\n" : "%d CORE TESTS FAILED\n", fails);
    return fails == 0 ? 0 : 1;
}
