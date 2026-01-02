#pragma once
#ifdef ESP32
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>ProBot DriverStation</title>
  <style>
    :root{
      --navy:#00204d;
      --ice:#e5e4e2;
      --start:#28a745;
      --stop:#d93025;
      --sky:#0aa6d9;
      font-family:"Inter","Segoe UI",sans-serif;
    }
    *{margin:0;padding:0;box-sizing:border-box;}
    body{
      min-height:100vh;
      background:var(--ice);
      color:var(--navy);
      display:flex;
      flex-direction:column;
    }
    .app-header{
      position:sticky;
      top:0;
      z-index:100;
      padding:16px 28px;
      display:flex;
      align-items:center;
      gap:16px;
      justify-content:space-between;
      background:var(--navy);
      color:var(--ice);
      box-shadow:0 8px 18px rgba(0,32,77,0.24);
      border-bottom:1px solid rgba(229,228,226,0.12);
    }
    .app-header .header-left{
      display:flex;
      flex-direction:column;
      gap:4px;
    }
    .app-header .header-left h1{
      font-size:1.5rem;
      letter-spacing:0.08em;
    }
    .app-header .header-left .header-subtitle{
      font-size:0.82rem;
      color:rgba(229,228,226,0.8);
      text-transform:uppercase;
      letter-spacing:0.08em;
    }
    .app-header nav{
      display:flex;
      gap:16px;
      flex:1;
      justify-content:center;
    }
    .app-header nav a{
      color:var(--ice);
      text-decoration:none;
      letter-spacing:0.1em;
      font-size:0.85rem;
      text-transform:uppercase;
      opacity:0.82;
      transition:opacity 120ms ease;
    }
    .app-header nav a:hover{opacity:1;}
    .app-header .header-status{
      display:flex;
      flex-direction:row;
      align-items:center;
      gap:6px;
      text-transform:uppercase;
      letter-spacing:0.16em;
    }
    .app-header .header-status .status-label{
      font-size:0.6rem;
      letter-spacing:0.2em;
      opacity:0.7;
    }
    .app-header .header-status .status-value{
      font-size:0.9rem;
      letter-spacing:0.14em;
    }
    .app-header .header-status .status-detail{
      display:none;
    }

    @media(max-width:900px){
      .app-header{
        flex-direction:column;
        align-items:flex-start;
        gap:12px;
        padding:18px 24px;
      }
      .app-header .header-left{
        width:100%;
      }
      .app-header .header-left h1{
        display:none;
      }
      .app-header .header-left .header-subtitle{
        display:none;
      }
      .app-header nav{
        order:3;
        width:100%;
        flex-wrap:wrap;
        justify-content:space-between;
        gap:12px;
      }
      .app-header nav a{
        font-size:0.98rem;
      }
      .app-header .header-status{
        display:none;
      }
    }
    main{
      flex:1;
      background:linear-gradient(180deg,var(--ice) 0%,rgba(229,228,226,0.85) 70%,rgba(229,228,226,0.7) 100%);
      box-shadow:0 -24px 48px rgba(0,32,77,0.12);
      padding:36px 48px 48px;
      display:grid;
      grid-template-columns:minmax(0,1fr) minmax(0,1fr);
      gap:36px;
    }
    .column{
      display:flex;
      flex-direction:column;
      gap:32px;
      min-width:0;
    }
    .stack-card{
      background:var(--ice);
      border-radius:24px;
      padding:28px;
      border:1px solid rgba(0,32,77,0.12);
      box-shadow:0 24px 36px rgba(0,32,77,0.08);
      display:flex;
      flex-direction:column;
      gap:20px;
      scroll-margin-top:120px;
    }
    .stack-card h2{
      font-size:1.1rem;
      letter-spacing:0.12em;
      text-transform:uppercase;
      color:rgba(0,32,77,0.85);
    }
    .control-row{
      display:grid;
      grid-template-columns:repeat(auto-fit,minmax(160px,1fr));
      gap:16px;
    }
    .control{
      background:rgba(229,228,226,0.9);
      border-radius:16px;
      padding:16px;
      border:1px solid rgba(0,32,77,0.08);
      display:flex;
      flex-direction:column;
      gap:10px;
    }
    .control label{
      font-size:0.96rem;
      letter-spacing:0.08em;
      color:rgba(0,32,77,0.65);
    }
    .control input[type="number"]{
      padding:10px;
      border-radius:12px;
      border:1px solid rgba(0,32,77,0.14);
      background:var(--ice);
      text-align:center;
      font-size:1rem;
      color:var(--navy);
    }
    .auto-progress{
      margin-top:16px;
      background:linear-gradient(180deg,rgba(0,32,77,0.08),rgba(0,32,77,0.03));
      border-radius:16px;
      padding:16px;
      border:1px solid rgba(0,32,77,0.12);
      display:flex;
      flex-direction:column;
      gap:12px;
      box-shadow:0 16px 28px rgba(0,32,77,0.08);
    }
    .auto-progress-header{
      display:flex;
      justify-content:space-between;
      align-items:center;
      font-size:0.95rem;
      color:rgba(0,32,77,0.75);
      letter-spacing:0.08em;
      text-transform:uppercase;
    }
    .auto-progress-header strong{
      font-size:1.25rem;
      letter-spacing:0.08em;
      color:rgba(0,32,77,0.8);
    }
    .auto-progress-bar{
      position:relative;
      height:12px;
      border-radius:999px;
      background:rgba(0,32,77,0.1);
      overflow:hidden;
    }
    .auto-progress-fill{
      position:absolute;
      left:0;
      top:0;
      bottom:0;
      width:0%;
      background:linear-gradient(90deg,var(--navy),rgba(0,32,77,0.6));
      border-radius:inherit;
      transition:width 120ms linear;
    }
    .joy-grid{
      display:grid;
      grid-template-columns:160px 1fr;
      gap:20px;
      align-items:center;
    }
    .joy-indicator{
      display:flex;
      flex-direction:column;
      align-items:center;
      gap:8px;
      color:rgba(0,32,77,0.7);
      letter-spacing:0.08em;
      font-size:0.92rem;
    }
    .joy-axes{
      position:relative;
      width:120px;
      height:120px;
      border-radius:50%;
      background:radial-gradient(circle,var(--ice) 0%,rgba(229,228,226,0.85) 65%,rgba(229,228,226,0.7) 100%);
      box-shadow:inset 0 4px 12px rgba(0,32,77,0.12);
    }
    .joy-cross{
      position:absolute;
      left:50%;
      top:50%;
      width:4px;
      height:100%;
      background:rgba(0,32,77,0.2);
      transform:translate(-50%,-50%);
    }
    .joy-cross::before{
      content:"";
      position:absolute;
      left:50%;
      top:50%;
      width:100%;
      height:4px;
      background:rgba(0,32,77,0.2);
      transform:translate(-50%,-50%);
    }
    .joy-dot{
      position:absolute;
      width:14px;
      height:14px;
      border-radius:50%;
      background:var(--navy);
      box-shadow:0 0 10px rgba(0,32,77,0.4);
      transform:translate(-50%,-50%);
      left:50%;
      top:50%;
    }
    .joy-buttons{
      display:grid;
      grid-template-columns:repeat(auto-fit,minmax(48px,1fr));
      gap:12px;
    }
    .joy-button{
      position:relative;
      width:48px;
      height:48px;
      border-radius:50%;
      background:radial-gradient(circle at 30% 30%,rgba(229,228,226,0.95),rgba(0,32,77,0.08));
      border:1px solid rgba(0,32,77,0.18);
      color:rgba(0,32,77,0.75);
      font-size:0.75rem;
      display:flex;
      align-items:center;
      justify-content:center;
      letter-spacing:0.08em;
      text-transform:uppercase;
      box-shadow:0 8px 16px rgba(0,32,77,0.16);
      transition:transform 120ms ease, box-shadow 120ms ease, background 160ms ease;
    }
    .joy-button::after{
      content:attr(data-label);
      position:absolute;
      inset:0;
      display:flex;
      align-items:center;
      justify-content:center;
    }
    .joy-button.active{
      background:radial-gradient(circle at 30% 30%,rgba(229,228,226,0.95),rgba(0,32,77,0.2));
      border-color:rgba(0,32,77,0.7);
      color:var(--navy);
      box-shadow:0 10px 20px rgba(0,32,77,0.35),0 0 12px rgba(0,32,77,0.3);
      transform:translateY(-2px);
    }
    .joy-status{
      margin-top:18px;
      display:flex;
      flex-direction:column;
      gap:4px;
      color:rgba(0,32,77,0.75);
      letter-spacing:0.06em;
    }
    .joy-status strong{
      font-size:1.05rem;
      text-transform:uppercase;
    }
    .switch{
      display:flex;
      align-items:center;
      justify-content:space-between;
      background:rgba(229,228,226,0.85);
      border-radius:16px;
      padding:10px 14px;
    }
    .switch span{
      font-size:0.92rem;
      letter-spacing:0.1em;
      color:rgba(0,32,77,0.7);
    }
    .switch input{
      width:46px;
      height:24px;
      appearance:none;
      background:rgba(0,32,77,0.18);
      border-radius:999px;
      position:relative;
      cursor:pointer;
      transition:background 160ms ease;
    }
    .switch input::after{
      content:"";
      width:20px;
      height:20px;
      border-radius:50%;
      position:absolute;
      top:2px;
      left:3px;
      background:var(--ice);
      box-shadow:0 6px 12px rgba(0,32,77,0.24);
      transition:transform 160ms ease;
    }
    .switch input:checked{
      background:var(--sky);
    }
    .switch input:checked::after{
      transform:translateX(20px);
    }
    button{
      padding:16px;
      border-radius:18px;
      border:none;
      background:var(--navy);
      color:var(--ice);
      font-size:1.14rem;
      font-weight:600;
      letter-spacing:0.12em;
      text-transform:uppercase;
      cursor:pointer;
      box-shadow:0 18px 26px rgba(0,32,77,0.3);
      transition:transform 140ms ease, box-shadow 140ms ease, background 140ms ease;
    }
    button:hover{transform:translateY(-2px);}
    button:active{transform:translateY(1px);box-shadow:0 12px 18px rgba(0,32,77,0.24);}
    .telemetry{
      display:flex;
      flex-direction:column;
      gap:16px;
    }
    select, .telemetry pre{
      width:100%;
      padding:14px;
      border-radius:16px;
      border:1px solid rgba(0,32,77,0.12);
      background:var(--ice);
      font-size:1.05rem;
      color:var(--navy);
      letter-spacing:0.04em;
    }
    .telemetry pre{
      height:140px;
      overflow:auto;
      background:rgba(229,228,226,0.92);
      font-family:"SFMono-Regular","Roboto Mono",monospace;
      line-height:1.5;
      white-space:pre-wrap;
      word-break:break-word;
    }
    .hint{
      font-size:0.88rem;
      color:rgba(0,32,77,0.6);
      letter-spacing:0.06em;
    }
    @media(max-width:992px){
      .app-header{
        padding:16px 28px;
        gap:18px;
      }
      .app-header .header-left h1{
        font-size:1.7rem;
      }
      .app-header .header-left .header-subtitle{
        font-size:0.9rem;
        letter-spacing:0.08em;
      }
      .app-header nav{
        gap:16px;
      }
      .app-header nav a{
        font-size:0.9rem;
        letter-spacing:0.1em;
      }
      .app-header .header-status{
        align-items:flex-start;
        gap:6px;
      }
      .app-header .header-status .status-label{
        font-size:0.62rem;
        letter-spacing:0.18em;
      }
      .app-header .header-status .status-value{
        font-size:0.98rem;
      }
      .app-header .header-status .status-detail{
        font-size:0.74rem;
      }
    }
    @media(max-width:900px){
      main{
        grid-template-columns:1fr;
        padding:24px 24px 32px;
        gap:24px;
      }
      .column{gap:24px;}
      .app-header{
        flex-direction:column;
        align-items:flex-start;
        gap:12px;
        padding:18px 24px;
      }
      .app-header .header-left h1{font-size:1.8rem;}
      .app-header .header-left .header-subtitle{font-size:0.88rem;}
      .app-header nav{
        order:3;
        width:100%;
        flex-wrap:wrap;
        gap:12px;
        justify-content:space-between;
      }
      .app-header nav a{
        font-size:0.98rem;
        letter-spacing:0.09em;
      }
      .app-header .header-status{
        display:none;
      }
      .stack-card h2{
        font-size:1.2rem;
      }
    }
    @media(orientation:portrait){
      main{
        grid-template-columns:1fr;
        padding:36px 36px 48px;
        gap:36px;
      }
      .column{gap:36px;}
      .app-header{
        flex-direction:column;
        align-items:flex-start;
        gap:16px;
        padding:18px 24px;
        justify-content:flex-start;
      }
      .app-header nav{
        order:3;
        width:100%;
        flex-wrap:wrap;
        justify-content:space-between;
        gap:18px;
      }
      .app-header nav a{
        font-size:1.15rem;
        letter-spacing:0.08em;
      }
      .app-header .header-left{
        gap:6px;
        width:100%;
      }
      .app-header .header-left h1{
        display:none;
      }
      .app-header .header-left .header-subtitle{
        display:none;
      }
      .app-header nav{
        gap:18px;
      }
      .app-header nav a{
        font-size:1.15rem;
      }
      .app-header .header-status{
        display:none;
      }
      body{
        font-size:1.9rem;
        line-height:1.5;
      }
      .stack-card h2{
        font-size:1.35rem;
        letter-spacing:0.12em;
      }
      .control-row{
        grid-template-columns:1fr;
        gap:20px;
      }
      button{
        font-size:2.4rem;
        padding:24px;
      }
      #robotButton{
        padding:109px 42px;
        font-size:4.7rem;
      }
      .control label{font-size:1.8rem;}
      .control input[type="number"]{
        font-size:2rem;
        padding:16px;
      }
      .switch{
        flex-direction:row;
        align-items:center;
        gap:18px;
        padding:14px 18px;
      }
      .switch span{font-size:1.6rem;}
      .switch input{
        width:88px;
        height:46px;
      }
      .switch input::after{
        width:38px;
        height:38px;
        top:4px;
        left:6px;
      }
      .switch input:checked::after{
        transform:translateX(42px);
      }
      select, .telemetry pre{
        font-size:1.9rem;
        padding:22px;
      }
      .telemetry pre{
        line-height:1.6;
      }
      .auto-progress-header{
        font-size:1.8rem;
        letter-spacing:0.05em;
      }
      .auto-progress-header strong{
        font-size:2.6rem;
      }
      .hint{font-size:1.6rem;}
      .joy-grid{
        grid-template-columns:1fr;
        gap:24px;
      }
      .joy-indicator{font-size:1.5rem;}
      .joy-axes{
        width:220px;
        height:220px;
        margin:0 auto;
      }
      .joy-buttons{
        grid-template-columns:repeat(auto-fit,minmax(84px,1fr));
        gap:16px;
      }
      .joy-button{
        width:84px;
        height:84px;
        font-size:1.4rem;
        margin:0 auto;
      }
      .joy-status strong{font-size:1.8rem;}
    }
    @media(max-width:1024px) and (orientation:landscape){
      .app-header{
        flex-direction:row;
        align-items:center;
        justify-content:center;
        padding:6px 18px;
        gap:12px;
      }
      .app-header .header-left,
      .app-header .header-status{
        display:none;
      }
      .app-header nav{
        width:auto;
        flex:0 1 auto;
        gap:12px;
        flex-wrap:wrap;
        justify-content:center;
      }
      .app-header nav a{
        font-size:0.78rem;
        letter-spacing:0.14em;
      }
      main{
        grid-template-columns:minmax(0,1fr) minmax(0,1fr);
        gap:20px;
        padding:16px 20px 22px;
      }
      .column{gap:20px;}
    }
  </style>
