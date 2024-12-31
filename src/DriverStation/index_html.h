// Single HTML page with sidebar navigation:
// - Main Drive (joystick status, battery voltage, init/start/stop button)
// - Joystick Test (shows axes & buttons)
// - Settings (checkbox placeholder)
//
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
  <title>ProBot Multi-Page</title>
  <style>
    body {
      margin: 0;
      font-family: Arial, sans-serif;
      display: flex;
      height: 100vh;
    }
    /* Sidebar styles */
    #sidebar {
      width: 200px;
      background: #444;
      color: #fff;
      padding: 20px;
      box-sizing: border-box;
    }
    #sidebar h2 {
      margin-top: 0;
    }
    #sidebar a {
      display: block;
      color: #ccc;
      text-decoration: none;
      margin: 8px 0;
    }
    #sidebar a:hover {
      color: #fff;
    }
    /* Content area */
    #content {
      flex: 1;
      padding: 20px;
      box-sizing: border-box;
    }
    .page-section {
      display: none; /* Hidden by default; shown via JS .active */
    }
    .page-section.active {
      display: block;
    }
    /* Joystick indicator (circle) */
    .indicator {
      display: inline-block;
      width: 16px;
      height: 16px;
      border-radius: 50%;
      margin-left: 8px;
      vertical-align: middle;
      background: red; /* default is red (no joystick) */
    }
    /* Battery box */
    .batteryBox {
      background: #f9f9f9;
      border: 1px solid #ccc;
      padding: 8px 12px;
      display: inline-block;
      margin: 8px 0;
    }
    .disabledNotice {
      color: red;
      font-weight: bold;
      margin-left: 8px;
    }
    .inlineLabel {
      display: inline-block;
      width: 180px;
    }
    /* Make the Init/Start/Stop button BIG */
    #robotButton {
      font-size: 1.2em;
      padding: 10px 20px;
      margin-top: 10px;
    }
    /* Joystick Test area */
    #axisData, #buttonData {
      border: 1px solid #ccc;
      background: #f9f9f9;
      padding: 10px;
      margin: 10px 0;
    }
    .axisItem, .buttonItem {
      margin-bottom: 4px;
    }
  </style>
