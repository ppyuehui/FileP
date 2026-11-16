# build_filep.ps1 - build FileP with the repository's MSVC environment.
# Updates stay off: this is a private fork, the official package must never
# replace it (see the manifest URL and public key in CMakeLists.txt).
param([string]$Target = "pulse", [switch]$Fresh, [string]$Config = "Release",
      # Keep the build id a tree already has (CI pins PULSE_BUILD_ID for releases).
      [switch]$KeepBuildId,
      # Product version stamped into this build; defaults to today's date
      # (YY.MM.DD), which is how FileP numbers its own builds. Pass -Version 1.0.60
      # to use the upstream number, or -Version '' to let CMake pick the date.
      [string]$Version = (Get-Date).ToString('yy.MM.dd'),
      # Leave a running FileP alone (the linker then fails on the locked exes).
      [switch]$KeepRunning)
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
Push-Location $root
try {
    cmd /c "call scripts\vcvars.bat >nul 2>&1 && set" | ForEach-Object {
        if ($_ -match '^([^=]+)=(.*)$') {
            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2])
        }
    }
    # A running FileP (or one of its hosts) holds the exe images and the linker
    # fails with LNK1104 halfway through. Close them first: settings are saved as
    # they change.
    if (-not $KeepRunning) {
        $running = @(Get-Process FileP, Pulse.Index, Pulse.Content, Pulse.Preview, Pulse.AI, Pulse.Document, pulse_shell -ErrorAction SilentlyContinue)
        foreach ($proc in $running) {
            try { Stop-Process -Id $proc.Id -Force -ErrorAction Stop }
            catch { Write-Host "  keep $($proc.ProcessName) ($($proc.Id)): no access" -ForegroundColor DarkYellow }
        }
        if ($running.Count -gt 0) {
            Write-Host ("closed {0} running host(s) before linking" -f $running.Count) -ForegroundColor Cyan
            Start-Sleep -Milliseconds 400
        }
    }

    # Helpers started from a higher-integrity process cannot be closed from this
    # session; refuse before compiling instead of failing on LNK1104 at link time.
    $exes = @('FileP.exe', 'pulse_shell.exe', 'pulse_launcher.exe', 'pulse_elevated.exe',
        'pulse_hotkey_broker.exe', 'Pulse.Index.exe', 'Pulse.Content.exe', 'Pulse.Preview.exe',
        'Pulse.AI.exe', 'Pulse.Document.exe')
    $locked = @()
    for ($attempt = 0; $attempt -lt 3; $attempt++) {
        $locked = @()
        foreach ($exe in $exes) {
            $path = Join-Path $root ('build/' + $exe)
            if (-not (Test-Path $path)) { continue }
            try {
                $stream = [System.IO.File]::Open($path, 'Open', 'ReadWrite', 'None')
                $stream.Close()
            } catch {
                $locked += $exe
            }
        }
        if ($locked.Count -eq 0) { break }
        if ($attempt -lt 2) { Start-Sleep -Seconds 1 }
    }
    if ($locked.Count -gt 0) {
        Write-Host ("这些输出 exe 被占用，链接会失败：" + ($locked -join ', ')) -ForegroundColor Red
        Write-Host '多半是资源管理器/高权限进程里的 pulse_shell 还活着。请用管理员 PowerShell 执行：' -ForegroundColor Yellow
        Write-Host ('  Get-Process ' + (($locked | ForEach-Object { $_.Replace('.exe', '') }) -join ',') + ' | Stop-Process -Force') -ForegroundColor Yellow
        Write-Host '或重启资源管理器后再编译（-KeepRunning 可跳过自动结束）。' -ForegroundColor Yellow
        exit 2
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
    $versionArg = if ($Version) { "-DPULSE_VERSION_OVERRIDE=$Version" } else { '-DPULSE_VERSION_OVERRIDE=' }
    if ($Version) { Write-Host "version: $Version (upstream base: $(Get-Content (Join-Path $root 'version.txt') -TotalCount 1))" -ForegroundColor Cyan }
    $cmakeArgs = $freshArgs + @('-S','.','-B','build','-G','Ninja',
        "-DCMAKE_BUILD_TYPE=$Config",
        '-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded',
        '-DPULSE_WITH_SELFTEST=OFF',
        '-DPULSE_UPDATE_MANIFEST_URL=',
        '-DPULSE_UPDATE_PUBLIC_KEY_HEX=',
        $versionArg)
    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & cmake --build build --target $Target
    exit $LASTEXITCODE
}
finally { Pop-Location }
