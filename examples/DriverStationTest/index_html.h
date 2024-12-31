#pragma once
#include <pgmspace.h>

// Minimal HTML for reading all axes/buttons from the first connected gamepad.
// Sends data as a JSON POST to "/updateController".
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8">
    <title>Gamepad Full Read</title>
  </head>
  <body>
    <h1>Gamepad Full Read</h1>
    <p>Connect a controller, press any button to register it, and watch the NodeMCU serial output!</p>
    <div id="status">No gamepad detected.</div>
    <script>
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

      async function sendControllerData(gp) {
        // Build an object with axes and button states
        const data = {
          axes: gp.axes,
          buttons: gp.buttons.map(btn => btn.pressed) // true/false for each
        };

        // Send via POST as JSON
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

      function gamepadLoop() {
        updateGamepads();

        // We'll only look at the first controller (index=0) for simplicity
        const gp = gamepads[0];
        if (gp) {
          document.getElementById('status').innerText = `Gamepad: ${gp.id} — Axes: ${gp.axes.length}, Buttons: ${gp.buttons.length}`;
          sendControllerData(gp);
        }

        requestAnimationFrame(gamepadLoop);
      }

      // Listen for gamepad connect/disconnect
      window.addEventListener('gamepadconnected', e => {
        console.log('Gamepad connected:', e.gamepad);
        document.getElementById('status').innerText = 'Gamepad connected!';
      });
      window.addEventListener('gamepaddisconnected', e => {
        console.log('Gamepad disconnected:', e.gamepad);
        document.getElementById('status').innerText = 'Gamepad disconnected.';
        delete gamepads[e.gamepad.index];
      });

      // On page load, start the animation loop
      window.addEventListener('load', () => {
        requestAnimationFrame(gamepadLoop);
      });
    </script>
  </body>
</html>
)=====";
