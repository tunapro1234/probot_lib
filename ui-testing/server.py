#!/usr/bin/env python3
"""
Lightweight local simulator for the Probot driver station web UI.

It serves the HTML bundle embedded in `src/platform/esp32s3/web/index_html.h`
and provides stub endpoints so the frontend behaves as if it were talking to
the ESP32 firmware. This lets us iterate on the UI without flashing hardware.
"""
from __future__ import annotations

import argparse
import http.server
import json
import logging
import re
import shutil
import socketserver
import subprocess
import sys
import threading
import time
import urllib.parse
from pathlib import Path
from typing import Dict, List, Optional, Tuple

REPO_ROOT = Path(__file__).resolve().parents[1]
INDEX_HEADER = REPO_ROOT / "src/platform/esp32s3/web/index_html.h"
START_TOKEN = 'R"=====('
END_TOKEN = ')=====";'
ESP32S3_FLASH_BYTES = 4 * 1024 * 1024  # 4 MiB default flash size
VARIANTS_DIR = Path(__file__).resolve().parent / "variants"
def load_embedded_html() -> str:
  """Extract the HTML payload from the C++ header without modifying the source."""
  try:
    text = INDEX_HEADER.read_text(encoding="utf-8")
  except FileNotFoundError as exc:  # pragma: no cover - guard rail
    raise SystemExit(
        f"Could not find driver station header at {INDEX_HEADER}. "
        "Run from inside the repository root."
    ) from exc

  try:
    start = text.index(START_TOKEN) + len(START_TOKEN)
    end = text.index(END_TOKEN, start)
  except ValueError as exc:  # pragma: no cover - guard rail
    raise SystemExit(
        "Unable to locate embedded HTML block. "
        "Expected tokens were missing in index_html.h."
    ) from exc

  # Preserve exact formatting so we are looking at the same bytes flashed to the ESP.
  return text[start:end]


def load_html(html_override: Optional[Path]) -> str:
  if html_override:
    return html_override.read_text(encoding="utf-8")
  return load_embedded_html()


def compute_html_stats(html: str) -> Tuple[int, float]:
  """Return payload size (including terminator) and flash percentage."""
  byte_count = len(html.encode("utf-8")) + 1  # include null terminator stored in PROGMEM
  percent = (byte_count / ESP32S3_FLASH_BYTES) * 100.0
  return byte_count, percent


def run_build_report(
    example: Path,
    build_dir: Path,
    arduino_cli: str,
    fqbn: str,
) -> Optional[Dict[str, float]]:
  """Compile the chosen example and parse flash/RAM usage."""
  if shutil.which(arduino_cli) is None:
    logging.error("arduino-cli not found on PATH; skipping build report.")
    return None

  if not example.exists():
    logging.error("Example sketch not found at %s", example)
    return None

  build_dir.mkdir(parents=True, exist_ok=True)

  cmd = [
      arduino_cli,
      "compile",
      "--fqbn",
      fqbn,
      "--warnings",
      "none",
      "--library",
      str(REPO_ROOT),
      "--build-path",
      str(build_dir),
      str(example),
  ]

  logging.info("Running: %s", " ".join(cmd))
  proc = subprocess.run(cmd, capture_output=True, text=True)
  output = (proc.stdout or "") + (proc.stderr or "")

  if proc.returncode != 0:
    logging.error("arduino-cli compile failed with exit code %s", proc.returncode)
    logging.error(output.strip())
    return None

  flash_match = re.search(
      r"Sketch uses (\d+) bytes \(~?([0-9.]+)%\) of program storage space\. Maximum is (\d+) bytes\.",
      output,
  )
  ram_match = re.search(
      r"Global variables use (\d+) bytes \(~?([0-9.]+)%\) of dynamic memory, "
      r"leaving (\d+) bytes for local variables\. Maximum is (\d+) bytes\.",
      output,
  )

  if not flash_match:
    logging.warning("Could not parse flash usage from arduino-cli output.")
    logging.debug(output)
    return None

  flash_used = int(flash_match.group(1))
  flash_percent = float(flash_match.group(2))
  flash_max = int(flash_match.group(3))

  stats: Dict[str, float] = {
      "flash_used": flash_used,
      "flash_percent": flash_percent,
      "flash_max": flash_max,
  }

  if ram_match:
    stats.update(
        ram_used=int(ram_match.group(1)),
        ram_percent=float(ram_match.group(2)),
        ram_max=int(ram_match.group(4)),
    )

  stats["log"] = output.strip()
  return stats


