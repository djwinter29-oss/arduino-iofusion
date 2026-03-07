#!/usr/bin/env bash
set -euo pipefail

OUTPUT_DIR=${1:-coverage}
COVERAGE_THRESHOLD=${COVERAGE_THRESHOLD:-90}
COVERAGE_FILTER=${COVERAGE_FILTER:-lib/IOFusion/src|src/cmdline.cpp}

if ! command -v pio >/dev/null 2>&1; then
  echo "PlatformIO (pio) not found in PATH." >&2
  exit 1
fi

if ! command -v python >/dev/null 2>&1; then
  echo "Python not found in PATH." >&2
  exit 1
fi

mkdir -p "$OUTPUT_DIR"

pio test -e native
python -m gcovr -r . --filter "$COVERAGE_FILTER" --fail-under-line "$COVERAGE_THRESHOLD" --txt -o "$OUTPUT_DIR/summary.txt"
python -m gcovr -r . --filter "$COVERAGE_FILTER" --html-details -o "$OUTPUT_DIR/index.html" --xml -o "$OUTPUT_DIR/coverage.xml"

echo "Coverage threshold: ${COVERAGE_THRESHOLD}% (filter: ${COVERAGE_FILTER})"
echo "Coverage reports generated: $OUTPUT_DIR/summary.txt, $OUTPUT_DIR/index.html and $OUTPUT_DIR/coverage.xml"
