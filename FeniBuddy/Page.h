#pragma once
const char BUDDY_PAGE[] PROGMEM = R"FENI(<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Feni desk buddy</title>
<style>
:root{color-scheme:dark;--bg:#101c23;--panel:#1a2b34;--ink:#e9f2ed;--muted:#a6b9bf;--accent:#b8f3cd}*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--ink);font:16px/1.5 system-ui,sans-serif}main{max-width:980px;margin:auto;padding:32px 20px 64px}header{margin-bottom:26px}h1{font-size:42px;letter-spacing:-2px;margin:0}h2{font-size:21px;margin:0 0 12px}p{margin:8px 0 18px;color:var(--muted)}.tag{color:var(--accent);letter-spacing:2px;font-size:12px}.grid{display:grid;grid-template-columns:1fr 1fr;gap:18px}section{background:var(--panel);padding:24px;border:1px solid #33454c;border-radius:18px}section.wide{grid-column:1/-1}label{display:block;margin:13px 0 5px;font-size:14px}input,select,button{font:inherit;border:1px solid #62777e;border-radius:8px;padding:10px 12px}input,select{width:100%;background:#12232b;color:var(--ink)}button{background:var(--accent);color:#13261c;cursor:pointer;font-weight:600;margin:14px 8px 0 0}button.secondary{background:transparent;color:var(--ink)}button:disabled{opacity:.5;cursor:wait}button:focus-visible,input:focus-visible,select:focus-visible{outline:3px solid #81bcff;outline-offset:2px}small{display:block;color:var(--muted);margin-top:5px}.live{display:flex;align-items:center;gap:24px}.preview{width:240px;max-width:100%;aspect-ratio:5/4;image-rendering:pixelated;background:black;border:5px solid #0b1419;border-radius:12px}.status{color:var(--accent)}#notice{position:sticky;top:8px;background:#dff8e7;color:#16301e;padding:12px 16px;border-radius:8px;z-index:2;white-space:pre-line}#notice:empty{display:none}.preset{display:grid;grid-template-columns:85px 1fr;gap:10px;margin:8px 0}details{margin-top:16px}summary{cursor:pointer;color:var(--accent)}a{color:var(--accent)}.steps{padding-left:22px;color:var(--muted)}code{color:var(--ink)}#events{padding-left:22px;color:var(--muted)}@media(max-width:680px){.grid{grid-template-columns:1fr}.live{flex-direction:column;align-items:start}h1{font-size:36px}section{padding:19px}}
</style></head><body><main>
<header><div class="tag">A LITTLE COMPANY, ON YOUR DESK</div><h1>Hello, I'm Feni.</h1><p id="connection">Connecting to your buddy...</p></header>
<div id="notice" role="status" aria-live="polite"></div>
<div class="grid">
<section class="wide"><div class="live"><img id="screen" class="preview" alt="Live copy of Feni's display"><div><h2>Your desk buddy</h2><p>Tap to move. Hold to choose.<br>Double tap to go back from any screen.</p><button data-action="tap">Tap</button><button data-action="hold">Hold</button><button data-action="back" class="secondary">Back</button><small id="timerState"></small></div></div></section>
<section><h2>1. Connect to Wi-Fi</h2><p>Feni uses a 2.4 GHz network. Enter its name exactly as shown on your router or phone.</p>
<form id="wifiForm"><label for="ssid">Network name</label><input id="ssid" name="ssid" maxlength="32" required autocomplete="off"><label for="password">Wi-Fi password</label><input id="password" name="password" type="password" maxlength="64" autocomplete="new-password"><small>Leave blank only for an open Wi-Fi network.</small><button>Connect Feni</button></form>
<button id="forgetWifi" class="secondary">Forget saved Wi-Fi</button><small id="wifiResult">First setup: join Feni-Setup (password fenibuddy), then open 192.168.4.1.</small>
<p>After connection, the setup hotspot closes. Rejoin your home Wi-Fi and open <a href="http://feni.local">feni.local</a>.</p></section>
<section><h2>2. Make it yours</h2><form id="modeForm"><label for="mode">Home screen</label><select id="mode" name="mode"><option value="0">Buddy - animated face</option><option value="1">Clock - always show time</option><option value="2">Auto - tap for a 10-second clock</option></select><label for="timezone">Clock timezone</label><input id="timezone" name="timezone" value="IST-5:30" maxlength="48" required><small>POSIX timezone: India IST-5:30; UTC UTC0. See README for daylight-saving examples.</small><button>Save preferences</button></form><button id="clockSync" class="secondary">Use this device's time</button><small id="clockState"></small><p>Hold the onboard FLASH button for the menu. Timer choices are 10, 20 and 30 minutes, plus Custom.</p><form id="timerForm"><label for="timerMinutes">Custom timer (minutes)</label><input id="timerMinutes" name="minutes" type="number" min="1" max="180" value="25" required><button>Start timer</button></form><button id="cancelTimer" class="secondary">Cancel timer</button><small>1–180 minutes. You can also edit a custom duration using FLASH on Feni.</small></section>
<section><h2>Google Calendar</h2><p>Connect once with a Google Apps Script bridge. Feni then checks your events over Wi-Fi, without a running computer.</p><div id="calendarState" class="status"></div><ol id="events"></ol>
<details><summary>Connect or change calendar</summary><ol class="steps"><li>Open <code>calendar/Code.gs</code> from this project in a new Apps Script project.</li><li>Run <code>setupFeni</code> once and authorize read-only Calendar access.</li><li>Deploy as a web app, executing as you, with access set to Anyone.</li><li>Copy the deployment URL and your generated key from Script properties.</li></ol>
<form id="calendarForm"><label for="calendarUrl">Apps Script deployment URL</label><input id="calendarUrl" name="calendarUrl" type="url" placeholder="https://script.google.com/macros/s/.../exec" required autocomplete="off"><label for="calendarKey">Feni calendar key</label><input id="calendarKey" name="calendarKey" type="password" minlength="32" maxlength="64" required autocomplete="new-password"><small>The key protects the calendar feed. Keep it private.</small><button>Connect calendar</button></form><button id="unlink" class="secondary">Disconnect calendar</button></details><button id="calendarSync" class="secondary">Refresh events</button><small>Checks every 2 minutes while on the home screen. Visual reminders appear 10 minutes before and at the start of timed events.</small></section>
<section><h2>WLED lighting</h2><p>Create lighting presets in WLED, then add their IDs here. Hold a preset on Feni's WLED screen to apply it.</p>
<form id="wledForm"><label for="wledIp">WLED IP on this network</label><input id="wledIp" name="wledIp" placeholder="192.168.1.50" inputmode="decimal"><small>Use the numeric IP without http://. Blank clears the address.</small><div id="presetFields"></div><button>Save WLED settings</button></form><div id="wledState" class="status"></div><div id="presetButtons"></div><small>Reserve this IP in your router so it stays the same.</small></section>
</div><p style="margin-top:24px">Feni | ESP8266 / ST7735 / FLASH button | Calendar alerts and timer alarms are visual.</p>
</main><script>
'use strict';
const $=id=>document.getElementById(id);let token='',initialized=false,refreshing=false,wifiPending=false;
let presetSignature='';
for(let i=0;i<6;i++){
 const row=document.createElement('div');row.className='preset';
 const id=document.createElement('input');id.type='number';id.min=1;id.max=250;id.name='p'+i+'id';id.id=id.name;id.placeholder='ID';id.setAttribute('aria-label','Preset '+(i+1)+' ID');
 const name=document.createElement('input');name.name='p'+i+'name';name.id=name.name;name.maxLength=20;name.placeholder='Preset name';name.setAttribute('aria-label','Preset '+(i+1)+' name');
 row.append(id,name);$('presetFields').append(row);
}
function notice(message){$('notice').textContent=message;}
async function post(path,data){
 if(!token){const response=await fetch('/session',{cache:'no-store'});if(!response.ok)throw new Error('Cannot open Feni session');token=await response.text();}
 const response=await fetch(path,{method:'POST',headers:{'X-Feni-Token':token},body:new URLSearchParams(data)});
 const text=await response.text();if(!response.ok){if(response.status===403)token='';throw new Error(text);}return text;
}
async function action(path,data,quiet=false){try{const message=await post(path,data);if(!quiet)notice(message);await refresh();}catch(error){notice(error.message);}}
for(const button of document.querySelectorAll('[data-action]'))button.onclick=()=>action('/control',{action:button.dataset.action},true);
for(const [id,path] of [['wifiForm','/wifi'],['modeForm','/settings'],['calendarForm','/settings'],['wledForm','/settings'],['timerForm','/timer']]){
 $(id).onsubmit=async event=>{event.preventDefault();const button=event.submitter;button.disabled=true;
 try{const message=await post(path,new FormData($(id)));notice(message);
 if(id==='wifiForm'){wifiPending=true;$('password').value='';$('wifiResult').textContent='Connecting... Keep this page open. If the hotspot closes, rejoin your home Wi-Fi and open feni.local.';}
 if(id==='calendarForm'){$('calendarKey').value='';$('calendarUrl').value='';}
 await refresh();}catch(error){notice(error.message);}finally{button.disabled=false;}};
}
$('cancelTimer').onclick=()=>action('/timer',{action:'cancel'});
$('forgetWifi').onclick=async()=>{if(!confirm('Forget the saved Wi-Fi and return Feni to setup?'))return;try{notice(await post('/wifi/forget',{}));}catch(error){notice(error.message);}};
$('clockSync').onclick=()=>action('/clock',{epoch:Math.floor(Date.now()/1000)});
$('calendarSync').onclick=()=>action('/calendar/sync',{});
$('unlink').onclick=()=>action('/settings',{clearCalendar:1});
async function refresh(){
 if(refreshing)return;refreshing=true;
 try{
 const response=await fetch('/status',{cache:'no-store'});if(!response.ok)throw new Error('Device unavailable');const state=await response.json();
 $('connection').textContent=state.wifi?'On your Wi-Fi | '+state.ip+(state.setup?' | setup hotspot closing...':''):'Setup / offline | '+state.networkMessage;
 if(wifiPending && state.wifi){$('wifiResult').textContent='Connected. Rejoin your home Wi-Fi, then open http://'+state.ip+' or feni.local.';wifiPending=false;}
 $('clockState').textContent=state.clock?'Clock is set. Time continues through Wi-Fi outages.':'Clock not set yet. Waiting for internet time, or use the button above.';
 $('timerState').textContent=state.timerRunning?'Timer: '+Math.floor(state.timerSeconds/60)+':'+String(state.timerSeconds%60).padStart(2,'0')+' remaining':state.timerDone?'Timer finished.':'Hold for the menu; double tap goes back.';
 $('calendarState').textContent=state.calendarMessage+(state.calendarFetched&&!state.calendarFresh?' | cached events':'');
 $('events').replaceChildren();for(const event of state.events){const row=document.createElement('li');row.textContent=event.title+' | '+(event.allDay?'all day':new Date(event.start*1000).toLocaleString());$('events').append(row);}
 $('wledState').textContent=state.wledMessage;
 if(!initialized){$('ssid').value=state.ssid;$('mode').value=state.mode;$('timezone').value=state.timezone;$('wledIp').value=state.wledIp;state.presets.forEach((p,i)=>{$('p'+i+'id').value=p.id;$('p'+i+'name').value=p.name;});initialized=true;}
 const signature=JSON.stringify(state.presets);if(signature!==presetSignature){presetSignature=signature;$('presetButtons').replaceChildren();state.presets.forEach((p,index)=>{const b=document.createElement('button');b.className='secondary';b.textContent=p.name;b.onclick=()=>action('/wled/apply',{index});$('presetButtons').append(b);});}
 $('screen').hidden=state.page===8;if(state.page!==8)$('screen').src='/frame.bmp?t='+Date.now();
 }catch(error){$('connection').textContent=wifiPending?'Feni is switching networks. Rejoin home Wi-Fi and open feni.local.':'Cannot reach Feni. Check that you are on the same Wi-Fi; it may be syncing Calendar.';}
 finally{refreshing=false;}
}
refresh();setInterval(refresh,2500);
</script></body></html>)FENI";
