#pragma once
#ifdef ESP32
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>ProBot UI</title>
  <style>
    body { margin: 0; font-family: Arial, sans-serif; display: flex; height: 100vh; }
    #sidebar { width: 200px; background: #444; color: #fff; padding: 20px; box-sizing: border-box; }
    #sidebar h2 { margin-top: 0; }
    #sidebar a { display: block; color: #ccc; text-decoration: none; margin: 8px 0; }
    #sidebar a:hover { color: #fff; }
    #content { flex: 1; padding: 20px; box-sizing: border-box; }
    .page-section { display: none; }
    .page-section.active { display: block; }
    .indicator { display: inline-block; width: 16px; height: 16px; border-radius: 50%; margin-left: 8px; vertical-align: middle; background: red; }
    #axisData, #buttonData { border: 1px solid #ccc; background: #f9f9f9; padding: 10px; margin: 10px 0; }
    .axisItem, .buttonItem { margin-bottom: 4px; }
    #robotButton { font-size: 1.2em; padding: 10px 20px; margin-top: 10px; }
  </style>
</head>
<body>
  <div id="sidebar">
    <h2>Menu</h2>
    <a href="#" onclick="showPage('mainDrive'); return false;">Main Drive</a>
    <a href="#" onclick="showPage('joystickTest'); return false;">Joystick Test</a>
  </div>
  <div id="content">
    <div id="mainDrive" class="page-section active">
      <h1>Main Drive</h1>
      <p><strong>Joystick Status:</strong> <span id="joystickStatusTxt">Not Connected</span> <span id="joystickIndicator" class="indicator"></span></p>
      <p><label><input type="checkbox" id="enableAutonomous" /> Enable Autonomous</label></p>
      <p><label>Autonomous Period Length: <input type="number" id="autoPeriod" value="30" style="width:60px;" /> (s)</label></p>
      <p><button id="robotButton" onclick="handleRobotButton()">Init</button></p>
    </div>
    <div id="joystickTest" class="page-section">
      <h1>Joystick Test</h1>
      <p>Connect one or more controllers, press a button to register them, then pick which to use:
        <select id="joystickSelect" onchange="changeSelectedGamepad()">
          <option value="-1">No Gamepad</option>
        </select>
      </p>
      <h2>Selected Gamepad Info</h2>
      <div id="joystickStatus">No gamepad selected.</div>
      <h3>Axes</h3>
      <div id="axisData">No data yet...</div>
      <h3>Buttons</h3>
      <div id="buttonData">No data yet...</div>
    </div>
  </div>
  <script>
    function showPage(pageId){ const sections=document.querySelectorAll('.page-section'); sections.forEach(sec=>sec.classList.remove('active')); document.getElementById(pageId).classList.add('active'); }
    let robotState = "init"; let selectedGamepadIndex = -1; let gamepads = {};
    async function handleRobotButton(){ let cmd=""; switch(robotState){ case "init":cmd="init"; break; case "start":cmd="start"; break; case "stop":cmd="stop"; break;} const enableAuto=document.getElementById('enableAutonomous').checked; const autoLen=document.getElementById('autoPeriod').value; const url = `/robotControl?cmd=${cmd}&auto=${enableAuto?1:0}&autoLen=${autoLen}`; try{ const r=await fetch(url); if(!r.ok) return; }catch(e){ console.error(e); return; } if(robotState==="init"){ robotState="start"; document.getElementById('robotButton').textContent="Start"; } else if(robotState==="start"){ robotState="stop"; document.getElementById('robotButton').textContent="Stop"; } else { robotState="init"; document.getElementById('robotButton').textContent="Init"; } }
    function updateGamepads(){ const gpList=navigator.getGamepads?navigator.getGamepads():[]; gamepads={}; for(let i=0;i<gpList.length;i++){ const gp=gpList[i]; if(gp) gamepads[gp.index]=gp; } }
    function rebuildGamepadSelect(){ const selectEl=document.getElementById('joystickSelect'); while(selectEl.options.length>1){ selectEl.remove(1);} for(let idx in gamepads){ const gp=gamepads[idx]; const option=document.createElement('option'); option.value=idx; option.text=`${gp.id} (index ${gp.index})`; selectEl.add(option);} if(Object.keys(gamepads).length>0 && selectedGamepadIndex<0){ const firstIdx=Object.keys(gamepads)[0]; selectEl.value=firstIdx; selectedGamepadIndex=parseInt(firstIdx);} }
    function changeSelectedGamepad(){ const val=document.getElementById('joystickSelect').value; selectedGamepadIndex=parseInt(val); if(isNaN(selectedGamepadIndex)) selectedGamepadIndex=-1; }
    async function sendGamepadData(gp){ if(robotState!=="stop") return; const data={ axes: gp.axes, buttons: gp.buttons.map(b=>b.pressed) }; try{ await fetch('/updateController',{ method:'POST', headers:{'Content-Type':'application/json'}, body: JSON.stringify(data)});}catch(e){ console.error(e);} }
    function displayJoystickData(gp){ const statusEl=document.getElementById('joystickStatus'); const axisEl=document.getElementById('axisData'); const buttonEl=document.getElementById('buttonData'); statusEl.textContent=`Gamepad: ${gp.id} (Axes: ${gp.axes.length}, Buttons: ${gp.buttons.length})`; let axisHTML=""; gp.axes.forEach((v,i)=>{ axisHTML+=`<div class="axisItem">Axis ${i}: ${v.toFixed(2)}</div>`; }); axisEl.innerHTML=axisHTML; let btnHTML=""; gp.buttons.forEach((b,i)=>{ btnHTML+=`<div class="buttonItem">Button ${i}: ${b.pressed?'Pressed':'Not Pressed'}</div>`; }); buttonEl.innerHTML=btnHTML; }
    function gamepadLoop(){ updateGamepads(); rebuildGamepadSelect(); const gp=gamepads[selectedGamepadIndex]; const indicator=document.getElementById('joystickIndicator'); const txt=document.getElementById('joystickStatusTxt'); if(gp){ indicator.style.background="green"; txt.textContent="Connected"; displayJoystickData(gp); sendGamepadData(gp);} else { indicator.style.background="red"; txt.textContent="Not Connected"; document.getElementById('joystickStatus').textContent="No gamepad selected."; document.getElementById('axisData').innerHTML="No data yet..."; document.getElementById('buttonData').innerHTML="No data yet..."; } requestAnimationFrame(gamepadLoop); }
    window.addEventListener('gamepadconnected', e=>{ console.log('Gamepad connected:', e.gamepad); });
    window.addEventListener('gamepaddisconnected', e=>{ console.log('Gamepad disconnected:', e.gamepad); delete gamepads[e.gamepad.index]; });
    window.addEventListener('load', ()=>{ requestAnimationFrame(gamepadLoop); });
  </script>
</body>
</html>
)====="; 