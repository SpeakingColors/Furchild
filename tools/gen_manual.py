#!/usr/bin/env python3
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