</head>
<body>
  <header class="app-header" id="appHeader">
    <div class="header-left">
      <h1>ProBot DriverStation</h1>
      <span class="header-subtitle">by NFR Products · c2025</span>
    </div>
    <nav>
      <a href="#dashboard">Dashboard</a>
      <a href="#logs">Logs</a>
      <a href="#settings">Settings</a>
    </nav>
    <div class="header-status">
      <span class="status-label">Status</span>
      <span class="status-value" id="headerStatusValue">Standby</span>
      <span class="status-detail" id="headerStatusDetail">Awaiting command</span>
    </div>
  </header>
<main>
  <div class="column column-primary">
    <section class="stack-card" id="dashboard">
      <h2>Match Control</h2>
      <div class="control-row">
        <button id="robotButton">Init</button>
        <div class="control">
          <label>Autonomous Duration</label>
          <input type="number" id="autoPeriod" value="30" min="1" max="120">
          <span class="hint">Seconds</span>
        </div>
        <div class="switch">
          <span>Autonomous</span>
          <input type="checkbox" id="enableAutonomous" checked>
        </div>
      </div>
      <div class="auto-progress">
        <div class="auto-progress-header">
          <span>Autonomous Countdown</span>
          <strong id="autoCountdown">00.0 s</strong>
        </div>
        <div class="auto-progress-bar">
          <div class="auto-progress-fill" id="autoProgress"></div>
        </div>
      </div>
    </section>
    <section class="stack-card" id="settings">
      <h2>Joystick Check</h2>
      <div class="joy-grid">
        <div class="joy-indicator">
          <div class="joy-axes">
            <div class="joy-dot" id="joyDot"></div>
            <div class="joy-cross"></div>
          </div>
          <span>Axes</span>
        </div>
        <div class="joy-buttons" id="joyButtons"></div>
      </div>
      <div class="joy-status" id="joyStatus">
        <strong id="joystickStatusTxt">Not Connected</strong>
        <p class="hint" id="gamepadHint" style="display:none;">Press any controller button to activate.</p>
      </div>
    </section>
  </div>
  <div class="column column-secondary">
    <section class="stack-card" id="telemetry-panel">
      <h2>Telemetry</h2>
      <pre id="telemetryOutput" style="height:150px;overflow-y:auto;background:rgba(0,32,77,0.05);padding:12px;border-radius:12px;font-size:0.9rem;"></pre>
      <button onclick="clearTelemetry()" style="margin-top:12px;padding:10px 20px;font-size:0.9rem;">Clear</button>
    </section>
    <section class="stack-card telemetry" id="logs">
      <h2>System Logs</h2>
      <select id="joystickSelect" onchange="changeSelectedGamepad()">
        <option value="-1">No Gamepad</option>
      </select>
      <pre id="joystickStatus">No gamepad selected.</pre>
      <pre id="axisData">No axis data...</pre>
      <pre id="buttonData">No button data...</pre>
    </section>
  </div>
