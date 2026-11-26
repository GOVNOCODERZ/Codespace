param([string]$FilterName = "")

Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;

public static class WinCap5 {
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
    [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
    [DllImport("user32.dll")] public static extern bool SetProcessDpiAwarenessContext(IntPtr value);
    [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr hWnd, out RECT lpRect);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr hWnd, ref POINT lpPoint);
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr hwnd, IntPtr hdcBlt, uint nFlags);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
    public const uint PW_RENDERFULLCONTENT = 0x00000002;
}
"@

try { [WinCap5]::SetProcessDpiAwarenessContext([IntPtr](-4)) | Out-Null } catch { [WinCap5]::SetProcessDPIAware() | Out-Null }

function Read-TextAuto([string]$path) {
    $bytes = [System.IO.File]::ReadAllBytes($path)
    if ($bytes.Length -ge 2 -and $bytes[0] -eq 0xFF -and $bytes[1] -eq 0xFE) {
        return [System.Text.Encoding]::Unicode.GetString($bytes, 2, $bytes.Length - 2)
    } elseif ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF) {
        return [System.Text.Encoding]::UTF8.GetString($bytes, 3, $bytes.Length - 3)
    }
    return [System.Text.Encoding]::UTF8.GetString($bytes)
}

$workDir = (Get-Location).Path
$l5 = (Read-TextAuto (Join-Path $workDir "Lab5\results\lab5_console.txt")) -split "`r?`n"

$slices = @(
    @{ Name="c5_p1.png"; Lines=$l5[0..18];    Cols=104; Title="L5_P1" },
    @{ Name="c5_p2.png"; Lines=$l5[19..63];   Cols=104; Title="L5_P2" },
    @{ Name="c5_p3.png"; Lines=$l5[65..109];  Cols=104; Title="L5_P3" },
    @{ Name="c5_p4.png"; Lines=$l5[111..155]; Cols=104; Title="L5_P4" },
    @{ Name="c5_p5.png"; Lines=$l5[157..201]; Cols=104; Title="L5_P5" },
    @{ Name="c5_p6.png"; Lines=$l5[203..251]; Cols=104; Title="L5_P6" },
    @{ Name="c5_p7.png"; Lines=$l5[253..297]; Cols=104; Title="L5_P7" },
    @{ Name="c5_p8.png"; Lines=$l5[299..343]; Cols=104; Title="L5_P8" },
    @{ Name="c5_p9.png"; Lines=$l5[345..391]; Cols=104; Title="L5_P9" }
)

$readyFile = Join-Path $workDir "Lab5\results\.cap5_ready.tmp"
$stopFile  = Join-Path $workDir "Lab5\results\.cap5_stop.tmp"
$txtFile   = Join-Path $workDir "Lab5\results\.cap5_slice.txt"
$batFile   = Join-Path $workDir "Lab5\results\.cap5_slice.bat"

