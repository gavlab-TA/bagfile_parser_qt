<#
.SYNOPSIS
    Build a redistributable Windows package of bagfile_parser_qt.

.DESCRIPTION
    Produces, in dist\:
      bagfile_parser_qt-<version>-windows-x64.exe    NSIS installer
      bagfile_parser_qt-<version>-windows-x64.zip    portable, unzip-and-run

    Both bundle the Qt DLLs and the platform plugin, so the machine you hand
    them to needs nothing preinstalled.

    Dependencies (matio, lz4, zstd) come from vcpkg and are linked statically by
    default, which keeps the payload to the .exe plus a handful of Qt DLLs.

.PARAMETER QtDir
    Qt kit directory, e.g. C:\Qt\6.11.1\mingw_64. Auto-detected under C:\Qt
    when omitted.

.PARAMETER VcpkgRoot
    vcpkg checkout. Defaults to $env:VCPKG_ROOT, then C:\vcpkg; cloned and
    bootstrapped there if missing.

.PARAMETER Triplet
    vcpkg triplet. Defaults to x64-mingw-static or x64-windows-static-md to
    match the detected Qt kit.

.EXAMPLE
    .\scripts\package.ps1
.EXAMPLE
    .\scripts\package.ps1 -QtDir C:\Qt\6.11.1\msvc2022_64
