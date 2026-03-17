# build_installer.ps1
# Запускати з директорії cpp/installer або cpp
# Створює:
#   1. PunicWarsCastra_Setup_v1.0.exe  — повноцінний інсталятор (Inno Setup)
#   2. PunicWarsCastra_Portable_v1.0.zip — portable ZIP архів

param(
    [string]$Version = "1.0",
    [switch]$PortableOnly
)

$ErrorActionPreference = "Stop"

# Визначаємо шляхи
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$GameDir   = Split-Path -Parent $ScriptDir   # cpp/
$OutDir    = Join-Path $ScriptDir "output"

Write-Host "=== Punic Wars: Castra - Build Installer ===" -ForegroundColor Cyan
Write-Host "Game dir : $GameDir"
Write-Host "Output   : $OutDir"

# Перевіряємо що exe існує
$ExePath = Join-Path $GameDir "punic_wars.exe"
if (-not (Test-Path $ExePath)) {
    Write-Host "ERROR: punic_wars.exe not found at $ExePath" -ForegroundColor Red
    Write-Host "Run compile.bat first!" -ForegroundColor Yellow
    exit 1
}

# Створюємо output директорію
if (-not (Test-Path $OutDir)) {
    New-Item -ItemType Directory -Path $OutDir | Out-Null
}

# ─────────────────────────────────────────────
# КРОК 1: Portable ZIP
# ─────────────────────────────────────────────
$ZipName = "PunicWarsCastra_Portable_v$Version.zip"
$ZipPath = Join-Path $OutDir $ZipName

Write-Host ""
Write-Host "[1/2] Creating portable ZIP..." -ForegroundColor Yellow

# Збираємо файли для ZIP
$TempDir = Join-Path $OutDir "temp_portable"
if (Test-Path $TempDir) { Remove-Item $TempDir -Recurse -Force }
New-Item -ItemType Directory -Path $TempDir | Out-Null

# Копіюємо exe
Copy-Item $ExePath $TempDir

# Копіюємо assets
$AssetsSource = Join-Path $GameDir "assets"
$AssetsDest   = Join-Path $TempDir "assets"
Copy-Item $AssetsSource $AssetsDest -Recurse

# Створюємо README для portable версії
$ReadmeContent = @"
Punic Wars: Castra v$Version — Portable Edition
================================================

ЗАПУСК:
  Двічі клацніть punic_wars.exe

КЕРУВАННЯ:
  ЛКМ          — вибір юніта / будівлі
  ПКМ          — рух / атака / будівництво
  WASD/стрілки — рух камери
  Миша до краю — скрол камери
  Середня кнопка миші — перетягування камери

ВИМОГИ:
  Windows 10/11 (64-bit)
  Відеокарта з підтримкою OpenGL 3.3+

ФРАКЦІЇ:
  Рим     — 200 їжі, 100 золота на старті
  Карфаген — 150 їжі, 200 золота на старті
"@
$ReadmeContent | Out-File (Join-Path $TempDir "README.txt") -Encoding UTF8

# Архівуємо
if (Test-Path $ZipPath) { Remove-Item $ZipPath }
Compress-Archive -Path "$TempDir\*" -DestinationPath $ZipPath -CompressionLevel Optimal

# Прибираємо temp
Remove-Item $TempDir -Recurse -Force

$ZipSize = [math]::Round((Get-Item $ZipPath).Length / 1MB, 1)
Write-Host "  OK: $ZipName ($ZipSize MB)" -ForegroundColor Green

if ($PortableOnly) {
    Write-Host ""
    Write-Host "Done! Portable ZIP: $ZipPath" -ForegroundColor Cyan
    exit 0
}

# ─────────────────────────────────────────────
# КРОК 2: Inno Setup інсталятор
# ─────────────────────────────────────────────
Write-Host ""
Write-Host "[2/2] Building installer with Inno Setup..." -ForegroundColor Yellow

# Шукаємо ISCC.exe
$IsccPaths = @(
    "C:\Program Files (x86)\Inno Setup 6\ISCC.exe",
    "C:\Program Files\Inno Setup 6\ISCC.exe",
    "C:\Program Files (x86)\Inno Setup 5\ISCC.exe",
    "C:\Program Files\Inno Setup 5\ISCC.exe"
)

$IsccExe = $null
foreach ($p in $IsccPaths) {
    if (Test-Path $p) { $IsccExe = $p; break }
}

# Якщо не знайдено — завантажуємо
if (-not $IsccExe) {
    Write-Host "  Inno Setup not found. Downloading..." -ForegroundColor Yellow

    $InnoInstaller = Join-Path $env:TEMP "innosetup_installer.exe"
    $InnoUrl = "https://jrsoftware.org/download.php/is.exe"

    try {
        Write-Host "  Downloading from $InnoUrl ..."
        Invoke-WebRequest -Uri $InnoUrl -OutFile $InnoInstaller -UseBasicParsing
        Write-Host "  Installing Inno Setup silently..."
        Start-Process $InnoInstaller -ArgumentList "/VERYSILENT /SUPPRESSMSGBOXES /NORESTART" -Wait
        Remove-Item $InnoInstaller -ErrorAction SilentlyContinue

        # Перевіряємо знову
        foreach ($p in $IsccPaths) {
            if (Test-Path $p) { $IsccExe = $p; break }
        }
    } catch {
        Write-Host "  WARNING: Could not download Inno Setup: $_" -ForegroundColor Yellow
    }
}

if ($IsccExe) {
    $IssScript = Join-Path $ScriptDir "punic_wars_setup.iss"
    Write-Host "  Using: $IsccExe"
    Write-Host "  Script: $IssScript"

    & $IsccExe $IssScript
    if ($LASTEXITCODE -eq 0) {
        $SetupFile = Join-Path $OutDir "PunicWarsCastra_Setup_v$Version.exe"
        if (Test-Path $SetupFile) {
            $SetupSize = [math]::Round((Get-Item $SetupFile).Length / 1MB, 1)
            Write-Host "  OK: PunicWarsCastra_Setup_v$Version.exe ($SetupSize MB)" -ForegroundColor Green
        } else {
            Write-Host "  OK: Installer built (check output/ folder)" -ForegroundColor Green
        }
    } else {
        Write-Host "  ERROR: Inno Setup compilation failed (exit code $LASTEXITCODE)" -ForegroundColor Red
    }
} else {
    Write-Host "  Inno Setup not available. Skipping .exe installer." -ForegroundColor Yellow
    Write-Host "  Install from: https://jrsoftware.org/isdl.php" -ForegroundColor Cyan
    Write-Host "  Then run: iscc installer\punic_wars_setup.iss" -ForegroundColor Cyan
}

# ─────────────────────────────────────────────
# Підсумок
# ─────────────────────────────────────────────
Write-Host ""
Write-Host "=== Done! Output files ===" -ForegroundColor Cyan
Get-ChildItem $OutDir | ForEach-Object {
    $size = [math]::Round($_.Length / 1MB, 1)
    Write-Host "  $($_.Name)  ($size MB)"
}
Write-Host ""
Write-Host "Output folder: $OutDir" -ForegroundColor Green