foreach ($sl in $slices) {
    if ($FilterName -and $sl.Name -ne $FilterName) { continue }
    Remove-Item -Force -ErrorAction SilentlyContinue $readyFile, $stopFile
    [System.IO.File]::WriteAllText($txtFile, ($sl.Lines -join "`r`n"), (New-Object System.Text.UTF8Encoding($false)))

    $winRows = [Math]::Max(22, $sl.Lines.Count + 3)
    $bat = @"
@echo off
chcp 65001 >nul
mode con: cols=$($sl.Cols) lines=$winRows
title $($sl.Title)
cls
C:\msys64\ucrt64\bin\python.exe -c "import sys; sys.stdout.buffer.write(open(sys.argv[1],'rb').read())" "$txtFile"
echo.
echo OK > "$readyFile"
:w
if exist "$stopFile" exit
ping -n 1 127.0.0.1 >nul
goto w
"@
    [System.IO.File]::WriteAllText($batFile, $bat, (New-Object System.Text.UTF8Encoding($false)))

    $proc = Start-Process -FilePath "conhost.exe" -ArgumentList @("cmd.exe", "/c", "`"$batFile`"") -PassThru
    try {
        $sw = [System.Diagnostics.Stopwatch]::StartNew()
        while (-not (Test-Path $readyFile)) {
            if ($sw.Elapsed.TotalSeconds -gt 20) { throw "Timeout on $($sl.Name)" }
            Start-Sleep -Milliseconds 80
        }
        Start-Sleep -Milliseconds 220

        $hwnd = [IntPtr]::Zero
        $found = Get-Process | Where-Object { $_.MainWindowTitle -like "*$($sl.Title)*" -and $_.MainWindowHandle -ne 0 } | Select-Object -First 1
        if ($found) { $hwnd = $found.MainWindowHandle }
        elseif ($proc.MainWindowHandle -ne 0) { $hwnd = $proc.MainWindowHandle }
        if ($hwnd -eq [IntPtr]::Zero) { throw "No HWND for $($sl.Name)" }

        [WinCap5]::ShowWindow($hwnd, 9) | Out-Null
        Start-Sleep -Milliseconds 120

        $wRect = New-Object WinCap5+RECT; $cRect = New-Object WinCap5+RECT
        [WinCap5]::GetWindowRect($hwnd, [ref]$wRect) | Out-Null
        [WinCap5]::GetClientRect($hwnd, [ref]$cRect) | Out-Null
        $pt = New-Object WinCap5+POINT; $pt.X = 0; $pt.Y = 0
        [WinCap5]::ClientToScreen($hwnd, [ref]$pt) | Out-Null

        $fullW = $wRect.Right - $wRect.Left; $fullH = $wRect.Bottom - $wRect.Top
        $fullBmp = New-Object System.Drawing.Bitmap($fullW, $fullH, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        $g = [System.Drawing.Graphics]::FromImage($fullBmp)
        $hdc = $g.GetHdc()
        $ok = [WinCap5]::PrintWindow($hwnd, $hdc, [WinCap5]::PW_RENDERFULLCONTENT)
        $g.ReleaseHdc($hdc); $g.Dispose()

        $cx = [Math]::Max(0, $pt.X - $wRect.Left)
        $cy = [Math]::Max(0, $pt.Y - $wRect.Top)
        $cw = [Math]::Min($cRect.Right - $cRect.Left, $fullW - $cx)
        $ch = [Math]::Min($cRect.Bottom - $cRect.Top, $fullH - $cy)

        $bg = $fullBmp.GetPixel($cx + $cw - 4, $cy + 4)
        $minX = $cw; $minY = $ch; $maxX = 0; $maxY = 0
        $lineH = [Math]::Max(18, [int]($ch / $winRows))
        $scanMaxY = [Math]::Max(20, $ch - $lineH - 6)
        for ($y = 0; $y -lt $scanMaxY; $y++) {
            for ($x = 0; $x -lt $cw; $x++) {
                $px = $fullBmp.GetPixel($cx + $x, $cy + $y)
                if ([Math]::Abs([int]$px.R - [int]$bg.R) -gt 25 -or [Math]::Abs([int]$px.G - [int]$bg.G) -gt 25 -or [Math]::Abs([int]$px.B - [int]$bg.B) -gt 25) {
                    if ($x -lt $minX) { $minX = $x }
                    if ($y -lt $minY) { $minY = $y }
                    if ($x -gt $maxX) { $maxX = $x }
                    if ($y -gt $maxY) { $maxY = $y }
                }
            }
        }
        if ($maxX -le $minX -or $maxY -le $minY) { $minX = 0; $minY = 0; $maxX = $cw - 1; $maxY = $ch - 1 }
        $pad = 16
        $cropX = [Math]::Max($cx, $cx + $minX - $pad)
        $cropY = [Math]::Max($cy, $cy + $minY - $pad)
        $cropR = [Math]::Min($cx + $cw, $cx + $maxX + $pad + 1)
        $cropB = [Math]::Min($cy + $ch, $cy + $maxY + $pad + 1)

        $rectCrop = New-Object System.Drawing.Rectangle($cropX, $cropY, ($cropR - $cropX), ($cropB - $cropY))
        $outBmp = $fullBmp.Clone($rectCrop, $fullBmp.PixelFormat)
        $outBmp.Save((Join-Path $workDir ("Lab5\results\" + $sl.Name)), [System.Drawing.Imaging.ImageFormat]::Png)
        $outBmp.Dispose(); $fullBmp.Dispose()
        Write-Host "[OK] $($sl.Name): crop=$($rectCrop.Width)x$($rectCrop.Height) (PrintWindow=$ok)"
    }
    finally {
        "STOP" | Out-File -FilePath $stopFile -Encoding ascii
        Start-Sleep -Milliseconds 150
        if (-not $proc.HasExited) { Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue }
    }
}

Remove-Item -Force -ErrorAction SilentlyContinue $readyFile, $stopFile, $txtFile, $batFile
Write-Host "All 9 Lab 5 real console screenshots captured."
