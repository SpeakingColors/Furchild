#!/usr/bin/env python3
"""Read every control position, label, font and tooltip from the approved mockup page
(docs/mockup/Furchild.html) with a headless browser and write docs/mockup/layout.json.
tools/gen_layout.py turns that into src/editor/Layout.h, so the native window cannot drift from the mockup.
Run from the repo root. Fonts come from assets/fonts so text metrics match the real window."""
import asyncio, json, os, pathlib
from playwright.async_api import async_playwright

root = pathlib.Path.cwd()
src = (root / 'docs/mockup/Furchild.html').read_text()
fonts = root / 'assets/fonts'
ff = ''.join(
    "@font-face{font-family:'%s';font-weight:%s;font-style:%s;src:url('file://%s')}" % (fam, w, st, fonts / f)
    for fam, w, st, f in [
        ('Barlow Condensed', 500, 'normal', 'BarlowCondensed-Medium.ttf'),
        ('Barlow Condensed', 600, 'normal', 'BarlowCondensed-SemiBold.ttf'),
        ('Barlow Condensed', 700, 'normal', 'BarlowCondensed-Bold.ttf'),
        ('JetBrains Mono', 400, 'normal', 'JetBrainsMono-Regular.ttf'),
        ('JetBrains Mono', 600, 'normal', 'JetBrainsMono-SemiBold.ttf'),
        ('Fraunces', 700, 'italic', 'Fraunces-BoldItalic.ttf')])
# drop the remote font links; the local files above replace them
import re
src = re.sub(r'<link[^>]*fonts\.g[^>]*>', '', src)
open('/tmp/furchild_wrapped.html', 'w').write('<!doctype html><html><head><meta charset="utf-8"><style>' + ff + '</style></head><body>' + src + '</body></html>')

JS = r"""
() => {
  const st=document.querySelector('#stage'), sr=st.getBoundingClientRect(), sc=sr.width/1200;
  const r2=v=>Math.round(v*100)/100;
  const R=e=>{const r=e.getBoundingClientRect();return [r2((r.left-sr.left)/sc),r2((r.top-sr.top)/sc),r2(r.width/sc),r2(r.height/sc)]};
  const q=s=>document.querySelector(s), qa=s=>[...document.querySelectorAll(s)];
  const info=e=>{const cs=getComputedStyle(e);return {t:e.textContent,r:R(e),tip:e.dataset.tip||'',aria:e.getAttribute('aria-label')||'',
    fs:parseFloat(cs.fontSize),fw:parseInt(cs.fontWeight),ls:cs.letterSpacing==='normal'?0:parseFloat(cs.letterSpacing),tt:cs.textTransform,
    mono:cs.fontFamily.indexOf('JetBrains')>=0,mark:cs.fontFamily.indexOf('Fraunces')>=0,color:cs.color}};
  const o={sc};
  for(const [k,s] of Object.entries({cheekL:'#cheekL',cheekR:'#cheekR',main:'.main',hdr:'.hdr',body:'.body',left:'.left',disp:'.disp',knobs:'#knobs',rig:'.rig',meters:'.meters',foot:'.foot',
    logo:'.brand svg',name:'.brand .name',s87:'.brand .s87',grpA:'#grpA',grpB:'#grpB',mid:'#mid',midK:'#midK'})) o[k]=info(q(s));
  for(const id of ['pPrev','pName','pNext','pSave','bBypass','bGuide','bSettings','bAbout','mLR','mLV','bLink','bLevel','mtR','mtL','selHP','cvDialA','cvDialB','cvCurve','cvHist','cvTubes','cvIn','cvOut','pkIn','pkOut','winsz','capA','capB','dsA','dtA','dsB','dtB','hsA','hsB','ghA','ghB'])
    o[id]=info(q('#'+id));
  o.dcells=qa('.dcell').map(c=>({cap:info(c.querySelector('.cap')),cv:info(c.querySelector('canvas')),sub:[...c.querySelectorAll('.sub span')].map(info)}));
  o.mcap=qa('.meters .cap span').map(info);
  o.hpCap=info(q('#mid .row .cap'));
  o.footL=info(q('.foot span'));
  o.ladders=['A','B'].map(ch=>qa('#grp'+ch+' .lc').map(c=>({n:info(c.querySelector('.n')),led:R(c.querySelector('.ld')),color:getComputedStyle(c.querySelector('.ld')).color})));
  o.ladderBox=['A','B'].map(ch=>info(q('#grp'+ch+' .ladder')));
  const cells=[...qa('#grpA .kcell'),...qa('#midK .kcell'),...qa('#grpB .kcell')];
  o.cells=cells.map(c=>{
    const rows=[]; const kv=c.querySelector('.kval');
    const w=document.createTreeWalker(kv,NodeFilter.SHOW_TEXT);let n;
    while(n=w.nextNode()){if(!n.textContent.trim())continue;const rg=document.createRange();rg.selectNodeContents(n);const rr=rg.getBoundingClientRect();
      rows.push({t:n.textContent,b:n.parentElement.tagName==='B',x:r2((rr.left-sr.left)/sc),y:r2((rr.top-sr.top)/sc),w:r2(rr.width/sc),h:r2(rr.height/sc)})}
    const ends=c.querySelector('.ends');
    return {lab:info(c.querySelector('.lab')),knob:info(c.querySelector('.knob')),kval:info(kv),rows:rows,ends:ends?[...ends.querySelectorAll('span')].map(info):[],cell:R(c)}});
  o.sel=[...q('#selHP').options].map(x=>x.textContent);
  o.topTips=qa('[data-tip]').map(e=>({id:e.id||e.className,tip:e.dataset.tip}));
  return o;
}
"""
async def main():
    async with async_playwright() as p:
        b = await p.chromium.launch()
        pg = await b.new_page(viewport={'width': 1500, 'height': 900})
        msgs = []
        pg.on('console', lambda m: msgs.append(m.text) if m.type == 'error' else None)
        await pg.goto('file:///tmp/furchild_wrapped.html')
        await pg.wait_for_timeout(1500)
        d = await pg.evaluate(JS)
        json.dump(d, open('docs/mockup/layout.json', 'w'), indent=1)
        await pg.locator('#stage').screenshot(path='docs/mockup/mockup_reference.png')
        print('scale', d['sc'], 'cells', len(d['cells']), 'rig', d['rig']['r'], 'errors', msgs)
        await b.close()
asyncio.run(main())