</head>
<body>

  <!-- SIDEBAR -->
  <div id="sidebar">
    <h2>Menu</h2>
    <a href="#" onclick="showPage('mainDrive'); return false;">Main Drive</a>
    <a href="#" onclick="showPage('joystickTest'); return false;">Joystick Test</a>
    <a href="#" onclick="showPage('settings'); return false;">Settings</a>
  </div>

  <!-- CONTENT AREA -->
  <div id="content">
    <!-- MAIN DRIVE PAGE -->
    <div id="mainDrive" class="page-section active">
      <h1>Main Drive</h1>

      <!-- Joystick status indicator -->
      <p>
        <strong>Joystick Status:</strong>
        <span id="joystickStatusTxt">Not Connected</span>
        <span id="joystickIndicator" class="indicator"></span>
      </p>

      <!-- Battery Voltage -->
      <p>
        <strong>Battery Voltage:</strong>
        <span id="batteryVoltage" class="batteryBox">--</span>
        <span id="lowBatteryNotice" class="disabledNotice" style="display:none;">
          Battery Too Low!
        </span>
      </p>

      <!-- Enable Autonomous + Period input -->
      <p>
        <label>
          <input type="checkbox" id="enableAutonomous" />
          Enable Autonomous
        </label>
      </p>
      <p>
        <label class="inlineLabel">Autonomous Period Length:</label>
        <input type="number" id="autoPeriod" value="30" style="width:60px;" /> (seconds)
      </p>

      <!-- Robot Button (Init -> Start -> Stop) -->
      <p>
        <button id="robotButton" onclick="handleRobotButton()">Init</button>
      </p>

    </div>

    <!-- JOYSTICK TEST PAGE -->
    <div id="joystickTest" class="page-section">
      <h1>Joystick Test</h1>
      <p>
        Connect one or more controllers, press a button to register them, then pick which to use:
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

    <!-- SETTINGS PAGE -->
    <div id="settings" class="page-section">
      <h1>Settings</h1>
      <!-- On-Screen Joystick (enabled by default) -->
      <p>
        <label>
          <input type="checkbox" id="onScreenJoystick" checked />
          Enable On-Screen Joystick (not implemented yet)
        </label>
      </p>
      <!-- NEW: "Allow robot to run if the battery is too low" -->
      <p>
        <label>
          <input type="checkbox" id="allowLowBattery" />
          Allow robot to run if the battery is too low
        </label>
      </p>
      <p>(You can fill in additional settings later.)</p>
    </div>
  </div>

  <script>
    /*******************************************************
     * PAGE NAVIGATION (show/hide sections)
     *******************************************************/
    function showPage(pageId) {
      const sections = document.querySelectorAll('.page-section');
      sections.forEach(sec => sec.classList.remove('active'));
      document.getElementById(pageId).classList.add('active');
    }

    /*******************************************************
     * GLOBALS
     *******************************************************/
    let robotState = "init";   // can be init, start, stop
    let batteryValue = 0;      // read from server
    let selectedGamepadIndex = -1; // which gamepad user selected (-1 = none)

    /*******************************************************
     * MAIN DRIVE: ROBOT BUTTON (Init → Start → Stop → Init)
     * Now also respects "allowLowBattery" setting
     *******************************************************/
    async function handleRobotButton() {
      const allowLowBattery = document.getElementById('allowLowBattery').checked;
      if (!allowLowBattery && batteryValue < 11.0) {
        alert("Battery too low to proceed! (Change settings if you want to allow.)");
        return;
      }

      let cmd = "";
      switch(robotState) {
        case "init":  cmd = "init";  break;
        case "start": cmd = "start"; break;
        case "stop":  cmd = "stop";  break;
      }

      const enableAuto = document.getElementById('enableAutonomous').checked;
      const autoLen    = document.getElementById('autoPeriod').value;

      const url = `/robotControl?cmd=${cmd}&auto=${enableAuto ? 1 : 0}&autoLen=${autoLen}`;
      try {
        const response = await fetch(url);
        if (!response.ok) {
          console.error("Failed to send robot command", cmd);
          return;
        }
      } catch(err) {
        console.error("Error:", err);
        return;
      }

      // Cycle the robot state
      if (robotState === "init") {
        robotState = "start";
        document.getElementById('robotButton').textContent = "Start";
      } else if (robotState === "start") {
        robotState = "stop";
        document.getElementById('robotButton').textContent = "Stop";
      } else if (robotState === "stop") {
        robotState = "init";
        document.getElementById('robotButton').textContent = "Init";
      }
    }

    /*******************************************************
     * MAIN DRIVE: BATTERY VOLTAGE
     * Fetched every 2s from /getBattery
     *******************************************************/
    async function updateBatteryVoltage() {
      try {
        const resp = await fetch('/getBattery');
        if (!resp.ok) return;
        const voltageStr = await resp.text(); // e.g. "12.3"
        batteryValue = parseFloat(voltageStr);
        const batteryEl = document.getElementById('batteryVoltage');
        batteryEl.textContent = batteryValue.toFixed(1) + " V";

        const allowLowBattery = document.getElementById('allowLowBattery').checked;
        const lowBatNotice = document.getElementById('lowBatteryNotice');

        // Show "Battery Too Low!" if < 11.0, unless user allows low battery
        if (!allowLowBattery && batteryValue < 11.0) {
          lowBatNotice.style.display = "inline";
        } else {
          lowBatNotice.style.display = "none";
        }
      } catch (err) {
        console.error("Battery fetch error:", err);
      }
    }

    /*******************************************************
     * JOYSTICK / GAMEPAD HANDLING
     * We can connect multiple gamepads, user picks one from a <select>
     *******************************************************/
    let gamepads = {};

    function updateGamepads() {
      const gpList = navigator.getGamepads ? navigator.getGamepads() : [];
      // Clear gamepads object
      gamepads = {};
      for (let i = 0; i < gpList.length; i++) {
        const gp = gpList[i];
        if (gp) {
          gamepads[gp.index] = gp;
        }
      }
    }

    // Build the <select> dropdown with all recognized gamepads
    function rebuildGamepadSelect() {
      const selectEl = document.getElementById('joystickSelect');
      // Clear out old options except the first
      while (selectEl.options.length > 1) {
        selectEl.remove(1); 
      }
      // Add each recognized gamepad
      for (let idx in gamepads) {
        const gp = gamepads[idx];
        const option = document.createElement('option');
        option.value = idx;
        option.text = `${gp.id} (index ${gp.index})`;
        selectEl.add(option);
      }
      // If there's at least one gamepad, default to the first
      if (Object.keys(gamepads).length > 0 && selectedGamepadIndex < 0) {
        const firstIdx = Object.keys(gamepads)[0];
        selectEl.value = firstIdx;
        selectedGamepadIndex = parseInt(firstIdx);
      }
    }

    // Called when user picks a new gamepad from the dropdown
    function changeSelectedGamepad() {
      const val = document.getElementById('joystickSelect').value;
      selectedGamepadIndex = parseInt(val);
      if (isNaN(selectedGamepadIndex)) {
        selectedGamepadIndex = -1;
      }
    }

    // Send full data to /updateController if robotState == "stop"
    // stop olduğu zaman robot start konumunda oluyor
    async function sendGamepadData(gp) {
      if (robotState !== "stop") return; // only send if started
      const data = {
        axes: gp.axes,
        buttons: gp.buttons.map(btn => btn.pressed)
      };
      try {
        await fetch('/updateController', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify(data)
        });
      } catch (err) {
        console.error('Error sending controller data:', err);
      }
    }

    function displayJoystickData(gp) {
      // Joystick Test page
      const statusEl  = document.getElementById('joystickStatus');
      const axisEl    = document.getElementById('axisData');
      const buttonEl  = document.getElementById('buttonData');

      statusEl.textContent = `Gamepad: ${gp.id} (Axes: ${gp.axes.length}, Buttons: ${gp.buttons.length})`;

      let axisHTML = "";
      gp.axes.forEach((val, idx) => {
        axisHTML += `<div class="axisItem">Axis ${idx}: ${val.toFixed(2)}</div>`;
      });
      axisEl.innerHTML = axisHTML;

      let btnHTML = "";
      gp.buttons.forEach((btn, idx) => {
        btnHTML += `<div class="buttonItem">Button ${idx}: ${btn.pressed ? 'Pressed' : 'Not Pressed'}</div>`;
      });
      buttonEl.innerHTML = btnHTML;
    }

    // Joystick loop ~60 FPS
    function gamepadLoop() {
      updateGamepads();
      rebuildGamepadSelect();

      // Decide which gamepad to use
      const gp = gamepads[selectedGamepadIndex];
      // Main Drive page indicator
      const indicator = document.getElementById('joystickIndicator');
      const indicatorTxt = document.getElementById('joystickStatusTxt');

      if (gp) {
        indicator.style.background = "green";
        indicatorTxt.textContent = "Connected";
        displayJoystickData(gp);
        sendGamepadData(gp);
      } else {
        indicator.style.background = "red";
        indicatorTxt.textContent = "Not Connected";
        // Joystick Test page display
        document.getElementById('joystickStatus').textContent = "No gamepad selected.";
        document.getElementById('axisData').innerHTML = "No data yet...";
        document.getElementById('buttonData').innerHTML = "No data yet...";
      }

      requestAnimationFrame(gamepadLoop);
    }

    // Detect connect/disconnect
    window.addEventListener('gamepadconnected', e => {
      console.log("Gamepad connected:", e.gamepad);
    });
    window.addEventListener('gamepaddisconnected', e => {
      console.log("Gamepad disconnected:", e.gamepad);
      delete gamepads[e.gamepad.index];
    });

    /*******************************************************
     * ON PAGE LOAD
     *******************************************************/
    window.addEventListener('load', () => {
      requestAnimationFrame(gamepadLoop);
      updateBatteryVoltage();
      setInterval(updateBatteryVoltage, 2000);
    });
  </script>
</body>
</html>

)=====";
