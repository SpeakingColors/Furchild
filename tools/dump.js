// Dumps the layout of the approved Furchild mockup (rects relative to #stage) to tools/layout.json
const { chromium } = require('playwright');
(async()=>{
const b=await chromium.launch();const p=await b.newPage({viewport:{width:1400,height:900}});
await p.goto('file://'+process.argv[2]);await p.waitForTimeout(1200);
const out=await p.evaluate(()=>{
 const st=document.querySelector('#stage'),S=st.getBoundingClientRect();const sc=S.width/st.offsetWidth;
 const rc=r=>[+((r.left-S.left)/sc).toFixed(2),+((r.top-S.top)/sc).toFixed(2),+(r.width/sc).toFixed(2),+(r.height/sc).toFixed(2)];
 const cs=e=>getComputedStyle(e);
 const rgba=s=>{const m=s.match(/[\d.]+/g)||[0,0,0,0];return [+m[0],+m[1],+m[2],m.length>3?+m[3]:1]};
 const o={stage:[st.offsetWidth,st.offsetHeight],ctl:[],cv:[],btn:[],box:[],txt:[],knob:[],lad:[],sel:[]};
 // boxes
 for(const s of ['.main','.hdr','.body','.left','section.disp','.dcell','section.knobs','.grp','.mid','section.rig','aside.meters','.foot','.cheek','canvas','.btn','select','.kcell']){
  document.querySelectorAll('#stage '+s).forEach(e=>{const c=cs(e),r=e.getBoundingClientRect();if(!r.width)return;
   o.box.push({s,id:e.id,r:rc(r),bg:rgba(c.backgroundColor),bd:parseFloat(c.borderTopWidth)||0,bc:rgba(c.borderTopColor),rad:parseFloat(c.borderTopLeftRadius)||0,bimg:c.backgroundImage.slice(0,200)})});}
 document.querySelectorAll('#stage [data-tip]').forEach(e=>{const r=e.getBoundingClientRect();if(!r.width)return;
  o.ctl.push({tag:e.tagName,id:e.id,cls:e.className&&e.className.baseVal===undefined?e.className:'',r:rc(r),tip:e.dataset.tip,text:e.textContent.trim().slice(0,40)})});
 // text leaves
 const w=document.createTreeWalker(st,NodeFilter.SHOW_TEXT);let n;
 while(n=w.nextNode()){const t=n.textContent.trim();if(!t)continue;const pe=n.parentElement;if(pe.closest('button,select,option,#modal,#tip,#toast,.zoomrow,script,style,svg'))continue;
  const rg=document.createRange();rg.selectNodeContents(n);const r=rg.getBoundingClientRect();if(!r.width)continue;const c=cs(pe);
  o.txt.push({t,r:rc(r),pid:pe.id,pc:pe.className,fs:parseFloat(c.fontSize),fw:+c.fontWeight,fam:c.fontFamily.slice(0,30),ls:parseFloat(c.letterSpacing)||0,col:rgba(c.color),up:c.textTransform==='uppercase',it:c.fontStyle==='italic',ta:c.textAlign})}
 document.querySelectorAll('#stage .kcell').forEach(k=>{const kn=k.querySelector('.knob'),lab=k.querySelector('.lab'),v=k.querySelector('.kval'),en=k.querySelector('.ends');
  o.knob.push({kn:rc(kn.getBoundingClientRect()),lab:rc(lab.getBoundingClientRect()),val:rc(v.getBoundingClientRect()),ends:en?rc(en.getBoundingClientRect()):null,tip:kn.dataset.tip,aria:kn.getAttribute('aria-label'),min:+kn.getAttribute('aria-valuemin'),max:+kn.getAttribute('aria-valuemax'),labT:lab.textContent})});
 document.querySelectorAll('#stage .ld').forEach(l=>{o.lad.push({r:rc(l.getBoundingClientRect()),col:cs(l).color,n:l.previousSibling?l.previousSibling.textContent:''})});
 document.querySelectorAll('#stage select').forEach(e=>{o.sel.push({id:e.id,r:rc(e.getBoundingClientRect()),opts:[...e.options].map(x=>x.text)})});
 return o});
require('fs').writeFileSync('tools/layout.json',JSON.stringify(out,null,1));
console.log(out.stage,Object.entries(out).map(([k,v])=>k+':'+(v.length||0)).join(' '));
await p.screenshot({path:'/tmp/fc_mock.png'});
await b.close()})();
