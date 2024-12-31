#pragma once
#include <pgmspace.h>

// Single HTML page with sidebar navigation:
// - Main Drive (joystick status, battery voltage, init/start/stop button)
// - Joystick Test (shows axes & buttons)
// - Settings (checkbox placeholder)
//
// Joystick detection is done client-side. If found, main-drive indicator goes green, otherwise red.

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
    /* Joystick Test boxes */
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
      <p>
        Joystick Status:
        <span id="joystickStatusTxt">Not Connected</span>
        <span id="joystickIndicator" class="indicator"></span>
      </p>
      <p>
        Battery Voltage:
        <span id="batteryVoltage" class="batteryBox">--</span>
      </p>
      <p>
        <button id="robotButton" onclick="handleRobotButton()">Init</button>
      </p>
    </div>

    <!-- JOYSTICK TEST PAGE -->
    <div id="joystickTest" class="page-section">
      <h1>Joystick Test</h1>
      <p>Connect a controller, press a button to register, and move the sticks. Axes and buttons will be shown here.</p>
      <div id="joystickStatus">No gamepad detected.</div>
      <h2>Axes</h2>
      <div id="axisData">No data yet...</div>
      <h2>Buttons</h2>
      <div id="buttonData">No data yet...</div>
    </div>

    <!-- SETTINGS PAGE -->
    <div id="settings" class="page-section">
      <h1>Settings</h1>
      <label>
        <input type="checkbox" id="someSetting" />
        <span>Some Setting</span>
      </label>
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
     * MAIN DRIVE: ROBOT BUTTON (Init → Start → Stop → Init)
     *******************************************************/
    let robotState = "init"; 
    // We'll cycle: init -> start -> stop -> init ...
    async function handleRobotButton() {
      let cmd = "";
      switch(robotState) {
        case "init": cmd = "init"; break;
        case "start": cmd = "start"; break;
        case "stop":  cmd = "stop"; break;
      }

      try {
        const response = await fetch(`/robotControl?cmd=${cmd}`);
        if (!response.ok) {
          console.error("Failed to send robot command", cmd);
          return;
        }
      } catch(err) {
        console.error("Error:", err);
        return;
      }

      // After sending the command successfully, move to next state
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
     *******************************************************/
    async function updateBatteryVoltage() {
      try {
        const resp = await fetch('/getBattery');
        if (!resp.ok) return;
        const voltage = await resp.text();
        document.getElementById('batteryVoltage').textContent = voltage + " V";
      } catch (err) {
        console.error("Battery fetch error:", err);
      }
    }

    /*******************************************************
     * JOYSTICK DETECTION
     *******************************************************/
    let gamepads = {};

    function updateGamepads() {
      const gpList = navigator.getGamepads ? navigator.getGamepads() : [];
      for (let i = 0; i < gpList.length; i++) {
        const gp = gpList[i];
        if (gp) {
          gamepads[gp.index] = gp;
        }
      }
    }

    // Send full data to /updateController
    async function sendGamepadData(gp) {
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

    // This loop runs ~60 FPS
    function gamepadLoop() {
      updateGamepads();
      const gp = gamepads[0]; // focus on the first gamepad

      // MAIN DRIVE joystick indicator
      const indicator = document.getElementById('joystickIndicator');
      const indicatorTxt = document.getElementById('joystickStatusTxt');

      if (gp) {
        // Turn the indicator GREEN
        indicator.style.background = "green";
        indicatorTxt.textContent = "Connected";

        // Joystick Test page display
        displayJoystickData(gp);

        // Send data to NodeMCU
        sendGamepadData(gp);
      } else {
        // Turn the indicator RED
        indicator.style.background = "red";
        indicatorTxt.textContent = "Not Connected";
      }

      requestAnimationFrame(gamepadLoop);
    }

    // Detect connect/disconnect
    window.addEventListener('gamepadconnected', (e) => {
      console.log("Gamepad connected:", e.gamepad);
    });
    window.addEventListener('gamepaddisconnected', (e) => {
      console.log("Gamepad disconnected:", e.gamepad);
      delete gamepads[e.gamepad.index];
    });

    /*******************************************************
     * ON PAGE LOAD
     *******************************************************/
    window.addEventListener('load', () => {
      // Kick off gamepad loop
      requestAnimationFrame(gamepadLoop);

      // Fetch battery voltage every 2 seconds
      updateBatteryVoltage();
      setInterval(updateBatteryVoltage, 2000);
    });
  </script>

</body>
</html>

)=====";
