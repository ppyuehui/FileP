# build_filep.ps1 - build FileP with the repository's MSVC environment.
# Updates stay off: this is a private fork, the official package must never
# replace it (see the manifest URL and public key in CMakeLists.txt).
param([string]$Target = "pulse", [switch]$Fresh, [string]$Config = "Release",
      # Keep the build id a tree already has (CI pins PULSE_BUILD_ID for releases).
      [switch]$KeepBuildId)
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
Push-Location $root
try {
    cmd /c "call scripts\vcvars.bat >nul 2>&1 && set" | ForEach-Object {
        if ($_ -match '^([^=]+)=(.*)$') {
            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
        }
    }
    # The fork shows the build date as its version (About > 构建时间, sidebar
    # footer). CMake keeps the first id it generated in build/pulse_build_id.txt
    # forever, so a tree built on an old day kept reporting that day. Give the
    # tree a fresh id on the first build of a new day; same-day rebuilds keep it.
    if (-not $KeepBuildId) {
        $buildDir = Join-Path $root 'build'
        $idFile = Join-Path $buildDir 'pulse_build_id.txt'
        $now = (Get-Date).ToUniversalTime()
        $today = $now.ToString('yyyyMMdd')
        $current = if (Test-Path $idFile) { (Get-Content $idFile -Raw).Trim() } else { '' }
        if (-not $current.StartsWith($today)) {
            New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
            $bytes = New-Object byte[] 4
            [System.Security.Cryptography.RandomNumberGenerator]::Fill($bytes)
            $random = ($bytes | ForEach-Object { $_.ToString('X2') }) -join ''
            Set-Content -Path $idFile -Value ($now.ToString('yyyyMMddTHHmmssZ') + '-' + $random) -NoNewline
            Write-Host "build id: $(Get-Content $idFile)" -ForegroundColor Cyan
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
