# Скрипт для запуску гри з автоматичним скріншотом
# Використання: .\run_with_screenshot.ps1

Write-Host "=== Punic Wars: Castra - Auto Screenshot ===" -ForegroundColor Cyan
Write-Host ""

# Перевірка чи існує exe файл
if (-not (Test-Path "punic_wars.exe")) {
    Write-Host "Error: punic_wars.exe not found!" -ForegroundColor Red
    Write-Host "Run compile.bat first" -ForegroundColor Yellow
    exit 1
}

# Створюємо папку для скріншотів якщо не існує
$screenshotDir = "screenshots"
if (-not (Test-Path $screenshotDir)) {
    New-Item -ItemType Directory -Path $screenshotDir | Out-Null
    Write-Host "Created screenshots directory" -ForegroundColor Green
}

# Запускаємо гру
Write-Host "Starting game..." -ForegroundColor Green
$process = Start-Process -FilePath ".\punic_wars.exe" -PassThru

# Чекаємо 3 секунди щоб гра завантажилась
Write-Host "Waiting 3 seconds for game to load..." -ForegroundColor Yellow
Start-Sleep -Seconds 3

# Робимо скріншот
Write-Host "Taking screenshot..." -ForegroundColor Green

# Генеруємо ім'я файлу з датою та часом
$timestamp = Get-Date -Format "yyyy-MM-dd_HH-mm-ss"
$screenshotPath = Join-Path $screenshotDir "menu_$timestamp.png"

# Використовуємо .NET для створення скріншота
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

# Знаходимо вікно гри
$windowTitle = "Punic Wars: Castra"
$hwnd = $null

# Пробуємо знайти вікно
for ($i = 0; $i -lt 10; $i++) {
    $windows = Get-Process | Where-Object { $_.MainWindowTitle -like "*Punic*" -or $_.ProcessName -eq "punic_wars" }
    if ($windows) {
        $hwnd = $windows[0].MainWindowHandle
        break
    }
    Start-Sleep -Milliseconds 500
}

if ($hwnd -and $hwnd -ne 0) {
    # Активуємо вікно
    [void][System.Reflection.Assembly]::LoadWithPartialName("System.Windows.Forms")
    [System.Windows.Forms.SendKeys]::SendWait("%")
    
    # Робимо скріншот всього екрану
    $bounds = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
    $bitmap = New-Object System.Drawing.Bitmap $bounds.Width, $bounds.Height
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $graphics.CopyFromScreen($bounds.Location, [System.Drawing.Point]::Empty, $bounds.Size)
    
    # Зберігаємо скріншот
    $bitmap.Save($screenshotPath, [System.Drawing.Imaging.ImageFormat]::Png)
    
    # Очищаємо ресурси
    $graphics.Dispose()
    $bitmap.Dispose()
    
    Write-Host "Screenshot saved: $screenshotPath" -ForegroundColor Green
} else {
    Write-Host "Warning: Could not find game window for screenshot" -ForegroundColor Yellow
    Write-Host "Game is running, but screenshot failed" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Game is running. Press any key to close it..." -ForegroundColor Cyan
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

# Закриваємо гру
if (-not $process.HasExited) {
    Write-Host "Closing game..." -ForegroundColor Yellow
    $process.CloseMainWindow() | Out-Null
    Start-Sleep -Seconds 2
    if (-not $process.HasExited) {
        $process.Kill()
    }
}

Write-Host "Done!" -ForegroundColor Green