#>
[CmdletBinding()]
param(
    [string]$QtDir,
    [string]$VcpkgRoot,
    [string]$Triplet,
    [string]$Output,
    [int]$Jobs = 0,
    [switch]$SkipDeps
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$RepoRoot  = Split-Path -Parent $PSScriptRoot
$BuildDir  = Join-Path $RepoRoot 'build-package'
if (-not $Output) { $Output = Join-Path $RepoRoot 'dist' }

function Write-Step($msg) { Write-Host "==> $msg" -ForegroundColor Blue }
function Write-Info($msg) { Write-Host "    $msg" }
function Write-Warn($msg) { Write-Host "warning: $msg" -ForegroundColor Yellow }
function Die($msg) { Write-Host "error: $msg" -ForegroundColor Red; exit 1 }

function Require-Command($name, $hint) {
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if (-not $cmd) { Die "$name was not found on PATH.`n    $hint" }
    return $cmd.Source
}

# ------------------------------------------------------------------- Qt -----
if (-not $QtDir) {
    Write-Step 'Looking for a Qt installation'
    # Newest version first; prefer a MinGW kit since that is what the README
    # walks people through installing.
    $candidates = @()
    foreach ($root in @('C:\Qt', "$env:USERPROFILE\Qt")) {
        if (Test-Path $root) {
            $candidates += Get-ChildItem $root -Directory -ErrorAction SilentlyContinue |
                Where-Object { $_.Name -match '^\d+\.\d+' } |
                Sort-Object { [version]($_.Name) } -Descending |
                ForEach-Object { Get-ChildItem $_.FullName -Directory -ErrorAction SilentlyContinue } |
                Where-Object { $_.Name -match '^(mingw|msvc)' -and (Test-Path (Join-Path $_.FullName 'bin\qmake.exe')) }
        }
    }
    if ($candidates.Count -eq 0) {
        Die @"
No Qt kit found under C:\Qt.
    Install Qt with the official online installer (https://www.qt.io/download-qt-installer),
    choosing the "MinGW 64-bit" desktop component, then re-run this script -- or
    pass the path explicitly:  .\scripts\package.ps1 -QtDir C:\Qt\6.11.1\mingw_64
"@
    }
    $QtDir = $candidates[0].FullName
}
if (-not (Test-Path (Join-Path $QtDir 'bin\qmake.exe'))) {
    Die "'$QtDir' does not look like a Qt kit (no bin\qmake.exe)"
}
Write-Info "Qt: $QtDir"

$IsMinGWKit = (Split-Path $QtDir -Leaf) -match '^mingw'
if (-not $Triplet) {
    # *-static / *-static-md keep matio+hdf5+lz4+zstd inside the .exe.
    # -md on MSVC means "static libs, dynamic CRT", which is what prebuilt Qt expects.
    $Triplet = if ($IsMinGWKit) { 'x64-mingw-static' } else { 'x64-windows-static-md' }
}
Write-Info "vcpkg triplet: $Triplet"

# Put Qt's toolchain first on PATH: its bin\ has qmake/windeployqt, and for a
# MinGW kit the matching GCC and Ninja live under the sibling Tools\ directory.
$env:PATH = "$QtDir\bin;$env:PATH"
if ($IsMinGWKit) {
    $qtRoot = Split-Path (Split-Path $QtDir -Parent) -Parent
    $toolsDir = Join-Path $qtRoot 'Tools'
    if (Test-Path $toolsDir) {
        $mingw = Get-ChildItem $toolsDir -Directory | Where-Object { $_.Name -match '^mingw' } |
                 Sort-Object Name -Descending | Select-Object -First 1
        if ($mingw) { $env:PATH = "$($mingw.FullName)\bin;$env:PATH"; Write-Info "MinGW: $($mingw.FullName)" }
        $ninja = Join-Path $toolsDir 'Ninja'
        if (Test-Path $ninja) { $env:PATH = "$ninja;$env:PATH" }
    }
}

# For an MSVC kit, cl.exe must be the compiler CMake picks. It is not on PATH by
# default, and Windows machines (including CI runners) often carry a MinGW gcc
# that Ninja would silently choose instead -- producing a binary that cannot link
# against vcpkg's MSVC-built static libraries (undefined __security_cookie,
# __GSHandlerCheck, __chkstk ...). Import the Visual Studio developer
# environment so the right toolchain wins.
function Enter-VsDevEnvironment {
    if (Get-Command cl.exe -ErrorAction SilentlyContinue) {
        Write-Info 'MSVC environment already active'
        return
    }
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) {
        Die @'
Visual Studio was not found, and the selected Qt kit is an MSVC build.
    Install the "Desktop development with C++" workload (Visual Studio or the
    standalone Build Tools: winget install Microsoft.VisualStudio.2022.BuildTools),
    or use a MinGW Qt kit instead:  .\scripts\package.ps1 -QtDir C:\Qt\<ver>\mingw_64
'@
    }
    $vsPath = & $vswhere -latest -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath
    if (-not $vsPath) { Die 'no Visual Studio installation with the C++ toolchain was found' }

    $devCmd = Join-Path $vsPath 'Common7\Tools\VsDevCmd.bat'
    if (-not (Test-Path $devCmd)) { Die "VsDevCmd.bat not found under $vsPath" }

    Write-Step 'Activating the MSVC build environment'
    # Run VsDevCmd in cmd, then copy the environment it sets back into this session.
    & cmd /c "`"$devCmd`" -arch=amd64 -host_arch=amd64 >nul && set" | ForEach-Object {
        if ($_ -match '^([^=]+)=(.*)$') {
            Set-Item -Path "env:$($matches[1])" -Value $matches[2] -ErrorAction SilentlyContinue
        }
    }
    if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
        Die 'activated VsDevCmd but cl.exe is still not on PATH'
    }
    Write-Info "MSVC: $((Get-Command cl.exe).Source)"
}

if (-not $IsMinGWKit) { Enter-VsDevEnvironment }

# ---------------------------------------------------------------- tools -----
Require-Command cmake @'
Install CMake from https://cmake.org/download/ and tick
    "Add CMake to the system PATH" during setup.
'@ | Out-Null

$makensis = Get-Command makensis -ErrorAction SilentlyContinue
if (-not $makensis) {
    # NSIS installs here but does not add itself to PATH.
    foreach ($p in @("${env:ProgramFiles(x86)}\NSIS", "$env:ProgramFiles\NSIS")) {
        if (Test-Path (Join-Path $p 'makensis.exe')) { $env:PATH = "$p;$env:PATH"; break }
    }
    $makensis = Get-Command makensis -ErrorAction SilentlyContinue
}
if (-not $makensis) {
    Die @'
makensis (NSIS) was not found; it builds the installer.
    Install it with:  winget install NSIS.NSIS
    (or from https://nsis.sourceforge.io/Download), then re-run this script.
'@
}
Write-Info "NSIS: $($makensis.Source)"

# ---------------------------------------------------------------- vcpkg -----
if (-not $SkipDeps) {
    if (-not $VcpkgRoot) {
        $VcpkgRoot = if ($env:VCPKG_ROOT) { $env:VCPKG_ROOT } else { 'C:\vcpkg' }
    }
    if (-not (Test-Path (Join-Path $VcpkgRoot 'vcpkg.exe'))) {
        if (-not (Test-Path $VcpkgRoot)) {
            Write-Step "Cloning vcpkg into $VcpkgRoot"
            Require-Command git 'Install Git from https://git-scm.com/download/win' | Out-Null
            git clone --depth 1 https://github.com/microsoft/vcpkg $VcpkgRoot
            if ($LASTEXITCODE -ne 0) { Die 'git clone of vcpkg failed' }
        }
        Write-Step 'Bootstrapping vcpkg'
        & (Join-Path $VcpkgRoot 'bootstrap-vcpkg.bat') -disableMetrics
        if ($LASTEXITCODE -ne 0) { Die 'bootstrap-vcpkg.bat failed' }
    }
    Write-Info "vcpkg: $VcpkgRoot"
    Write-Info 'Dependencies build from source on first run (matio pulls in HDF5) - expect several minutes.'
}

# ------------------------------------------------------------ configure -----
$cmakeArgs = @(
    '-S', $RepoRoot
    '-B', $BuildDir
    '-DCMAKE_BUILD_TYPE=Release'
    "-DCMAKE_PREFIX_PATH=$QtDir"
)
# CI stamps the release tag onto the package name; locally the version in
# CMakeLists.txt is used.
if ($env:BPQ_VERSION) { $cmakeArgs += "-DBPQ_VERSION=$env:BPQ_VERSION" }
if (Get-Command ninja -ErrorAction SilentlyContinue) { $cmakeArgs += @('-G', 'Ninja') }
if ($IsMinGWKit) {
    $cmakeArgs += @('-DCMAKE_CXX_COMPILER=g++', '-DCMAKE_C_COMPILER=gcc')
} else {
    $cmakeArgs += @('-DCMAKE_CXX_COMPILER=cl', '-DCMAKE_C_COMPILER=cl')
}
if (-not $SkipDeps) {
    # Manifest mode: vcpkg.json at the repo root drives what gets built.
    $cmakeArgs += @(
        "-DCMAKE_TOOLCHAIN_FILE=$VcpkgRoot\scripts\buildsystems\vcpkg.cmake"
        "-DVCPKG_TARGET_TRIPLET=$Triplet"
        "-DVCPKG_HOST_TRIPLET=$Triplet"
    )
}

Write-Step 'Configuring'
& cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) { Die 'CMake configure failed (see the output above)' }

Write-Step 'Building'
$buildArgs = @('--build', $BuildDir)
if ($Jobs -gt 0) { $buildArgs += @('--parallel', $Jobs) } else { $buildArgs += '--parallel' }
& cmake @buildArgs
if ($LASTEXITCODE -ne 0) { Die 'Build failed (see the output above)' }

# -------------------------------------------------------------- package -----
Write-Step 'Packaging'
Push-Location $BuildDir
try {
    & cpack
    if ($LASTEXITCODE -ne 0) { Die 'cpack failed (see the output above)' }
} finally { Pop-Location }

Write-Step "Collecting packages into $Output"
New-Item -ItemType Directory -Force -Path $Output | Out-Null
$artifacts = Get-ChildItem $BuildDir -File | Where-Object { $_.Extension -in '.exe', '.zip' }
if (-not $artifacts) { Die 'cpack produced no packages; see the output above' }
foreach ($a in $artifacts) {
    Move-Item -Force $a.FullName (Join-Path $Output $a.Name)
    Write-Info ('{0}  ({1:N1} MB)' -f $a.Name, ($a.Length / 1MB))
}

Write-Host ''
Write-Host 'Hand the .exe to anyone on 64-bit Windows: it installs to Program Files,'
Write-Host 'adds a Start Menu entry, and can put bagfile_parser_qt on their PATH.'
Write-Host 'The .zip is the same build with nothing to install.'
Write-Host ''
Write-Host 'Note: the binaries are unsigned, so SmartScreen shows a "Windows protected'
Write-Host 'your PC" prompt on first run -- More info -> Run anyway.'
Write-Host 'Done.' -ForegroundColor Green