</main>
  <script>
    let controlState="idle";
    let autoModeEnabled=false;
    let selectedGamepadIndex=-1;
    let gamepads={};
    let gamepadDetected=false;

    const headerEl=document.getElementById('appHeader');
const headerStatusValue=document.getElementById('headerStatusValue');
const headerStatusDetail=document.getElementById('headerStatusDetail');

function setPhaseDisplay(mode){
  const map={
    init:{title:"Init", detail:"Systems primed"},
    auto:{title:"Autonomous", detail:"Running script"},
    teleop:{title:"Teleop", detail:"Drivers in control"},
    stopped:{title:"Stopped", detail:"Motors safe"},
    standby:{title:"Standby", detail:"Awaiting command"}
  };
  const next = map[mode] || map.standby;
  if(headerStatusValue){
    headerStatusValue.textContent = next.title;
  }
  if(headerStatusDetail){
    if(mode === 'auto'){
      headerStatusDetail.textContent = `Auto running – ${autoRemaining.toFixed(1)} s`;
    }else{
      headerStatusDetail.textContent = next.detail;
    }
  }
  document.body.dataset.state = mode;
}

let autoTimer=null;
let autoRemaining=0;
const DEFAULT_BUTTON_COUNT=12;
const BUTTON_LABELS=["A","B","X","Y","LB","RB","LT","RT","Back","Start","L3","R3","P13","P14","P15","P16","P17","P18","P19","P20"];

