#pragma once
#include <pgmspace.h>

// Basit joystick sayfası: Gamepad API ile eksen verisini okur ve
// bir fetch isteğiyle NodeMCU'ya gönderir.
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Joystick Deneme</title>
</head>
<body>
  <h1>Joystick Testi</h1>
  <p>Bilgisayarınıza bir gamepad/joystick bağlayın ve herhangi bir butona basarak tarayıcıya tanıtın. Ardından eksenler değiştikçe NodeMCU’ya veri gönderilecektir.</p>
  <div id="status">Henüz Joystick bağlanmadı.</div>
  <script>
    let gamepads = {};

    function updateGamepads() {
      // Bağlı gamepadleri al
      const connected = navigator.getGamepads ? navigator.getGamepads() : [];
      for (let i = 0; i < connected.length; i++) {
        const g = connected[i];
        if (g) {
          gamepads[g.index] = g;
        }
      }
    }

    function sendJoystickData(xVal, yVal) {
      // xVal ve yVal’i /joystick endpoint’ine GET isteğiyle gönderiyoruz
      fetch(`/joystick?x=${xVal}&y=${yVal}`)
        .then(r => console.log(`Joystick verisi gonderildi: x=${xVal}, y=${yVal}`))
        .catch(e => console.error(e));
    }

    function gamepadLoop() {
      updateGamepads();

      // Sadece ilk (index=0) gamepad’e odaklanıyoruz (birden fazla olabilir).
      const gp = gamepads[0];
      if (gp) {
        document.getElementById('status').innerText = `Gamepad: ${gp.id}`;
        
        // Örnek: gp.axes[0] (X ekseni), gp.axes[1] (Y ekseni)
        let xAxis = gp.axes[0] || 0.0;
        let yAxis = gp.axes[1] || 0.0;

        // Aşırı sıklıkta çağırmamak için isterseniz bir threshold kontrolü ekleyebilirsiniz.
        // Burada basitlik için her frame gönderiyoruz:
        sendJoystickData(xAxis.toFixed(2), yAxis.toFixed(2));
      }

      requestAnimationFrame(gamepadLoop);
    }

    window.addEventListener('load', () => {
      // gamepad bağlandığında / ayrıldığında tetiklenen event’ler
      window.addEventListener("gamepadconnected", e => {
        console.log("Gamepad baglandi:", e.gamepad);
        document.getElementById('status').innerText = `Gamepad baglandi: ${e.gamepad.id}`;
      });
      window.addEventListener("gamepaddisconnected", e => {
        console.log("Gamepad ayrildi:", e.gamepad);
        document.getElementById('status').innerText = "Gamepad baglantisi kesildi.";
        delete gamepads[e.gamepad.index];
      });

      // Sürekli joystick verilerini kontrol et
      requestAnimationFrame(gamepadLoop);
    });
  </script>
</body>
</html>
)=====";
