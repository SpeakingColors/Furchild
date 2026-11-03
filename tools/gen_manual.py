#!/usr/bin/env python3
<<<<<<< HEAD
"""Build docs/manual.html and docs/manual/tour.png from the screenshot of the real window (docs/screenshot.png)
and the mockup layout data (docs/mockup/layout.json). CI turns manual.html into the PDF with headless Chromium.
Needs Pillow. Usage: gen_manual.py [version]"""
import json, html, sys, os
from PIL import Image, ImageDraw, ImageFont

ver = sys.argv[1] if len(sys.argv) > 1 else '0.1.0'
L = json.load(open('docs/mockup/layout.json'))
shot = Image.open('docs/screenshot.png').convert('RGB')
S = shot.width / 1200.0
tips = {t['id']: t['tip'] for t in L['topTips']}
R = lambda k: L[k]['r']
cell = lambda i: L['cells'][i]['knob']['r']

# (title, rect, text, range, default)
rows = [
 ('Preset bar', R('pName'), 'Arrows step through presets. The name opens the preset browser (search, type chips, load, export, edit, delete). Double-click the name to rename one of your own. Save stores the current settings as your own preset and asks before overwriting. A star after the name means the settings differ from the loaded preset.', '16 factory presets + yours', 'Start Here'),
 ('Bypass', R('bBypass'), tips['bBypass'] + ' Switching is crossfaded over about 10 ms, no clicks. Shortcut B.', 'on / off', 'off'),
 ('Guide', R('bGuide'), tips['bGuide'], '', ''),
 ('Settings', R('bSettings'), tips['bSettings'] + ' Looks: C charcoal (default), A neon navy, B purple soda pop.', '', 'Look C, bubbles on'),
 ('About & Support', R('bAbout'), tips['bAbout'] + ' Optional Ko-fi link.', '', ''),
 ('Needle meters', R('cvDialA'), tips['cvDialA'], 'Reduce 0 to 20 dB; Level -30 to +3 dB', ''),
 ('Transfer curve', R('cvCurve'), tips['cvCurve'], '-60 to +6 dB in, -60 to +18 dB out', ''),
 ('Gain reduction history', R('cvHist'), tips['cvHist'], '0 to 30 dB, 6 s', ''),
 ('Channel one ladder', L['ladderBox'][0]['r'], 'Gain reduction in dB for this channel. More lights, more turned down. Green up to 4 dB, yellow 5 to 8, orange 10 to 12, pink from 16.', '1 to 24 dB', ''),
 ('Input', cell(0), 'Input sets how hard the signal hits the compressor, in 1 dB steps. More input means more gain reduction and more tube colour.', '0 to +24 dB', '0 dB'),
 ('Threshold', cell(1), 'Threshold is the level where the box starts turning things down. 0 catches only the loudest peaks, 10 catches quieter sound too. The row under the knob shows the level in dB (knob x -3.2 dB).', '0 to 10 (0 to -32 dB)', '2'),
 ('Time', cell(2), 'Time selects one of six attack and release pairs. 1 to 4 are fixed (attack 0.2, 0.2, 0.4 and 0.8 ms; release 0.3, 0.8, 2 and 5 s). 5 and 6 are automatic: the release stretches when peaks keep coming (5: 2 to 10 s, 6: 0.3 to 25 s).', 'TC 1 to TC 6', 'TC 3'),
 ('Tubes', R('cvTubes'), tips['cvTubes'], '', ''),
 ('Mode', R('mLR'), 'Left / Right works on the two sides. Lat / Vert works on the middle (sum) and the sides (difference). Shortcut M.', 'Left / Right, Lat / Vert', 'Left / Right'),
 ('Link', R('bLink'), tips['bLink'] + ' Shortcut L. Turning Link on copies channel one to channel two.', 'on / off', 'on'),
 ('Level Match', R('bLevel'), tips['bLevel'] + ' While on, the button shows the gain it applies.', 'on / off', 'off'),
 ('Needle: GR / Level', R('mtR'), 'Chooses what the two needle meters show.', 'GR, Level', 'GR'),
 ('Side-chain HP', R('selHP'), tips['selHP'], 'Off, 60, 120, 240 Hz', 'Off'),
 ('Output', cell(3), 'Output is the volume after the squeeze. It is the last gain stage before the Mix.', '-12 to +12 dB', '+2 dB'),
 ('Mix', cell(4), 'Mix blends the original with the squeezed sound. 100 is all squeezed. The dry path is delayed by the plugin latency so the blend stays in phase.', '0 to 100 %', '100 %'),
 ('In and Out meters', R('cvIn'), 'Peak level, -60 to +6 dB. Blue up to -12, purple up to 0, pink above. The small block at the top marks a clip until you click it. The numbers under the strips are the highest peaks.', '-60 to +6 dB', ''),
]
img = shot.copy(); d = ImageDraw.Draw(img)
font = ImageFont.truetype('assets/fonts/BarlowCondensed-Bold.ttf', max(8, int(13 * S)))
for i, (n, r, t, rg, df) in enumerate(rows, 1):
    cx, cy, rad = (r[0] + 9) * S, (r[1] + 9) * S, 8 * S
    d.ellipse((cx - rad, cy - rad, cx + rad, cy + rad), fill=(255, 123, 177), outline=(0, 0, 0), width=max(1, int(S)))
    w = d.textlength(str(i), font=font)
    d.text((cx - w / 2, cy - 8 * S), str(i), fill=(11, 11, 13), font=font)
