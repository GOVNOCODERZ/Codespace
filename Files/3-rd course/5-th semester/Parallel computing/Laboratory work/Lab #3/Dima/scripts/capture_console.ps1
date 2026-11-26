param(
    [Parameter(Mandatory=$true)][string]$CommandText,
    [Parameter(Mandatory=$true)][string]$WindowTitle,
    [Parameter(Mandatory=$true)][string]$OutputPng,
    [int]$Cols = 114,
    [int]$Lines = 40,
    [int]$TimeoutSec = 90
)

Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;

public static class WinCapture {
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }

    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

    [DllImport("dwmapi.dll")]
    public static extern int DwmGetWindowAttribute(IntPtr hwnd, int dwAttribute, out RECT pvAttribute, int cbAttribute);

    [DllImport("user32.dll")]
    public static extern bool PrintWindow(IntPtr hwnd, IntPtr hdcBlt, uint nFlags);

    [DllImport("user32.dll")]
    public static extern bool SetForegroundWindow(IntPtr hWnd);

    [DllImport("user32.dll")]
    public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

    public const int DWMWA_EXTENDED_FRAME_BOUNDS = 9;
    public const uint PW_RENDERFULLCONTENT = 0x00000002;
}
"@

$workDir = (Get-Location).Path
$readyFile = Join-Path $workDir "results\.capture_ready.tmp"
$stopFile  = Join-Path $workDir "results\.capture_stop.tmp"
Remove-Item -Force -ErrorAction SilentlyContinue $readyFile, $stopFile

# Запускаем классическое окно консоли Windows (conhost.exe + cmd.exe) с реальным выполнением программы
$batPath = Join-Path $workDir "results\.run_capture.bat"
$batContent = @"
@echo off
chcp 65001 >nul
mode con: cols=$Cols lines=$Lines
title $WindowTitle
cd /d "$workDir"
echo $workDir^>$CommandText
$CommandText
echo DONE > "$readyFile"
:waitloop
if exist "$stopFile" goto endloop
ping -n 1 127.0.0.1 >nul
goto waitloop
:endloop
exit
"@
[System.IO.File]::WriteAllText($batPath, $batContent, (New-Object System.Text.UTF8Encoding($false)))

$proc = Start-Process -FilePath "conhost.exe" -ArgumentList @("cmd.exe", "/c", "`"$batPath`"") -PassThru

try {
    # Ожидаем завершения выполнения программы внутри окна консоли
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    while (-not (Test-Path $readyFile)) {
        if ($sw.Elapsed.TotalSeconds -gt $TimeoutSec) {
            throw "Timeout waiting for command completion in console window."
        }
        Start-Sleep -Milliseconds 250
    }
    Start-Sleep -Milliseconds 600

    # Находим дескриптор окна консоли по заголовку
    $hwnd = [IntPtr]::Zero
    $procs = Get-Process | Where-Object { $_.MainWindowTitle -like "*$WindowTitle*" -and $_.MainWindowHandle -ne 0 }
    if ($procs) {
        $hwnd = ($procs | Select-Object -First 1).MainWindowHandle
    } elseif ($proc.MainWindowHandle -ne 0) {
        $hwnd = $proc.MainWindowHandle
    }

    if ($hwnd -eq [IntPtr]::Zero) {
        throw "Could not find console window handle (HWND)."
    }

    [WinCapture]::ShowWindow($hwnd, 9) | Out-Null
    [WinCapture]::SetForegroundWindow($hwnd) | Out-Null
    Start-Sleep -Milliseconds 400

    $rect = New-Object WinCapture+RECT
    $hr = [WinCapture]::DwmGetWindowAttribute($hwnd, [WinCapture]::DWMWA_EXTENDED_FRAME_BOUNDS, [ref]$rect, [System.Runtime.InteropServices.Marshal]::SizeOf($rect))
    if ($hr -ne 0) {
        [WinCapture]::GetWindowRect($hwnd, [ref]$rect) | Out-Null
    }

    $w = $rect.Right - $rect.Left
    $h = $rect.Bottom - $rect.Top
    if ($w -le 0 -or $h -le 0) {
        throw "Invalid window bounds: ${w}x${h}"
    }

    $bmp = New-Object System.Drawing.Bitmap($w, $h, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $hdc = $g.GetHdc()
    $ok = [WinCapture]::PrintWindow($hwnd, $hdc, [WinCapture]::PW_RENDERFULLCONTENT)
    $g.ReleaseHdc($hdc)

    # Проверяем, не пустой ли снимок (при необходимости пробуем CopyFromScreen)
    $nonBlack = 0
    for ($sy = 10; $sy -lt [Math]::Min($h - 10, 400); $sy += 20) {
        for ($sx = 10; $sx -lt [Math]::Min($w - 10, 600); $sx += 30) {
            $px = $bmp.GetPixel($sx, $sy)
            if ($px.R -gt 20 -or $px.G -gt 20 -or $px.B -gt 20) { $nonBlack++ }
        }
    }
    if (-not $ok -or $nonBlack -lt 5) {
        $g.CopyFromScreen($rect.Left, $rect.Top, 0, 0, (New-Object System.Drawing.Size($w, $h)), [System.Drawing.CopyPixelOperation]::SourceCopy)
    }

    $g.Dispose()
    $bmp.Save($OutputPng, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    Write-Host "Captured real console screenshot: $OutputPng (${w}x${h}, PrintWindow=$ok, nonBlack=$nonBlack)"
}
finally {
    "STOP" | Out-File -FilePath $stopFile -Encoding ascii
    Start-Sleep -Milliseconds 400
    if (-not $proc.HasExited) {
        Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
    }
    Remove-Item -Force -ErrorAction SilentlyContinue $readyFile, $stopFile, $batPath
}
