#!/usr/bin/env python3
"""Set library.json version from a provided semantic version string."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

SEMVER_RE = re.compile(r"^\d+\.\d+\.\d+(?:[-+][0-9A-Za-z.-]+)?$")


def main() -> int:
    if len(sys.argv) != 2:
        print("Usage: set_library_version.py <version>", file=sys.stderr)
        return 2

    version = sys.argv[1].strip()
    if not SEMVER_RE.match(version):
        print(f"Invalid semantic version: {version}", file=sys.stderr)
        return 2

    path = Path("library.json")
    data = json.loads(path.read_text(encoding="utf-8"))
    data["version"] = version
    path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
    print(f"library.json version set to {version}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
