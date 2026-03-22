from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

import pytest


REPO_ROOT = Path(__file__).resolve().parents[1]


def run_python(*args: str, cwd: Path | None = None) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [sys.executable, *args],
        cwd=cwd or REPO_ROOT,
        capture_output=True,
        text=True,
        check=False,
    )


@pytest.mark.smoke
def test_git_version_script_prints_a_version() -> None:
    result = run_python("tools/git_version.py", "--version")

    assert result.returncode == 0
    assert result.stdout.strip()


@pytest.mark.smoke
def test_set_library_version_updates_json_in_temp_directory(tmp_path: Path) -> None:
    library_json = tmp_path / "library.json"
    library_json.write_text(
        json.dumps({"name": "IOFusion", "version": "0.0.0"}, indent=2) + "\n",
        encoding="utf-8",
    )

    result = run_python(str(REPO_ROOT / "tools/set_library_version.py"), "1.2.3", cwd=tmp_path)

    assert result.returncode == 0
    assert "library.json version set to 1.2.3" in result.stdout
    updated = json.loads(library_json.read_text(encoding="utf-8"))
    assert updated["version"] == "1.2.3"


@pytest.mark.smoke
def test_poll_all_help_succeeds() -> None:
    result = run_python("tools/poll_all.py", "--help")

    assert result.returncode == 0
    assert "all? once per interval" in result.stdout