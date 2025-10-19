#!/usr/bin/env python3
"""Synchronise project version across metadata files.

Reads the semver string from VERSION and ensures both library.properties
and library.json expose the same value. The script is idempotent and only
touches the files when an update is required.
"""

from __future__ import annotations

import json
import sys
from pathlib import Path


class VersionSyncError(RuntimeError):
    pass


def read_version(version_path: Path) -> str:
    try:
        version = version_path.read_text(encoding="utf-8").strip()
    except OSError as exc:  # pragma: no cover - catastrophic failure
        raise VersionSyncError(f"Failed to read {version_path}") from exc

    if not version:
        raise VersionSyncError(f"VERSION file {version_path} is empty")
    return version


def update_library_properties(path: Path, version: str) -> bool:
    if not path.exists():
        raise VersionSyncError(f"Missing {path} required for Arduino metadata")

    text = path.read_text(encoding="utf-8").splitlines()
    updated = False
    for idx, line in enumerate(text):
        if line.startswith("version="):
            if line != f"version={version}":
                text[idx] = f"version={version}"
                updated = True
            break
    else:
        text.append(f"version={version}")
        updated = True

    if updated:
        path.write_text("\n".join(text) + "\n", encoding="utf-8")
    return updated


def default_library_json() -> dict:
    return {
        "name": "Probot Lib",
        "version": "0.0.0",
        "description": "ESP32-S3 robotics control library",
        "keywords": ["robotics", "pid", "motion-control"],
        "repository": {
            "type": "git",
            "url": "https://github.com/nfrproducts/probot-lib"
        },
        "frameworks": "arduino",
        "platforms": ["espressif32"],
        "build": {
            "flags": ["-DESP32S3", "-DARDUINO_USB_MODE=1"]
        },
        "dependencies": [
            {
                "name": "Adafruit NeoPixel",
                "version": ">=1.12.0"
            }
        ]
    }


def update_library_json(path: Path, version: str) -> bool:
    if path.exists():
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
        except json.JSONDecodeError as exc:
            raise VersionSyncError(f"Invalid JSON in {path}") from exc
    else:
        data = default_library_json()

    if data.get("version") == version:
        return False

    data["version"] = version
    path.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return True


def update_idf_component(path: Path, version: str) -> bool:
    if not path.exists():
        template = [
            f"version: {version}",
            "description: ESP32-S3 robotics control library",
            "url: https://github.com/nfrproducts/probot-lib",
            "dependencies:",
            "  idf:",
            "    version: \">=5.0\"",
            "  espressif/arduino-esp32:",
            "    version: \">=3.0.0\"",
        ]
        path.write_text("\n".join(template) + "\n", encoding="utf-8")
        return True

    lines = path.read_text(encoding="utf-8").splitlines()
    updated = False
    for idx, line in enumerate(lines):
        stripped = line.lstrip()
        if stripped.startswith("version:"):
            indent = line[: len(line) - len(stripped)]
            replacement = f"{indent}version: {version}"
            if line != replacement:
                lines[idx] = replacement
                updated = True
            break
    else:
        lines.insert(0, f"version: {version}")
        updated = True

    if updated:
        path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return updated


def main() -> int:
    root = Path(__file__).resolve().parent.parent
    version_path = root / "VERSION"
    lib_props_path = root / "library.properties"
    lib_json_path = root / "library.json"

    version = read_version(version_path)

    props_changed = update_library_properties(lib_props_path, version)
    json_changed = update_library_json(lib_json_path, version)

    idf_yaml_path = root / "idf_component.yml"
    idf_changed = update_idf_component(idf_yaml_path, version)

    if props_changed or json_changed or idf_changed:
        sys.stdout.write(f"Version metadata synced to {version}\n")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except VersionSyncError as exc:
        sys.stderr.write(f"error: {exc}\n")
        raise SystemExit(1)
