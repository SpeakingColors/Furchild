#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <BinaryData.h>

<<<<<<< HEAD
// Colours and fonts of the faceplate. Values are the mockup's CSS tokens.
namespace theme
{
using juce::Colour;

struct Look
{
    Colour bg, plate, plate2, line, ink, dim, title, accent, disp, yellow;
};

inline const Look& look (char c)
{
    // Look C is the base token set with an amber accent (the default look).
    static const Look C { Colour (0xff0b0b0d), Colour (0xff16171a), Colour (0xff1e2024), Colour (0xff33363d), Colour (0xfff4f5f7),
                          Colour (0xffa9adb8), Colour (0xffe8e9ee), Colour (0xffFFA600), Colour (0xff06080a), Colour (0xffFFD100) };
=======
// Colours and fonts of the faceplate. Values are the mockup's CSS tokens (look C is the default).
namespace theme
{
using juce::Colour;
struct Look { Colour bg, plate, plate2, line, ink, dim, title, accent, disp, yellow; };
inline const Look& look (int i)
{
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))
    static const Look A { Colour (0xff161638), Colour (0xff1c1c4a), Colour (0xff2C2C62), Colour (0xff33403f), Colour (0xffe8e6e1),
                          Colour (0xff9a9690), Colour (0xffE1CBF4), Colour (0xffFFA600), Colour (0xff05060d), Colour (0xffFFA600) };
    static const Look B { Colour (0xff0b0b0d), Colour (0xff15161a), Colour (0xff1c1d22), Colour (0xff2b2d34), Colour (0xfff4f5f7),
                          Colour (0xffa9adb8), Colour (0xffE1CBF4), Colour (0xffA43DFF), Colour (0xff06080a), Colour (0xffFFD100) };
<<<<<<< HEAD
    return c == 'A' ? A : c == 'B' ? B : C;
}

// Fixed colours (the same in all three looks).
inline const Colour green  { 0xff02EBBA }, orange { 0xffF57F01 }, red { 0xffFF1383 }, blue { 0xff6CC9F5 },
                    purple { 0xffA43DFF }, sel { 0xff241238 }, selb { 0xffCD92FE }, pink { 0xffFF7BB1 }, lilac { 0xffE1CBF4 },
                    amber { 0xffFFA600 };

enum class FontKind { regular, semi, bold, brand, mono, monoBold };

=======
    static const Look C { Colour (0xff0b0b0d), Colour (0xff16171a), Colour (0xff1e2024), Colour (0xff33363d), Colour (0xfff4f5f7),
                          Colour (0xffa9adb8), Colour (0xffe8e9ee), Colour (0xffFFA600), Colour (0xff06080a), Colour (0xffFFD100) };
    return i == 0 ? A : i == 1 ? B : C;
}
inline const Colour red { 0xffFF1383 }, blue { 0xff6CC9F5 }, purple { 0xffA43DFF }, sel { 0xff241238 }, selb { 0xffCD92FE },
                    pink { 0xffFF7BB1 }, lilac { 0xffE1CBF4 }, green { 0xff02EBBA }, orange { 0xffF57F01 };
enum class FontKind { regular, semi, bold, serif, mono };
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))
inline juce::Font font (FontKind k, float px)
{
    static const auto regular = juce::Typeface::createSystemTypefaceFor (BinaryData::BarlowCondensedMedium_ttf, BinaryData::BarlowCondensedMedium_ttfSize);
    static const auto semi    = juce::Typeface::createSystemTypefaceFor (BinaryData::BarlowCondensedSemiBold_ttf, BinaryData::BarlowCondensedSemiBold_ttfSize);
    static const auto bold    = juce::Typeface::createSystemTypefaceFor (BinaryData::BarlowCondensedBold_ttf, BinaryData::BarlowCondensedBold_ttfSize);
<<<<<<< HEAD
    static const auto brand   = juce::Typeface::createSystemTypefaceFor (BinaryData::FrauncesBoldItalic_ttf, BinaryData::FrauncesBoldItalic_ttfSize);
    static const auto mono    = juce::Typeface::createSystemTypefaceFor (BinaryData::JetBrainsMonoRegular_ttf, BinaryData::JetBrainsMonoRegular_ttfSize);
    static const auto monoB   = juce::Typeface::createSystemTypefaceFor (BinaryData::JetBrainsMonoSemiBold_ttf, BinaryData::JetBrainsMonoSemiBold_ttfSize);
    const auto& tf = k == FontKind::regular ? regular : k == FontKind::semi ? semi : k == FontKind::bold ? bold : k == FontKind::brand ? brand
                   : k == FontKind::monoBold ? monoB : mono;
    return juce::Font (juce::FontOptions (tf).withPointHeight (px));
=======
    static const auto serif   = juce::Typeface::createSystemTypefaceFor (BinaryData::LiberationSerifBoldItalic_ttf, BinaryData::LiberationSerifBoldItalic_ttfSize);
    static const auto mono    = juce::Typeface::createSystemTypefaceFor (BinaryData::JetBrainsMonoRegular_ttf, BinaryData::JetBrainsMonoRegular_ttfSize);
    const auto& tf = k == FontKind::regular ? regular : k == FontKind::semi ? semi : k == FontKind::bold ? bold : k == FontKind::serif ? serif : mono;
    // CSS pixel size (em) -> JUCE height in pixels: JUCE's height is ascent + descent (1.2 em Barlow Condensed, 1.32 em JetBrains Mono, 1.107 em Liberation Serif).
    const float f = k == FontKind::mono ? 1.32f : k == FontKind::serif ? 1.107f : 1.2f;
    return juce::Font (juce::FontOptions (tf).withHeight (px * f));
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))
}
} // namespace theme
