#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>
#include <WiFiClient.h>
#include <vector>

struct WiFiScanResultUI {
  String ssid;
  String bssid_str;
  short rssi;
  uint8_t channel;
  int index;
};

void sendHeader(WiFiClient &client) {
  client.print("HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\n\n");
  client.print("<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">");
  client.print("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
  client.print("<title>WiFi Pentester Pro</title>");
  client.print("<style>");
  client.print(":root{--bg:#020617;--surface:#0f172a;--primary:#3b82f6;--primary-glow:rgba(59,130,246,0.5);--danger:#ef4444;--danger-glow:rgba(239,68,68,0.5);--success:#22c55e;--border:#1e293b;--text:#f8fafc;--text-muted:#94a3b8}");
  client.print("body{font-family:'Inter',system-ui,-apple-system,sans-serif;background:var(--bg);color:var(--text);margin:0;padding:0;display:flex;justify-content:center;min-height:100vh}");
  client.print(".container{width:100%;max-width:500px;padding:20px;box-sizing:border-box}");
  client.print(".glass{background:var(--surface);border:1px solid var(--border);border-radius:24px;padding:24px;box-shadow:0 25px 50px -12px rgba(0,0,0,0.5)}");
  client.print(".header{display:flex;justify-content:space-between;align-items:center;margin-bottom:32px}");
  client.print("h1{margin:0;font-size:1.5rem;font-weight:800;letter-spacing:-0.025em;background:linear-gradient(to right,#60a5fa,#3b82f6);-webkit-background-clip:text;-webkit-text-fill-color:transparent}");
  client.print(".status-pill{padding:6px 12px;border-radius:99px;font-size:0.75rem;font-weight:700;text-transform:uppercase;display:flex;align-items:center;gap:6px;background:var(--bg);border:1px solid var(--border)}");
  client.print(".dot{width:8px;height:8px;border-radius:50%;background:var(--text-muted)}");
  client.print(".dot-attacking{background:var(--danger);box-shadow:0 0 10px var(--danger);animation:blink 1s infinite}");
  client.print(".dot-resting{background:var(--success);box-shadow:0 0 10px var(--success)}");
  client.print("@keyframes blink{50%{opacity:0.3}}");
  client.print(".big-btn{width:100%;padding:18px;border-radius:18px;font-size:1.1rem;font-weight:800;cursor:pointer;border:none;transition:all 0.3s cubic-bezier(0.4,0,0.2,1);color:white;margin-bottom:24px;display:flex;align-items:center;justify-content:center;gap:10px;box-shadow:0 10px 15px -3px rgba(0,0,0,0.2)}");
  client.print(".btn-jam{background:var(--danger);box-shadow:0 0 20px var(--danger-glow)}");
  client.print(".btn-jam:hover{transform:translateY(-2px);box-shadow:0 0 30px var(--danger-glow)}");
  client.print(".btn-release{background:var(--primary);box-shadow:0 0 20px var(--primary-glow)}");
  client.print(".btn-release:hover{transform:translateY(-2px);box-shadow:0 0 30px var(--primary-glow)}");
  client.print(".grid-stats{display:grid;grid-template-columns:1fr 1fr;gap:16px;margin-bottom:24px}");
  client.print(".stat-card{background:rgba(255,255,255,0.03);padding:16px;border-radius:18px;border:1px solid var(--border);text-align:center}");
  client.print(".stat-val{display:block;font-size:1.5rem;font-weight:800;color:#fff;margin-top:4px}");
  client.print(".stat-lab{font-size:0.65rem;color:var(--text-muted);text-transform:uppercase;font-weight:700;letter-spacing:0.05em}");
  client.print(".tabs-nav{display:flex;background:var(--bg);padding:4px;border-radius:14px;margin-bottom:16px;border:1px solid var(--border)}");
  client.print(".tab-btn{flex:1;padding:10px;border-radius:10px;font-size:0.85rem;font-weight:600;cursor:pointer;border:none;background:transparent;color:var(--text-muted);transition:0.2s}");
  client.print(".tab-btn.active{background:var(--surface);color:var(--text);box-shadow:0 4px 6px -1px rgba(0,0,0,0.1)}");
  client.print(".list-box{max-height:300px;overflow-y:auto;background:rgba(255,255,255,0.02);border-radius:18px;border:1px solid var(--border);margin-bottom:24px}");
  client.print("table{width:100%;border-collapse:collapse}td,th{padding:14px;text-align:left;border-bottom:1px solid var(--border)}th{color:var(--text-muted);font-size:0.7rem;text-transform:uppercase}td{font-size:0.85rem}");
  client.print("input[type=checkbox]{width:20px;height:20px;border-radius:6px;accent-color:var(--primary)}");
  client.print(".settings-row{display:flex;align-items:center;justify-content:space-between;gap:12px;background:rgba(255,255,255,0.03);padding:12px 16px;border-radius:16px;border:1px solid var(--border);margin-bottom:12px}");
  client.print("input[type=number]{background:transparent;border:none;color:white;font-weight:800;font-size:1.1rem;width:50px;text-align:right;outline:none}");
  client.print(".icon-btn{padding:10px;border-radius:12px;background:var(--border);border:none;color:white;cursor:pointer;transition:0.2s}");
  client.print(".icon-btn:hover{background:var(--primary)}");
  client.print("</style>");
  client.print("<script>");
  client.print("function tab(e,id){document.querySelectorAll('.tab-content').forEach(x=>x.style.display='none');document.querySelectorAll('.tab-btn').forEach(x=>x.className='tab-btn');document.getElementById(id).style.display='block';e.target.className='tab-btn active';}");
  client.print("async function api(p,d={}){try{await fetch(p,{method:'POST',body:new URLSearchParams(d)});update();}catch(e){}}");
  client.print("function jam(){const s=Array.from(document.querySelectorAll('input:checked')).map(i=>['network',i.value]);if(!s.length)return alert('Select targets!');api('/deauth',new URLSearchParams(s));}");
  client.print("async function update(){try{const r=await fetch('/stats');const d=await r.json();");
  client.print("const st=document.getElementById('st');const dt=document.getElementById('dt');");
  client.print("st.innerText=d.status==='Idle'?'Idle':d.mode;dt.className='dot '+(d.status==='Idle'?'':'dot-'+d.mode.toLowerCase());");
  client.print("document.getElementById('fr').innerText=d.frames;document.getElementById('jam-btn').style.display=d.status==='Idle'?'flex':'none';");
  client.print("document.getElementById('stop-btn').style.display=d.status!=='Idle'?'flex':'none';");
  client.print("}catch(e){}} setInterval(update,1000);");
  client.print("</script></head><body><div class=\"container\"><div class=\"glass\">");
  client.print("<div class=\"header\"><h1>WiFi Deauther Pro</h1><div class=\"status-pill\"><div id=\"dt\" class=\"dot\"></div><span id=\"st\">Idle</span></div></div>");
}

void sendFooter(WiFiClient &client) {
  client.print("</div></div></body></html>");
}

void sendNetworkTable(WiFiClient &client, const std::vector<WiFiScanResultUI> &networks) {
  client.print("<table><thead><tr><th>TGT</th><th>SSID</th><th>CH</th><th>RSSI</th></tr></thead><tbody>");
  for (const auto &net : networks) {
    client.print("<tr><td><input type='checkbox' value='" + String(net.index) + "'></td>");
    client.print("<td>" + (net.ssid.length()>0 ? net.ssid : "Hidden") + "</td>");
    client.print("<td>" + String(net.channel) + "</td>");
    client.print("<td>" + String(net.rssi) + "</td></tr>");
  }
  client.print("</tbody></table>");
}

void sendMainUI(WiFiClient &client, const std::vector<WiFiScanResultUI> &n24, const std::vector<WiFiScanResultUI> &n5, bool isDeauthing, uint32_t sent, int frames) {
  client.print("<button id=\"jam-btn\" class=\"big-btn btn-jam\" onclick=\"jam()\" style=\"display:" + String(isDeauthing ? "none" : "flex") + "\">LAUNCH JAMMER</button>");
  client.print("<button id=\"stop-btn\" class=\"big-btn btn-release\" onclick=\"api('/stop')\" style=\"display:" + String(isDeauthing ? "flex" : "none") + "\">RELEASE JAM</button>");
  
  client.print("<div class=\"grid-stats\">");
  client.print("<div class=\"stat-card\"><span class=\"stat-lab\">Packets Sent</span><span id=\"fr\" class=\"stat-val\">" + String(sent) + "</span></div>");
  client.print("<div class=\"stat-card\"><span class=\"stat-lab\">Power Level</span><span class=\"stat-val\">" + String(frames) + "</span></div>");
  client.print("</div>");
  
  client.print("<div class=\"tabs-nav\"><button class=\"tab-btn active\" onclick=\"tab(event,'t24')\">2.4 GHz</button><button class=\"tab-btn\" onclick=\"tab(event,'t5')\">5 GHz</button></div>");
  client.print("<div id=\"t24\" class=\"tab-content\" style=\"display:block\"><div class=\"list-box\">");
  sendNetworkTable(client, n24);
  client.print("</div></div><div id=\"t5\" class=\"tab-content\" style=\"display:none\"><div class=\"list-box\">");
  sendNetworkTable(client, n5);
  client.print("</div></div>");
  
  client.print("<div class=\"settings-row\"><div><div class=\"stat-lab\">Attack Intensity</div><div style=\"font-size:0.7rem;color:var(--text-muted)\">Recommended: 10-20</div></div>");
  client.print("<div style=\"display:flex;align-items:center;gap:8px\"><input type=\"number\" id=\"p-in\" value=\""+String(frames)+"\">");
  client.print("<button class=\"icon-btn\" onclick=\"api('/setframes',{frames:document.getElementById('p-in').value})\">SET</button></div></div>");

  client.print("<div class=\"settings-row\"><div><div class=\"stat-lab\">RGB LED Control</div><div style=\"font-size:0.7rem;color:var(--text-muted)\">Status visualizer</div></div>");
  client.print("<div style=\"display:flex;gap:8px\"><button class=\"icon-btn\" onclick=\"api('/led_enable')\">ON</button><button class=\"icon-btn\" onclick=\"api('/led_disable')\">OFF</button></div></div>");

  client.print("<button class=\"btn btn-rescan\" style=\"width:100%;background:rgba(255,255,255,0.05);color:var(--text-muted);border-radius:12px;border:1px solid var(--border);padding:10px;cursor:pointer\" onclick=\"api('/rescan');location.reload();\">RESCAN NETWORKS</button>");
}

#endif
