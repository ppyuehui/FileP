# build_filep.ps1 - build FileP with the repository's MSVC environment.
# Updates stay off: this is a private fork, the official package must never
# replace it (see the manifest URL and public key in CMakeLists.txt).
param([string]$Target = "pulse", [switch]$Fresh, [string]$Config = "Release")
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
Push-Location $root
try {
    cmd /c "call scripts\vcvars.bat >nul 2>&1 && set" | ForEach-Object {
        if ($_ -match '^([^=]+)=(.*)$') {
            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
        }
    }
    $freshArgs = @()
    if ($Fresh) { $freshArgs += '--fresh' }
    $cmakeArgs = $freshArgs + @('-S','.','-B','build','-G','Ninja',
        "-DCMAKE_BUILD_TYPE=$Config",
        '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded',
        '-DPULSE_WITH_SELFTEST=OFF',
        '-DPULSE_UPDATE_MANIFEST_URL=',
        '-DPULSE_UPDATE_PUBLIC_KEY_HEX=')
    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & cmake --build build --target $Target
    exit $LASTEXITCODE
}
finally { Pop-Location }