os.makedirs('docs/manual', exist_ok=True)
img.save('docs/manual/tour.png')
def crop(name, box): shot.crop(tuple(int(v * S) for v in box)).save('docs/manual/%s.png' % name)
crop('header', (52, 0, 1148, 46)); crop('displays', (52, 46, 1056, 230)); crop('controls', (52, 230, 1056, 548)); crop('meters', (1056, 46, 1148, 548))

e = html.escape
table = '\n'.join('<tr><td>%d</td><th>%s</th><td>%s</td><td>%s</td><td>%s</td></tr>' % (i, e(n), e(t), e(rg), e(df)) for i, (n, r, t, rg, df) in enumerate(rows, 1))
page = open('tools/manual_template.html').read().replace('@VER@', ver).replace('@TABLE@', table)
open('docs/manual.html', 'w').write(page)
print('manual ok', len(rows), 'controls')
=======
"""Build docs/manual.html from tools/layout.json (control names and info-bubble texts of the mockup).
CI turns it into a PDF. Run from the repo root after refreshing docs/screenshot.png."""
import json, html
L = json.load(open('tools/layout.json'))
e = html.escape
tip = {}
kn = [c for c in L['ctl'] if c['cls'] == 'knob']
for c, n in zip(kn, ['inA', 'thA', 'tcA', 'out', 'mix', 'inB', 'thB', 'tcB']): tip[n] = c['tip']
for c in L['ctl']:
    if c['id']: tip[c['id']] = c['tip']
tip['ladA'] = next(c['tip'] for c in L['ctl'] if c['cls'] == 'ladder')
ROWS = [('Preset arrows, name, Save', tip['pName'] + ' ' + tip['pSave'], '16 factory presets plus yours', 'Start Here'),
        ('Bypass', tip['bBypass'], 'on or off', 'off'),
        ('Guide, Settings, About', 'Short guide, info bubbles, power saving and colour look, about and where your files live.', '', ''),
        ('Input (per channel)', tip['inA'], '0 to +24 dB, 1 dB steps', '0 dB'),
        ('Threshold (per channel)', tip['thA'], '0 to 10 (0 = -0 dB, 10 = -32 dB)', '2.0 (-6.4 dB)'),
        ('Time (per channel)', tip['tcA'], '1 to 6', '3'),
        ('Left / Right, Lat / Vert', tip['mLR'] + ' ' + tip['mLV'], '', 'Left / Right'),
        ('Link', tip['bLink'], 'on or off', 'on'),
        ('Level Match', tip['bLevel'], 'on or off', 'off'),
        ('Needle: GR, Needle: Level', tip['mtR'] + ' ' + tip['mtL'], '', 'GR'),
        ('Side-chain HP', tip['selHP'], 'Off, 60, 120, 240 Hz', 'Off'),
        ('Output', tip['out'], '-12 to +12 dB', '+2 dB'),
        ('Mix', tip['mix'], '0 to 100 %', '100 %'),
        ('Needle meters', tip['cvDialA'], '', ''),
        ('Transfer curve', tip['cvCurve'], '', ''),
        ('Gain reduction history', tip['cvHist'], '6 seconds', ''),
        ('Valves', tip['cvTubes'], '', ''),
        ('Gain reduction lights', tip['ladA'], '1 to 24 dB', ''),
        ('In and Out meters', tip['cvIn'], '-60 to +6 dB', '')]
def table():
    return '<table><tr><th>Control</th><th>What it does</th><th>Range</th><th>Default</th></tr>' + ''.join(
        '<tr><th>%s</th><td>%s</td><td>%s</td><td>%s</td></tr>' % (e(n), e(t), e(r), e(d)) for n, t, r, d in ROWS) + '</table>'