class DriverStationHandler(http.server.BaseHTTPRequestHandler):
  """HTTP handler that mimics the ESP32 endpoints exposed by the driver station."""

  server_version = "ProbotUISim/0.1"

  # Shared simulator state. This is intentionally minimal for now.
  robot_state = {
      "status": "init",          # init | start | stop (matches UI expectations)
      "autonomous": True,
      "auto_period": 30,
      "battery": 12.4,
      "last_gamepad_update": 0.0,
  }
  state_lock = threading.Lock()

  def do_GET(self) -> None:  # noqa: N802 - keep BaseHTTPRequestHandler signature
    parsed = urllib.parse.urlparse(self.path)
    path = parsed.path

    if path in ("/", "/index.html"):
      self._serve_index()
      return

    if path == "/robotControl":
      self._handle_robot_control(parsed.query)
      return

    if path == "/getBattery":
      self._send_text("text/plain", f"{self.robot_state['battery']:.1f}")
      return

    if path == "/favicon.ico":
      self.send_error(404)
      return

    self.send_error(404, f"Unsupported path: {path}")

  def do_POST(self) -> None:  # noqa: N802 - keep BaseHTTPRequestHandler signature
    parsed = urllib.parse.urlparse(self.path)
    if parsed.path == "/updateController":
      self._handle_update_controller()
      return

    self.send_error(404, f"Unsupported path: {parsed.path}")

  # ===== Helpers ============================================================
  def _serve_index(self) -> None:
    try:
      html = load_html(getattr(self.server, "html_override", None))
    except OSError as exc:
      logging.error("Failed to load HTML content: %s", exc)
      self.send_error(500, "Unable to load UI content.")
      return
    payload = html.encode("utf-8")
    self.send_response(200)
    self.send_header("Content-Type", "text/html; charset=utf-8")
    self.send_header("Content-Length", str(len(payload)))
    self.end_headers()
    self.wfile.write(payload)

  def _handle_robot_control(self, query: str) -> None:
    params = urllib.parse.parse_qs(query)
    cmd = params.get("cmd", [""])[0]
    auto = params.get("auto", ["1"])[0] == "1"
    auto_len = params.get("autoLen", ["30"])[0]

    with self.state_lock:
      self.robot_state["autonomous"] = auto
      try:
        self.robot_state["auto_period"] = int(auto_len)
      except ValueError:
        pass
      if cmd in ("init", "start", "stop"):
        self.robot_state["status"] = cmd

    logging.info(
        "robotControl cmd=%s auto=%s autoLen=%s", cmd, auto, auto_len
    )

    self._send_text("text/plain", "OK")

  def _handle_update_controller(self) -> None:
    length_header = self.headers.get("Content-Length")
    try:
      length = int(length_header or "0")
    except ValueError:
      self.send_error(400, "Invalid Content-Length")
      return

    body = self.rfile.read(length)
    try:
      payload = json.loads(body.decode("utf-8"))
    except json.JSONDecodeError:
      logging.warning("Received malformed controller payload: %r", body)
      payload = {}

    with self.state_lock:
      self.robot_state["last_gamepad_update"] = time.time()

    logging.debug("Controller update: %s", payload)
    self._send_text("text/plain", "OK")

  def log_message(self, format: str, *args: Tuple[object, ...]) -> None:  # type: ignore[override]
    # Route all logs through logging module for consistency.
    logging.info("%s - %s", self.address_string(), format % args)

  def _send_text(self, content_type: str, payload: str) -> None:
    data = payload.encode("utf-8")
    self.send_response(200)
    self.send_header("Content-Type", content_type)
    self.send_header("Content-Length", str(len(data)))
    self.end_headers()
    self.wfile.write(data)


def parse_args(argv: list[str]) -> argparse.Namespace:
  parser = argparse.ArgumentParser(
      description="Serve the Probot driver station UI locally."
  )
  parser.add_argument(
      "--port", "-p", type=int, default=8080,
      help="Port to bind (default: 8080).",
  )
  parser.add_argument(
      "--open-browser", action="store_true",
      help="Attempt to open the UI in the default browser after starting.",
  )
  parser.add_argument(
      "--log-level", default="INFO",
      choices=("DEBUG", "INFO", "WARNING", "ERROR"),
      help="Logging verbosity (default: INFO).",
  )
  parser.add_argument(
      "--build-report", action="store_true",
      help="Run arduino-cli compile to capture flash/RAM usage before serving.",
  )
  parser.add_argument(
      "--report-only", action="store_true",
      help="Emit size report (with optional build report) then exit.",
  )
  parser.add_argument(
      "--suite", action="store_true",
      help="Serve every HTML variant on consecutive ports starting at --suite-base-port.",
  )
  parser.add_argument(
      "--suite-base-port", type=int, default=9030,
      help="Base port for --suite mode (default: 9030).",
  )
  parser.add_argument(
      "--suite-limit", type=int, default=10,
      help="Maximum number of variants to serve when --suite is enabled (default: 10).",
  )
  parser.add_argument(
      "--suite-dir",
      default=str(VARIANTS_DIR),
      help="Directory scanned for variant HTML files in --suite mode.",
  )
  parser.add_argument(
      "--example",
      default=str(REPO_ROOT / "examples/__library_impl/DriverStationDemo/DriverStationDemo.ino"),
      help="Sketch path to compile for build report.",
  )
  parser.add_argument(
      "--arduino-cli",
      default="arduino-cli",
      help="arduino-cli executable to use for build report.",
  )
  parser.add_argument(
      "--fqbn",
      default="esp32:esp32:esp32s3",
      help="FQBN passed to arduino-cli during build report.",
  )
  parser.add_argument(
      "--build-dir",
      default=str(REPO_ROOT / ".build/ui-testing-report"),
      help="Output directory for build artifacts when computing build report.",
  )
  parser.add_argument(
      "--html",
      help="Path to an HTML variant to serve/measure instead of the embedded header.",
  )
  return parser.parse_args(argv)


