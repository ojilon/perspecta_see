# build.ps1 — Windows build (gcc or zig cc).
#
#   .\build.ps1                 build perspecta_see.exe
#   .\build.ps1 -Run            build, then serve on http://127.0.0.1:8787
#   .\build.ps1 -Run -Bind 0.0.0.0   ...or on every interface (sandbox demos)
#   .\build.ps1 -Run -Port 9000
#
# Headers live in include/, sources in src/ (one .c per header). Every .c under
# src/ is compiled, so adding a file needs no change here.
param(
    [switch]$Run,
    [int]$Port = 8787,
    [string]$Bind = "127.0.0.1"
)
$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

$sources = Get-ChildItem -Path src -Filter *.c | ForEach-Object { $_.FullName }
$out = "perspecta_see.exe"
$flags = @("-O2", "-std=c99", "-Wall", "-Wextra", "-Iinclude")

if (Get-Command gcc -ErrorAction SilentlyContinue) {
    & gcc @flags -o $out @sources -lws2_32
} elseif (Get-Command zig -ErrorAction SilentlyContinue) {
    & zig cc @flags -o $out @sources -lws2_32
} else {
    throw "Need gcc or zig on PATH"
}
if ($LASTEXITCODE -ne 0) { throw "build failed (is the old server still running? Ctrl+C it first)" }
Write-Host "built $out"

if ($Run) { & ".\$out" --port $Port --host $Bind }