page = '''<!doctype html>
<html lang="en"><head><meta charset="utf-8"><title>Furchild manual</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
:root{--bg:#0b0b0d;--card:#16171a;--line:#33363d;--ink:#e8e9ee;--dim:#a9adb8;--teal:#02EBBA;--pink:#FF7BB1}
@media print{:root{--bg:#fff;--card:#f4f4f6;--line:#c9cbd3;--ink:#16171a;--dim:#4a4d55;--teal:#007a62;--pink:#b0114f}}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--ink);font:16px/1.5 "Barlow Condensed","Arial Narrow",Arial,sans-serif;padding:24px max(16px,env(safe-area-inset-left,0px))}
main{max-width:980px;margin:0 auto}
h1{font-size:40px;margin:0 0 4px;font-style:italic}
h2{font-size:26px;margin:36px 0 8px;color:var(--teal);letter-spacing:1px;text-transform:uppercase;break-after:avoid}
a{color:var(--pink)}
img{max-width:100%;height:auto;border:1px solid var(--line);border-radius:6px;display:block;margin:8px 0}
table{border-collapse:collapse;width:100%;font-size:15px;display:block;overflow-x:auto}
td,th{border-bottom:1px solid var(--line);padding:5px 8px;text-align:left;vertical-align:top}
th{white-space:nowrap}
code,kbd{font-family:"JetBrains Mono",Consolas,monospace;font-size:13px;background:var(--card);border:1px solid var(--line);border-radius:3px;padding:0 4px}
.dim{color:var(--dim)}
@media print{body{padding:0}tr{break-inside:avoid}img{break-inside:avoid}}
</style></head><body><main>
<h1>ClawShaper</h1>
<p class="dim">Manual for version @VER@. Studio 87. Free and open source (AGPL-3.0-or-later). Collects nothing, never uses the network.</p>
<p>A multiband transient shaper with a safety limiter. It turns the punch (attack) and the tail (sustain) of your sound up or down, in up to four bands, and it reacts to changes in level, not to the absolute level.</p>

<h1>Furchild</h1>
<p class="dim">Manual for version @VER@. Studio 87. Free and open source (AGPL-3.0-or-later). Collects nothing, never uses the network.</p>
<p>A two-channel variable-mu style compressor: the harder the signal pushes, the more it turns down, with a soft knee, from almost 1:1 up to about 20:1. Six time constants (two of them automatic), Left / Right or Lat / Vert operation, a light valve-style colour and a side-chain high-pass.</p>

<h2>Quick start</h2>
<ol>
<li>Put Furchild on a track or bus. Pick a preset (for example <b>Vocal Level</b> or <b>Mix Bus Glue</b>) or start from <b>Start Here</b>.</li>
<li>Raise <b>Input</b> one dB at a time until the needle moves 3 to 6 dB on the loud parts. More input means more gain reduction and more colour.</li>
<li><b>Threshold</b> sets where squeezing starts. <b>Time</b> sets speed: 1 is the fastest, 4 the slowest fixed one, 5 and 6 are automatic.</li>
<li>Switch on <b>Level Match</b>, press <b>Bypass</b> (B) to compare, then switch it off again and set <b>Output</b> by ear.</li>
</ol>

<h2>The window</h2>
<img src="screenshot.png" alt="Furchild window">
@T1@

<h2>How it works</h2>
<ul>
<li>The gain reduction follows a soft curve: the ratio rises with level, about 1:1 near the threshold and about 20:1 when pushed hard. The knee is 5 dB wide.</li>
<li>Attack and release come from six pairs. Pairs 5 and 6 are program dependent: release stretches (2 to 10 s, or 0.3 to 25 s) when peaks keep coming.</li>
<li>Lat / Vert turns the stereo signal into middle (sum) and sides (difference), squeezes each on its own and turns it back. Link makes both channels follow the louder one.</li>
<li>A gentle valve-style saturation adds a little colour after the gain stage; a DC blocker and a soft clip above 0.9 keep the output safe.</li>
<li>No latency: nothing is looked ahead. Mix 100 % is the compressed sound, 0 % the dry input.</li>
</ul>
<p class="dim">The gain law, time constants and colour are our own design; they are not copied from any commercial product.</p>

<h2>Keyboard and mouse</h2>
<table><tr><td><kbd>B</kbd></td><td>Bypass on or off</td></tr><tr><td><kbd>[</kbd> <kbd>]</kbd></td><td>Previous and next preset</td></tr><tr><td>Knobs</td><td>Drag up or down, mouse wheel. Hold Shift for fine steps. Double-click to reset.</td></tr><tr><td>In and Out meters</td><td>Click to clear the clip light.</td></tr><tr><td>Window</td><td>Drag a corner; it keeps its proportions. Double-click the wood to return to 100%.</td></tr><tr><td>Presets</td><td>Save stores your own preset in your user data folder (Studio 87/Furchild/presets). Double-click the preset name to rename your own preset. Presets only ever live on your computer.</td></tr></table>

<h2>Install and uninstall</h2>
<ul>
<li><b>Linux:</b> unpack the zip, run <code>install.sh</code> (copies the VST3 to <code>~/.vst3</code>). <code>uninstall.sh</code> removes it.</li>
<li><b>Windows:</b> run the installer. It adds an uninstaller. Close your DAW first.</li>
<li><b>macOS:</b> open the package. The build is not notarised, so macOS may ask you to allow it in System Settings, Privacy and Security.</li>
</ul>
<p>After installing, rescan plugins in your host.</p>

<h2>Licence and thanks</h2>
<p>AGPL-3.0-or-later. Built with JUCE (AGPLv3). Fonts: Barlow Condensed and JetBrains Mono (SIL Open Font Licence 1.1), Liberation Serif (SIL Open Font Licence 1.1). Source: <a href="https://github.com/SpeakingColors/Furchild">github.com/SpeakingColors/Furchild</a>.</p>
</main></body></html>
'''
open('docs/manual.html', 'w').write(page.replace('@VER@', '0.1.0').replace('@T1@', table()))
print('manual ok', len(ROWS))
>>>>>>> ffce006 (Furchild plugin: engine, native window, presets, manual, installers, workflows (work in progress, first build pending))