class ThreadedHTTPServer(socketserver.ThreadingMixIn, http.server.HTTPServer):
  daemon_threads = True
  allow_reuse_address = True

  def __init__(self, server_address, RequestHandlerClass, html_override=None):
    self.html_override = html_override
    super().__init__(server_address, RequestHandlerClass)


def main(argv: list[str]) -> int:
  args = parse_args(argv)
  logging.basicConfig(
      level=getattr(logging, args.log_level),
      format="[%(levelname)s] %(message)s",
  )

  if not INDEX_HEADER.exists():
    logging.error("Driver station header not found at %s", INDEX_HEADER)
    return 1

  if args.html and args.suite:
    logging.error("--html and --suite cannot be used together.")
    return 1

  html_variants: List[Optional[Path]] = []
  if args.suite:
    suite_dir = Path(args.suite_dir).resolve()
    if not suite_dir.exists():
      logging.error("Variant directory not found: %s", suite_dir)
      return 1
    html_variants = sorted(suite_dir.glob("*.html"))[: max(args.suite_limit, 0)]
    if not html_variants:
      logging.error("No HTML variants found in %s", suite_dir)
      return 1
  else:
    override = None
    if args.html:
      override = Path(args.html)
      if not override.is_absolute():
        override = (REPO_ROOT / args.html).resolve()
      if not override.exists():
        logging.error("HTML override file not found at %s", override)
        return 1
    html_variants.append(override)

  build_stats = None
  if args.build_report:
    build_stats = run_build_report(
        example=Path(args.example),
        build_dir=Path(args.build_dir),
        arduino_cli=args.arduino_cli,
        fqbn=args.fqbn,
    )

  flash_mib = ESP32S3_FLASH_BYTES / (1024 * 1024)
  for html_path in html_variants:
    payload = load_html(html_path)
    bytes_used, pct = compute_html_stats(payload)
    name = html_path.name if html_path else "embedded"
    logging.info(
        "Embedded UI payload (%s): %d bytes (%.4f%% of %.2f MiB flash)",
        name,
        bytes_used,
        pct,
        flash_mib,
    )
    if build_stats:
      logging.info(
          "Sketch flash usage: %d bytes (%.2f%% of %d bytes)",
          int(build_stats["flash_used"]),
          build_stats["flash_percent"],
          int(build_stats["flash_max"]),
      )
      if build_stats["flash_used"]:
        logging.info(
            "UI payload share: %.3f%% of compiled image (%d / %d bytes)",
            (bytes_used / build_stats["flash_used"]) * 100.0,
            bytes_used,
            int(build_stats["flash_used"]),
        )
      if build_stats.get("ram_used") is not None:
        logging.info(
            "Sketch RAM usage: %d bytes (%.2f%% of %d bytes)",
            int(build_stats["ram_used"]),
            build_stats["ram_percent"],
            int(build_stats["ram_max"]),
        )

  if args.report_only:
    return 0

  handler = DriverStationHandler

  if args.suite:
    servers: List[ThreadedHTTPServer] = []
    threads: List[threading.Thread] = []
    try:
      for idx, html_path in enumerate(html_variants):
        port = args.suite_base_port + idx
        httpd = ThreadedHTTPServer(("", port), handler, html_override=html_path)
        thread = threading.Thread(target=httpd.serve_forever, daemon=True)
        thread.start()
        servers.append(httpd)
        threads.append(thread)
        logging.info("Serving %s at http://localhost:%d/", html_path.name, port)
      logging.info("Press Ctrl+C to stop all servers.")
      while True:
        time.sleep(1)
    except KeyboardInterrupt:
      logging.info("Stopping suite servers...")
    finally:
      for httpd in servers:
        httpd.shutdown()
      for thread in threads:
        thread.join(timeout=1)
    return 0

  html_override = html_variants[0]
  with ThreadedHTTPServer(("", args.port), handler, html_override=html_override) as httpd:
    url = f"http://localhost:{args.port}/"
    logging.info("Serving driver station UI at %s", url)

    if args.open_browser:
      try:
        import webbrowser

        webbrowser.open(url)
      except Exception as exc:  # pragma: no cover - guard rail
        logging.warning("Failed to open browser: %s", exc)

    try:
      httpd.serve_forever()
    except KeyboardInterrupt:
      logging.info("Shutting down simulator.")
      httpd.shutdown()

  return 0


if __name__ == "__main__":  # pragma: no cover
  sys.exit(main(sys.argv[1:]))
