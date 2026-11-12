# Opens a browser page that shows each test-stage subject's Meshy / Rodin / fal versions
# side by side with one shared orbit camera. Serves the project root on localhost only.
#   .\Scripts\compare-models.ps1            (Ctrl+C to stop the server)
param([int]$Port = 8917)

$root = Split-Path -Parent (Get-ChildItem -Path (Join-Path $PSScriptRoot '..') -Filter *.uproject | Select-Object -First 1).FullName
Write-Host "Serving $root on http://127.0.0.1:$Port (Ctrl+C to stop)"
Start-Process "http://127.0.0.1:$Port/Scripts/compare-models.html"
python -I -m http.server $Port --bind 127.0.0.1 --directory $root

