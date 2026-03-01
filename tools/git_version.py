#!/usr/bin/env python3
"""Git-version helper.

Dual-purpose:
- PlatformIO extra_script (SCons): generates include/version_info.h and sets env["FW_VERSION"].
- CLI: prints a version string to stdout.

Version strategy:
- Prefer latest tag (vX.Y.Z or X.Y.Z)
- Else fallback to YY.MM.DD+g<shorthash>
"""

from __future__ import annotations

from datetime import datetime
from pathlib import Path
import argparse
import subprocess


def _run_git(cmd: list[str], cwd: Path) -> str:
    return subprocess.check_output(cmd, cwd=cwd, stderr=subprocess.STDOUT).decode().strip()


def current_git_version(project_dir: Path) -> str:
    try:
        tag = _run_git(["git", "describe", "--tags", "--abbrev=0"], cwd=project_dir)
        if tag.lower().startswith("v") and len(tag) > 1:
            tag = tag[1:]
        return tag
    except Exception:
        today = datetime.utcnow().strftime("%y.%m.%d")
        try:
            short_hash = _run_git(["git", "rev-parse", "--short", "HEAD"], cwd=project_dir)
        except Exception:
            short_hash = "unknown"
        return f"{today}+g{short_hash}"


def write_header(project_dir: Path, version: str) -> None:
    include_dir = project_dir / "include"
    header_path = include_dir / "version_info.h"
    include_dir.mkdir(parents=True, exist_ok=True)
    header_path.write_text(
        "#pragma once\n" f"#define FW_VERSION \"{version}\"\n",
        encoding="utf-8",
    )


def cli() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--project-dir", default=str(Path(__file__).resolve().parents[1]))
    ap.add_argument("--version", action="store_true", help="Print version and exit")
    args = ap.parse_args()

    project_dir = Path(args.project_dir).resolve()
    v = current_git_version(project_dir)

    if args.version:
        print(v)
        return 0

    # default behavior: just print
    print(v)
    return 0


# If invoked by PlatformIO/SCons, `env` is injected and Import exists.
try:
    Import  # type: ignore[name-defined]
except NameError:
    Import = None  # type: ignore[assignment]

if Import is not None:
    # PlatformIO context
    Import("env")  # type: ignore[misc]
    project_dir = Path(env["PROJECT_DIR"])  # type: ignore[name-defined]
    version = current_git_version(project_dir)
    write_header(project_dir, version)
    env["FW_VERSION"] = version  # type: ignore[index]
    print(f"Firmware version: {version}")
else:
    raise SystemExit(cli())
