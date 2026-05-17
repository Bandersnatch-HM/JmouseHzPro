#ifndef WEB_PAGES_H
#define WEB_PAGES_H

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Jmouse HzPro</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
:root{--bg:#0f172a;--card:#1e293b;--card-border:#334155;--accent:#38bdf8;--accent2:#818cf8;--text:#f8fafc;--text2:#94a3b8;--success:#22c55e;--warn:#f59e0b;--danger:#ef4444;--radius:12px}
body{font-family:system-ui,-apple-system,sans-serif;background:var(--bg);color:var(--text);min-height:100vh;padding-bottom:32px}
.container{max-width:480px;margin:0 auto;padding:16px}
header{text-align:center;padding:20px 0 16px}
.logo{font-size:26px;font-weight:700;color:var(--accent);letter-spacing:-0.5px}
.logo span{font-size:13px;display:block;color:var(--text2);font-weight:400;margin-top:2px}
.status-bar{display:flex;align-items:center;justify-content:center;gap:8px;padding:10px 20px;border-radius:30px;background:var(--card);border:1px solid var(--card-border);margin-bottom:20px}
.dot{width:10px;height:10px;border-radius:50%;flex-shrink:0}
.dot.on{background:var(--success);box-shadow:0 0 8px var(--success)}
.dot.off{background:var(--danger)}
.status-text{font-size:13px;font-weight:600}
.nav{display:flex;gap:4px;background:var(--card);border-radius:12px;padding:4px;margin-bottom:16px;border:1px solid var(--card-border)}
.nav button{flex:1;padding:10px 0;border:none;background:none;color:var(--text2);font-size:13px;font-weight:600;border-radius:8px;cursor:pointer;transition:all .2s}
.nav button.active{background:rgba(56,189,248,0.15);color:var(--accent)}
.card{background:var(--card);border:1px solid var(--card-border);border-radius:var(--radius);padding:20px;margin-bottom:14px}
.card h3{font-size:13px;color:var(--text2);margin-bottom:14px;text-transform:uppercase;letter-spacing:1px;font-weight:700}
.stat-grid{display:grid;grid-template-columns:1fr 1fr;gap:12px}
.stat{text-align:center;padding:14px;background:#0f172a;border-radius:10px;border:1px solid var(--card-border)}
.stat .val{font-size:22px;font-weight:700;color:var(--accent)}
.stat .lbl{font-size:11px;color:var(--text2);margin-top:4px}
.panel{display:none}
.panel.active{display:block}
.mode-grid{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.mode-card{padding:14px;border-radius:10px;background:#0f172a;border:2px solid var(--card-border);cursor:pointer;text-align:center;font-size:14px;font-weight:600;transition:all .2s}
.mode-card.active{border-color:var(--accent);background:rgba(56,189,248,0.1);color:var(--accent)}
.slider-group{margin-bottom:20px}
.slider-group label{display:flex;justify-content:space-between;font-size:13px;font-weight:600;margin-bottom:8px;color:var(--text)}
input[type="range"]{width:100%;height:8px;background:#0f172a;border-radius:4px;outline:none;border:1px solid var(--card-border)}
.input-group{margin-bottom:16px}
.input-group label{display:block;font-size:13px;font-weight:600;margin-bottom:6px;color:var(--text2)}
.input-group input{width:100%;padding:12px 14px;border:1px solid var(--card-border);border-radius:10px;background:#0f172a;color:var(--text);font-size:14px;outline:none}
.btn{width:100%;padding:14px;border:none;border-radius:10px;font-size:14px;font-weight:600;cursor:pointer;transition:all .2s}
.btn-primary{background:var(--accent);color:#0f172a;font-weight:700}
.btn-primary:active{opacity:0.8}
.btn-outline{background:none;border:1px solid var(--card-border);color:var(--text2);margin-top:10px}
.btn-danger-full{background:rgba(239,68,68,0.15);border:1px solid rgba(239,68,68,0.3);color:var(--danger);margin-top:10px}
.paused-badge{display:none;text-align:center;padding:10px 16px;background:rgba(245,158,11,0.15);border:1px solid rgba(245,158,11,0.3);border-radius:10px;color:var(--warn);font-size:13px;font-weight:700;margin-bottom:16px}
.paused-badge.show{display:block}
</style>
</head>
<body>
<div class="container">
<header><div class="logo">Jmouse HzPro<span>Bluetooth Mouse Jiggler</span></div></header>
<div id="app">
<div class="status-bar"><div class="dot off" id="statusDot"></div><span class="status-text" id="statusText">Esperando...</span></div>
<div class="paused-badge" id="pausedBadge">⏸ JIGGLE PAUSADO</div>
<nav class="nav">
<button class="active" onclick="showTab('dash')">📊 Estado</button>
<button onclick="showTab('move')">🎯 Motion</button>
<button onclick="showTab('settings')">⚙️ Config</button>
</nav>
<div class="panel active" id="tab-dash">
<div class="card"><h3>Estadísticas</h3>
<div class="stat-grid">
<div class="stat"><div class="val" id="jiggleCount">0</div><div class="lbl">Jiggles</div></div>
<div class="stat"><div class="val" id="uptime">0m</div><div class="lbl">Uptime</div></div>
<div class="stat"><div class="val" id="modeDisplay">-</div><div class="lbl">Modo</div></div>
<div class="stat"><div class="val" id="intervalDisplay">-</div><div class="lbl">Intervalo</div></div>
</div></div>
<div class="card"><h3>Acciones</h3>
<button class="btn btn-primary" onclick="sendCmd('toggle')" id="btnToggle">⏸ Pausar</button>
<button class="btn btn-outline" onclick="sendCmd('jiggle')">🔄 Jiggle Ahora</button>
</div></div>
<div class="panel" id="tab-move">
<div class="card"><h3>Patrón</h3><div class="mode-grid" id="modeGrid"></div></div>
<div class="card"><h3>Ajustes</h3>
<div class="slider-group"><label>Intervalo <span id="intVal">30s</span></label><input type="range" id="sliderInterval" min="5" max="180" value="30" oninput="updateSlider()"></div>
<div class="slider-group"><label>Amplitud <span id="ampVal">2px</span></label><input type="range" id="sliderAmplitude" min="1" max="10" value="2" oninput="updateSlider()"></div>
<button class="btn btn-primary" onclick="saveMovement()">💾 Guardar</button>
</div></div>
<div class="panel" id="tab-settings">
<div class="card"><h3>WiFi (STA)</h3>
<div class="input-group"><label>SSID de tu Red</label><input type="text" id="wifiSSID" placeholder="Nombre de tu WiFi"></div>
<div class="input-group"><label>Contraseña</label><input type="text" id="wifiPass" placeholder="Password"></div>
<button class="btn btn-primary" onclick="saveWiFi()">💾 Conectar a mi WiFi</button>
<p style="font-size:10px;color:var(--text2);margin-top:8px">Usa http://jmouse.local para entrar desde tu red.</p>
</div>
<div class="card"><h3>Dispositivo</h3>
<div class="input-group"><label>Nombre Bluetooth</label><input type="text" id="inputName" maxlength="30"></div>
<button class="btn btn-primary" onclick="saveName()">💾 Guardar</button>
</div>
<div class="card"><button class="btn btn-outline" onclick="sendCmd('restart')">🔄 Reiniciar</button><button class="btn btn-danger-full" onclick="sendCmd('factory')">⚠️ Factory Reset</button></div>
</div></div></div>
<script>
const modeNames=['Micro','Horiz','Vert','Cruz','Bezier','Circle','Drift','Mix','Full+Shift'];
let state={connected:false,paused:false,mode:7,interval:30,amplitude:2,jiggles:0,uptime:0};
function init(){buildModeGrid();fetchState();setInterval(fetchState,8000);}
function fetchState(){fetch('/api/status').then(r=>r.json()).then(d=>{Object.assign(state,d);render();}).catch(()=>{});}
function render(){
const dot=document.getElementById('statusDot');
dot.className=state.connected?'dot on':'dot off';
document.getElementById('statusText').textContent=state.connected?'Conectado':'Desconectado';
document.getElementById('pausedBadge').className=state.paused?'paused-badge show':'paused-badge';
document.getElementById('btnToggle').textContent=state.paused?'▶ Reanudar':'⏸ Pausar';
document.getElementById('jiggleCount').textContent=state.jiggles;
document.getElementById('uptime').textContent=Math.floor(state.uptime/60000)+'m';
document.getElementById('modeDisplay').textContent=modeNames[state.mode]||'?';
document.getElementById('intervalDisplay').textContent=state.interval+'s';
document.querySelectorAll('.mode-card').forEach(c=>{c.classList.toggle('active',parseInt(c.dataset.mode)===state.mode);});
}
function buildModeGrid(){
const g=document.getElementById('modeGrid');
modeNames.forEach((n,i)=>{g.innerHTML+='<div class="mode-card" data-mode="'+i+'" onclick="selectMode('+i+')"><div class="name">'+n+'</div></div>';});
}
function showTab(t){
document.querySelectorAll('.panel').forEach(p=>p.classList.remove('active'));
document.querySelectorAll('.nav button').forEach(b=>b.classList.remove('active'));
document.getElementById('tab-'+t).classList.add('active');
event.currentTarget.classList.add('active');
}
function selectMode(m){state.mode=m;render();}
function updateSlider(){
document.getElementById('intVal').textContent=document.getElementById('sliderInterval').value+'s';
document.getElementById('ampVal').textContent=document.getElementById('sliderAmplitude').value+'px';
}
function sendCmd(cmd,arg){fetch('/api/cmd',{method:'POST',body:JSON.stringify({cmd:cmd,arg:arg||0})}).then(()=>fetchState());}
function saveMovement(){
fetch('/api/movement',{method:'POST',body:JSON.stringify({
mode:state.mode,
interval:parseInt(document.getElementById('sliderInterval').value),
amplitude:parseInt(document.getElementById('sliderAmplitude').value)
})}).then(()=>alert('Guardado'));
}
function saveWiFi(){
const s=document.getElementById('wifiSSID').value;
const p=document.getElementById('wifiPass').value;
fetch('/api/wifi',{method:'POST',body:JSON.stringify({ssid:s,pass:p})}).then(()=>alert('Configurado. Reinicia para conectar.'));
}
function saveName(){fetch('/api/name',{method:'POST',body:JSON.stringify({name:document.getElementById('inputName').value})}).then(()=>alert('Nombre guardado'));}
window.onload=init;
</script>
</body>
</html>
)rawliteral";

#endif
