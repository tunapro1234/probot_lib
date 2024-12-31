#pragma once
#include <pgmspace.h>

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8" />
    <title>Slider Test</title>
  </head>
  <body>
    <h1>Slider Test</h1>
    <input type="range" min="0" max="100" value="50" oninput="changeSlider(this.value)" />
    <script>
      function changeSlider(value) {
        fetch('/slider?val=' + value)
          .then(r => console.log("Slider changed: " + value))
          .catch(e => console.error(e));
      }
    </script>
  </body>
</html>
)=====";