function ensureJoyButtons(count){
  const container=document.getElementById('joyButtons');
  if(container.childElementCount === count) return;
  container.innerHTML="";
  for(let i=0;i<count;i++){
    const cell=document.createElement('div');
    cell.className='joy-button';
    const label=BUTTON_LABELS[i] || `B${i+1}`;
    cell.dataset.label=label;
    container.appendChild(cell);
  }
}

function updateJoyVisuals(gp){
  const dot=document.getElementById('joyDot');
  const container=document.getElementById('joyButtons');
  if(gp){
    const x=gp.axes && gp.axes.length>0 ? gp.axes[0] : 0;
    const y=gp.axes && gp.axes.length>1 ? gp.axes[1] : 0;
    dot.style.left=`${50 + x*45}%`;
    dot.style.top=`${50 + y*45}%`;
    ensureJoyButtons(gp.buttons.length);
    gp.buttons.forEach((btn,i)=>{
      const node=container.children[i];
      if(node) node.classList.toggle('active', !!btn.pressed);
    });
  }else{
    ensureJoyButtons(DEFAULT_BUTTON_COUNT);
    dot.style.left='50%';
    dot.style.top='50%';
    Array.from(container.children).forEach(node=>node.classList.remove('active'));
  }
}

function updateAutoDisplay(){
  const display=document.getElementById('autoCountdown');
  const fill=document.getElementById('autoProgress');
  const duration=parseFloat(document.getElementById('autoPeriod').value)||0;
  display.textContent=`${autoRemaining.toFixed(1)} s`;
  const pct = duration>0 ? Math.max(0,Math.min(100,(autoRemaining/duration)*100)) : 0;
  fill.style.width=`${pct}%`;
  if(autoModeEnabled){
    setPhaseDisplay('auto');
  }
}

