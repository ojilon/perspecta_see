param([switch]$Run, [int]$Port = 8787)
$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot
$sources = Get-ChildItem -Path src -Filter *.c | ForEach-Object { $_.FullName }
$out = "perspecta_see.exe"
if (Get-Command gcc -ErrorAction SilentlyContinue) {
    & gcc -O2 -Wall -Wextra -o $out @sources -lws2_32
} elseif (Get-Command zig -ErrorAction SilentlyContinue) {
    & zig cc -O2 -Wall -o $out @sources -lws2_32
} else {
    throw "Need gcc or zig on PATH"
}
if ($LASTEXITCODE -ne 0) { throw "build failed (is the old server still running? Ctrl+C it first)" }
Write-Host "built $out"
if ($Run) { & ".\$out" --port $Port }