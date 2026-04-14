#pragma once

// Embedded web UI. Placeholders replaced at runtime:
//   {{VERSION}}, {{WIFI_SSID}}, {{GPIO_PIN}}, {{TOGGLE_MS}}, {{POLL_MIN}}, {{AP_MODE}}

static const char CONFIG_HTML[] PROGMEM = R"rawhtml(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 Thing Config</title>
<!-- Project emoji: change ⚙️ in the favicon and h1 to match your project -->
<link rel="icon" href="data:image/svg+xml,<svg xmlns=%22http://www.w3.org/2000/svg%22 viewBox=%220 0 100 100%22><text y=%22.9em%22 font-size=%2290%22>⚙️</text></svg>">
<style>
  :root{--bg:#1a1a1a;--card:#2a2a2a;--accent:#444;--hi:#1e7a4a;--dirty:#b06000;--txt:#eee;--muted:#aaa;--inp:#2a2a2a;}
  *{box-sizing:border-box;margin:0;padding:0;}
  body{background:var(--bg);color:var(--txt);font-family:system-ui,sans-serif;min-height:100vh;display:flex;flex-direction:column;align-items:center;padding:1rem;}
  h1{font-size:1.4rem;margin-bottom:1rem;color:var(--hi);}
  .version{font-size:.75rem;color:var(--muted);margin-bottom:1.5rem;}
  .card{background:var(--card);border-radius:8px;padding:1.5rem;width:100%;max-width:460px;margin-bottom:1rem;}
  .card h2{font-size:1rem;margin-bottom:1rem;border-bottom:1px solid var(--accent);padding-bottom:.5rem;}
  label{display:block;font-size:.85rem;color:var(--muted);margin-bottom:.25rem;}
  input[type=text],input[type=password],input[type=number]{width:100%;background:var(--inp);border:1px solid var(--accent);color:var(--txt);padding:.5rem .75rem;border-radius:4px;font-size:.95rem;margin-bottom:.75rem;}
  input:focus{outline:none;border-color:var(--hi);}
  .row{display:flex;gap:.5rem;align-items:flex-end;margin-bottom:.75rem;}
  .row>div{flex:1;}
  .row>div>input{margin-bottom:0;}
  button{background:var(--accent);color:var(--txt);border:none;padding:.55rem 1.25rem;border-radius:4px;cursor:pointer;font-size:.9rem;}
  button:hover{background:var(--hi);}
  #saveBtn{width:100%;padding:.65rem;font-size:1rem;background:var(--hi);}
  #saveBtn.dirty{background:var(--dirty);}
  #scanBtn{white-space:nowrap;}
  #scanResults{font-size:.8rem;color:var(--muted);margin-top:.5rem;display:none;}
  .scan-item{cursor:pointer;padding:.25rem .5rem;border-radius:3px;}
  .scan-item:hover{background:var(--accent);}
  .hidden{display:none!important;}
  #otaSection input[type=file]{color:var(--txt);}
  .note{font-size:.78rem;color:var(--muted);margin-top:.5rem;}
</style>
</head>
<body>
<h1>⚙️ ESP32 Thing</h1>
<div class="version">Firmware {{VERSION}}</div>

<form id="cfg" method="POST" action="/save">

<!-- WiFi -->
<div class="card">
  <h2>WiFi</h2>
  <label>SSID</label>
  <div class="row">
    <div><input type="text" name="wifi_ssid" id="wifi_ssid" value="{{WIFI_SSID}}" autocomplete="off"></div>
    <button type="button" id="scanBtn">Scan</button>
  </div>
  <div id="scanResults"></div>
  <label>Password</label>
  <input type="password" name="wifi_pass" id="wifi_pass" placeholder="(unchanged)" autocomplete="off">
</div>

<!-- GPIO / Task (hidden in AP mode) -->
<div class="card" id="gpioCard">
  <h2>GPIO Toggle</h2>
  <label>Pin number (0–39)</label>
  <input type="number" name="gpio_pin" id="gpio_pin" value="{{GPIO_PIN}}" min="0" max="39">
  <label>Toggle interval (ms, min 100)</label>
  <input type="number" name="toggle_ms" id="toggle_ms" value="{{TOGGLE_MS}}" min="100">
  <label>Poll interval (minutes, 1–1440)</label>
  <input type="number" name="poll_min" id="poll_min" value="{{POLL_MIN}}" min="1" max="1440">
</div>

<!-- OTA (hidden in AP mode) -->
<div class="card hidden" id="otaSection">
  <h2>OTA Firmware Update</h2>
  <input type="file" id="otaFile" accept=".bin">
  <button type="button" id="otaBtn">Upload Firmware</button>
  <div id="otaStatus" class="note"></div>
</div>

<button type="submit" id="saveBtn">Save</button>
</form>

<script>
(function(){
  var apMode = {{AP_MODE}};

  // Hide station-only sections in AP mode
  if(apMode){
    document.getElementById('gpioCard').classList.add('hidden');
  } else {
    document.getElementById('otaSection').classList.remove('hidden');
  }

  // Dirty tracking
  var saveBtn = document.getElementById('saveBtn');
  document.getElementById('cfg').addEventListener('input', function(){
    saveBtn.classList.add('dirty');
  });

  // WiFi scan
  document.getElementById('scanBtn').addEventListener('click', function(){
    var el = document.getElementById('scanResults');
    el.style.display='block';
    el.textContent='Scanning...';
    fetch('/scan').then(function(r){return r.json();}).then(function(aps){
      if(!aps.length){el.textContent='No networks found.';return;}
      el.innerHTML='';
      aps.forEach(function(ap){
        var d=document.createElement('div');
        d.className='scan-item';
        d.textContent=ap.ssid+' ('+ap.rssi+' dBm)';
        d.addEventListener('click',function(){
          document.getElementById('wifi_ssid').value=ap.ssid;
          saveBtn.classList.add('dirty');
          el.style.display='none';
        });
        el.appendChild(d);
      });
    }).catch(function(){el.textContent='Scan failed.';});
  });

  // OTA upload
  document.getElementById('otaBtn').addEventListener('click', function(){
    var file = document.getElementById('otaFile').files[0];
    var status = document.getElementById('otaStatus');
    if(!file){status.textContent='Select a .bin file first.';return;}
    var fd = new FormData();
    fd.append('firmware', file, file.name);
    status.textContent='Uploading...';
    fetch('/update',{method:'POST',body:fd}).then(function(r){
      status.textContent = r.ok ? 'Update successful! Rebooting...' : 'Update failed (HTTP '+r.status+').';
    }).catch(function(){status.textContent='Upload error.';});
  });
})();
</script>
</body>
</html>
)rawhtml";