function startAutoTimer(duration){
  autoModeEnabled = true;
  autoRemaining = duration;
  updateAutoDisplay();
  clearInterval(autoTimer);
  autoTimer = setInterval(()=>{
    autoRemaining = Math.max(0, autoRemaining - 0.1);
    updateAutoDisplay();
    if(autoRemaining <= 0){
      clearInterval(autoTimer);
      autoTimer=null;
      autoModeEnabled = false;
      setPhaseDisplay('teleop');
    }
  }, 100);
}

function stopAutoTimer(){
  clearInterval(autoTimer);
  autoTimer=null;
  autoRemaining=0;
  autoModeEnabled=false;
  updateAutoDisplay();
}

    async function handleRobotButton(){
      let cmd="";
      const enableAuto=document.getElementById('enableAutonomous').checked;
      const autoLen=Math.max(0, parseFloat(document.getElementById('autoPeriod').value) || 0);

      switch(controlState){
        case "idle": cmd="init"; break;
        case "armed": cmd="start"; break;
        default: cmd="stop"; break;
      }

      const url=`/robotControl?cmd=${cmd}&auto=${enableAuto?1:0}&autoLen=${autoLen}`;
      try{
        const r=await fetch(url);
        if(!r.ok) throw new Error("Robot command failed");
      }catch(err){
        console.error(err);
        return;
      }

      const btn=document.getElementById('robotButton');
      if(controlState==="idle"){
        controlState="armed";
        btn.textContent="Start";
        btn.style.background="var(--start)";
        btn.style.color="var(--ice)";
        stopAutoTimer();
        autoRemaining = autoLen;
        updateAutoDisplay();
        setPhaseDisplay('init');
      }else if(controlState==="armed"){
        controlState="running";
        btn.textContent="Stop";
        btn.style.background="var(--stop)";
        btn.style.color="var(--ice)";
        if(enableAuto && autoLen>0){
          startAutoTimer(autoLen);
          setPhaseDisplay('auto');
        }else{
          stopAutoTimer();
          setPhaseDisplay('teleop');
        }
      }else{
        controlState="idle";
        btn.textContent="Init";
        btn.style.background="var(--navy)";
        btn.style.color="var(--ice)";
        stopAutoTimer();
        setPhaseDisplay('stopped');
      }
    }
    document.getElementById('robotButton').addEventListener('click',handleRobotButton);

    function updateGamepads(){
      const gpList=navigator.getGamepads?navigator.getGamepads():[];
      gamepads={};
      for(let i=0;i<gpList.length;i++){
        const gp=gpList[i];
        if(gp) gamepads[gp.index]=gp;
      }
    }
    function rebuildGamepadSelect(){
      const selectEl=document.getElementById('joystickSelect');
      while(selectEl.options.length>1){ selectEl.remove(1); }
      Object.keys(gamepads).forEach(idx=>{
        const gp=gamepads[idx];
        const option=document.createElement('option');
        option.value=idx;
        option.text=`${gp.id} (idx ${gp.index})`;
        selectEl.add(option);
      });
      if(Object.keys(gamepads).length>0 && selectedGamepadIndex<0){
        const firstIdx=Object.keys(gamepads)[0];
        selectEl.value=firstIdx;
        selectedGamepadIndex=parseInt(firstIdx,10);
      }
      if(Object.keys(gamepads).length===0){
        selectedGamepadIndex=-1;
        selectEl.value="-1";
      }
    }
    function changeSelectedGamepad(){
      const val=document.getElementById('joystickSelect').value;
      selectedGamepadIndex=parseInt(val,10);
      if(isNaN(selectedGamepadIndex)) selectedGamepadIndex=-1;
    }
    let gamepadSending=false;
    let lastGamepadSend=0;
    const GAMEPAD_SEND_INTERVAL=20; // 50Hz max
    async function sendGamepadData(gp){
      const now=performance.now();
      if(gamepadSending || (now-lastGamepadSend)<GAMEPAD_SEND_INTERVAL) return;
      gamepadSending=true;
      lastGamepadSend=now;
      const data={axes:Array.from(gp.axes),buttons:gp.buttons.map(b=>b.pressed)};
      try{
        await fetch("/updateController",{
          method:"POST",
          headers:{"Content-Type":"application/json"},
          body:JSON.stringify(data)
        });
      }catch(err){
        console.error(err);
      }finally{
        gamepadSending=false;
      }
    }
    function displayJoystickData(gp){
      document.getElementById('joystickStatus').textContent=`${gp.id}\nAxes:${gp.axes.length} Buttons:${gp.buttons.length}`;
      document.getElementById('axisData').textContent=gp.axes.map((v,i)=>`Axis ${i}: ${v.toFixed(2)}`).join("\n");
      document.getElementById('buttonData').textContent=gp.buttons.map((b,i)=>`${b.pressed?"●":"○"} ${i}`).join("  ");
      document.getElementById('joystickStatusTxt').textContent="Connected";
      updateJoyVisuals(gp);
    }
    function gamepadLoop(){
      updateGamepads();
      rebuildGamepadSelect();
      const txt=document.getElementById('joystickStatusTxt');
      const hint=document.getElementById('gamepadHint');
      const gp=gamepads[selectedGamepadIndex];
      if(gp){
        txt.textContent="Connected";
        hint.style.display="none";
        gamepadDetected=true;
        displayJoystickData(gp);
        sendGamepadData(gp);
      }else{
        txt.textContent="Not Connected";
        if(!gamepadDetected) hint.style.display="block";
        document.getElementById('joystickStatus').textContent="No gamepad selected.";
        document.getElementById('axisData').textContent="No axis data...";
        document.getElementById('buttonData').textContent="No button data...";
        updateJoyVisuals(null);
      }
      requestAnimationFrame(gamepadLoop);
    }

    window.addEventListener('gamepadconnected',()=>{
      updateGamepads();
      rebuildGamepadSelect();
    });
    window.addEventListener('gamepaddisconnected',e=>{
      delete gamepads[e.gamepad.index];
    });
    window.addEventListener('load',()=>{
      ensureJoyButtons(DEFAULT_BUTTON_COUNT);
      updateJoyVisuals(null);
      autoRemaining=parseFloat(document.getElementById('autoPeriod').value)||0;
      updateAutoDisplay();
      setPhaseDisplay('standby');
      requestAnimationFrame(gamepadLoop);
    });

    document.getElementById('autoPeriod').addEventListener('input',e=>{
      const duration=parseFloat(e.target.value)||0;
      if(autoTimer){
        autoRemaining=Math.min(autoRemaining,duration);
      } else {
        autoRemaining=duration;
      }
      updateAutoDisplay();
    });

    document.getElementById('enableAutonomous').addEventListener('change',e=>{
      if(!e.target.checked){
        stopAutoTimer();
      }
    });

    // Telemetry polling
    async function pollTelemetry(){
      try{
        const r=await fetch('/telemetry');
        if(r.ok){
          const text=await r.text();
          const el=document.getElementById('telemetryOutput');
          if(el && text) el.textContent=text;
        }
      }catch(e){}
    }
    function clearTelemetry(){
      const el=document.getElementById('telemetryOutput');
      if(el) el.textContent='';
    }
    setInterval(pollTelemetry,50);
</script>
</body>
</html>

)=====";
