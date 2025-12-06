from datetime import datetime
from pathlib import Path
import subprocess
# SCons build environment from PlatformIO, provided automatically
Import("env")

PROJECT_DIR = Path(env["PROJECT_DIR"])
INCLUDE_DIR = PROJECT_DIR / "include"
HEADER_PATH = INCLUDE_DIR / "version_info.h"


def _run_git(cmd):
    return subprocess.check_output(cmd, cwd=PROJECT_DIR, stderr=subprocess.STDOUT).decode().strip()


def current_git_version() -> str:
    try:
        tag = _run_git(["git", "describe", "--tags", "--abbrev=0"])
        if tag.lower().startswith("v") and len(tag) > 1:
            tag = tag[1:]
        return tag
    except Exception:
        # No tag available – fallback to date + short hash (SemVer compliant)
        today = datetime.utcnow().strftime("%y.%m.%d")
        try:
            short_hash = _run_git(["git", "rev-parse", "--short", "HEAD"])
        except Exception:
            short_hash = "unknown"
        return f"{today}+g{short_hash}"


def write_header(version: str) -> None:
    INCLUDE_DIR.mkdir(parents=True, exist_ok=True)
    HEADER_PATH.write_text(
        "#pragma once\n"
        f"#define FW_VERSION \"{version}\"\n",
        encoding="utf-8",
    )


version = current_git_version()
write_header(version)

env["FW_VERSION"] = version
print(f"Firmware version: {version}")
