param(
  [string]$OutputDir = "coverage"
)

$ErrorActionPreference = "Stop"

if (-not (Get-Command pio -ErrorAction SilentlyContinue)) {
  Write-Error "PlatformIO (pio) not found in PATH."
}

if (-not (Get-Command python -ErrorAction SilentlyContinue)) {
  Write-Error "Python not found in PATH."
}

if (-not (Test-Path $OutputDir)) {
  New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

pio test -e native
python -m gcovr -r . --html-details -o "$OutputDir\index.html" --xml -o "$OutputDir\coverage.xml"

Write-Host "Coverage reports generated: $OutputDir\index.html and $OutputDir\coverage.xml"
