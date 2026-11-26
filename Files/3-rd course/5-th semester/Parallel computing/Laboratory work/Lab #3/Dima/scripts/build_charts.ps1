Add-Type -AssemblyName System.Drawing

function Format-Ru([double]$val, [int]$dec = 2) {
    return $val.ToString("F$dec", [System.Globalization.CultureInfo]::GetCultureInfo("ru-RU"))
}

function New-Canvas([int]$W, [int]$H) {
    $bmp = New-Object System.Drawing.Bitmap($W, $H, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::ClearTypeGridFit
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $g.Clear([System.Drawing.Color]::White)
    return @{ Bmp = $bmp; G = $g }
}

function Draw-AxesAndGrid($g, [float]$x0, [float]$y0, [float]$pw, [float]$ph,
                          [double]$xmin, [double]$xmax, [double]$ymin, [double]$ymax,
                          [int]$nxTicks, [int]$nyTicks, [string]$xLabel, [string]$yLabel,
                          [string]$title, [int]$xDec = 0, [int]$yDec = 2, [double[]]$customXTicks = $null) {
    $fTitle = New-Object System.Drawing.Font("Times New Roman", 20, [System.Drawing.FontStyle]::Bold)
    $fAxis  = New-Object System.Drawing.Font("Times New Roman", 15, [System.Drawing.FontStyle]::Bold)
    $fTick  = New-Object System.Drawing.Font("Times New Roman", 13)
    $bBlack = [System.Drawing.Brushes]::Black
    $pAxis  = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(40,40,40), 2.0)
    $pGrid  = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(215,215,215), 1.0)
    $pGrid.DashStyle = [System.Drawing.Drawing2D.DashStyle]::Dash

    # Title
    $sfCenter = New-Object System.Drawing.StringFormat
    $sfCenter.Alignment = [System.Drawing.StringAlignment]::Center
    $g.DrawString($title, $fTitle, $bBlack, ($x0 + $pw / 2.0), ($y0 - 52.0), $sfCenter)

    # Horizontal grid & Y ticks
    $sfRight = New-Object System.Drawing.StringFormat
    $sfRight.Alignment = [System.Drawing.StringAlignment]::Far
    $sfRight.LineAlignment = [System.Drawing.StringAlignment]::Center
    for ($i = 0; $i -le $nyTicks; $i++) {
        $yv = $ymin + ($ymax - $ymin) * ($i / [double]$nyTicks)
        $py = $y0 + $ph - [float](($yv - $ymin) / ($ymax - $ymin) * $ph)
        if ($i -gt 0 -and $i -lt $nyTicks) {
            $g.DrawLine($pGrid, $x0, $py, $x0 + $pw, $py)
        }
        $g.DrawLine($pAxis, $x0 - 6, $py, $x0, $py)
        $g.DrawString((Format-Ru $yv $yDec), $fTick, $bBlack, ($x0 - 10), $py, $sfRight)
    }

    # Vertical grid & X ticks
    $xTickVals = @()
    if ($null -ne $customXTicks) {
        $xTickVals = $customXTicks
    } else {
        for ($i = 0; $i -le $nxTicks; $i++) {
            $xTickVals += ($xmin + ($xmax - $xmin) * ($i / [double]$nxTicks))
        }
    }
    foreach ($xv in $xTickVals) {
        if ($xv -lt $xmin - 1e-9 -or $xv -gt $xmax + 1e-9) { continue }
        $px = $x0 + [float](($xv - $xmin) / ($xmax - $xmin) * $pw)
        if ($xv -gt $xmin + 1e-9 -and $xv -lt $xmax - 1e-9) {
            $g.DrawLine($pGrid, $px, $y0, $px, $y0 + $ph)
        }
        $g.DrawLine($pAxis, $px, $y0 + $ph, $px, $y0 + $ph + 6)
        $g.DrawString((Format-Ru $xv $xDec), $fTick, $bBlack, $px, ($y0 + $ph + 10), $sfCenter)
    }

    # Plot frame
    $g.DrawRectangle($pAxis, $x0, $y0, $pw, $ph)

    # Axis labels
    $g.DrawString($xLabel, $fAxis, $bBlack, ($x0 + $pw / 2.0), ($y0 + $ph + 42), $sfCenter)

    $state = $g.Save()
    $g.TranslateTransform($x0 - 82, $y0 + $ph / 2.0)
    $g.RotateTransform(-90)
    $g.DrawString($yLabel, $fAxis, $bBlack, 0, 0, $sfCenter)
    $g.Restore($state)

    $fTitle.Dispose(); $fAxis.Dispose(); $fTick.Dispose(); $pAxis.Dispose(); $pGrid.Dispose()
}

function Draw-Legend($g, [float]$lx, [float]$ly, [array]$items) {
    $fLeg = New-Object System.Drawing.Font("Times New Roman", 12.5)
    $maxW = 0.0
    foreach ($it in $items) {
        $sz = $g.MeasureString($it.Label, $fLeg)
        if ($sz.Width -gt $maxW) { $maxW = $sz.Width }
    }
    $rowH = 26.0
    $boxW = $maxW + 64.0
    $boxH = $items.Count * $rowH + 14.0

    $bBg = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(242, 255, 255, 255))
    $pBr = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(140, 140, 140), 1.2)
    $g.FillRectangle($bBg, $lx, $ly, $boxW, $boxH)
    $g.DrawRectangle($pBr, $lx, $ly, $boxW, $boxH)

    for ($i = 0; $i -lt $items.Count; $i++) {
        $it = $items[$i]
        $cy = $ly + 8.0 + $i * $rowH + $rowH / 2.0
        $col = $it.Color
        if ($it.Kind -eq "dot") {
            $br = New-Object System.Drawing.SolidBrush($col)
            $g.FillEllipse($br, $lx + 18, $cy - 5, 10, 10)
            $br.Dispose()
        } elseif ($it.Kind -eq "cross") {
            $pen = New-Object System.Drawing.Pen($col, 2.5)
            $g.DrawLine($pen, $lx + 18, $cy - 5, $lx + 28, $cy + 5)
            $g.DrawLine($pen, $lx + 18, $cy + 5, $lx + 28, $cy - 5)
            $pen.Dispose()
        } else {
            $pen = New-Object System.Drawing.Pen($col, $it.Width)
            $pen.DashStyle = $it.Dash
            $g.DrawLine($pen, $lx + 10, $cy, $lx + 42, $cy)
            if ($it.Marker) {
                $br = New-Object System.Drawing.SolidBrush($col)
                $g.FillEllipse($br, $lx + 22, $cy - 4, 8, 8)
                $br.Dispose()
            }
            $pen.Dispose()
        }
        $g.DrawString($it.Label, $fLeg, [System.Drawing.Brushes]::Black, ($lx + 50), ($cy - 10))
    }
    $fLeg.Dispose(); $bBg.Dispose(); $pBr.Dispose()
}

function Draw-ScatterCI($rows, [string]$title, [string]$outPath, [int]$yDec = 3) {
    $c = New-Canvas 1600 940
    $g = $c.G
    $x0 = 130.0; $y0 = 90.0; $pw = 1410.0; $ph = 740.0

    $times = $rows | ForEach-Object { [double]$_.time_ms }
    $med = [double]$rows[0].median_ms
    $sd  = [double]$rows[0].sd_ms
    $kept = $rows | Where-Object { [int]$_.in_interval -eq 1 } | ForEach-Object { [double]$_.time_ms }
    $trusted = ($kept | Measure-Object -Average).Average
    $nPts = $rows.Count

    $minV = ($times | Measure-Object -Minimum).Minimum
    $maxV = ($times | Measure-Object -Maximum).Maximum
    $ymin = [Math]::Min($minV, $med - $sd)
    $ymax = [Math]::Max($maxV, $med + $sd)
    $pad = ($ymax - $ymin) * 0.14
    if ($pad -le 0) { $pad = 0.05 }
    $ymin = [Math]::Max(0.0, $ymin - $pad)
    $ymax = $ymax + $pad * 1.4

    Draw-AxesAndGrid $g $x0 $y0 $pw $ph 0 ($nPts + 2) $ymin $ymax 11 7 "Номер замера" "Время выполнения, мс" $title 0 $yDec

    # Shaded confidence band [med - sd, med + sd]
    $yLow = $y0 + $ph - [float]((([Math]::Max($ymin, $med - $sd)) - $ymin) / ($ymax - $ymin) * $ph)
    $yHi  = $y0 + $ph - [float]((([Math]::Min($ymax, $med + $sd)) - $ymin) / ($ymax - $ymin) * $ph)
    $bBand = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(30, 34, 139, 34))
    $g.FillRectangle($bBand, $x0, $yHi, $pw, ($yLow - $yHi))
    $bBand.Dispose()

    # Reference lines: med, med +/- sd, trusted avg
    $pMed = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(20, 130, 50), 2.5)
    $pLim = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(20, 130, 50), 2.0)
    $pLim.DashStyle = [System.Drawing.Drawing2D.DashStyle]::Dash
    $pAvg = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(215, 110, 0), 2.5)
    $pAvg.DashStyle = [System.Drawing.Drawing2D.DashStyle]::DashDot

    $pyMed = $y0 + $ph - [float](($med - $ymin) / ($ymax - $ymin) * $ph)
    $pyAvg = $y0 + $ph - [float](($trusted - $ymin) / ($ymax - $ymin) * $ph)
    $g.DrawLine($pMed, $x0, $pyMed, $x0 + $pw, $pyMed)
    $g.DrawLine($pLim, $x0, $yHi,  $x0 + $pw, $yHi)
    $g.DrawLine($pLim, $x0, $yLow, $x0 + $pw, $yLow)
    $g.DrawLine($pAvg, $x0, $pyAvg, $x0 + $pw, $pyAvg)

    # Scatter points
    $bIn = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(220, 31, 119, 180))
    $pOut = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(220, 214, 39, 40), 2.8)
    foreach ($r in $rows) {
        $rx = [double]$r.run_idx
        $ry = [double]$r.time_ms
        $px = $x0 + [float]($rx / ($nPts + 2.0) * $pw)
        $py = $y0 + $ph - [float](($ry - $ymin) / ($ymax - $ymin) * $ph)
        if ([int]$r.in_interval -eq 1) {
            $g.FillEllipse($bIn, $px - 5, $py - 5, 10, 10)
        } else {
            $g.DrawLine($pOut, $px - 6, $py - 6, $px + 6, $py + 6)
            $g.DrawLine($pOut, $px - 6, $py + 6, $px + 6, $py - 6)
        }
    }

    $disc = $nPts - $kept.Count
    Draw-Legend $g ($x0 + $pw - 470) ($y0 + 16) @(
        @{ Kind="dot";   Color=[System.Drawing.Color]::FromArgb(31,119,180); Label="Принятые замеры ($($kept.Count) из $nPts)" },
        @{ Kind="cross"; Color=[System.Drawing.Color]::FromArgb(214,39,40);  Label="Отброшенные выбросы ($disc)" },
        @{ Kind="line";  Color=[System.Drawing.Color]::FromArgb(20,130,50);  Width=2.5; Dash=[System.Drawing.Drawing2D.DashStyle]::Solid;   Marker=$false; Label="Медиана med = $(Format-Ru $med 4) мс" },
        @{ Kind="line";  Color=[System.Drawing.Color]::FromArgb(20,130,50);  Width=2.0; Dash=[System.Drawing.Drawing2D.DashStyle]::Dash;    Marker=$false; Label="Границы med ± SD (SD = $(Format-Ru $sd 4) мс)" },
        @{ Kind="line";  Color=[System.Drawing.Color]::FromArgb(215,110,0);  Width=2.5; Dash=[System.Drawing.Drawing2D.DashStyle]::DashDot; Marker=$false; Label="Итог AvgTrustedIntervalMed = $(Format-Ru $trusted 4) мс" }
    )

    $pMed.Dispose(); $pLim.Dispose(); $pAvg.Dispose(); $bIn.Dispose(); $pOut.Dispose()
    $c.Bmp.Save($outPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $c.Bmp.Dispose()
}

function Draw-Polyline($g, $pts, [float]$x0, [float]$y0, [float]$pw, [float]$ph,
                       [double]$xmin, [double]$xmax, [double]$ymin, [double]$ymax,
                       [System.Drawing.Color]$col, [float]$width, $dash, [bool]$markers) {
    $pen = New-Object System.Drawing.Pen($col, $width)
    $pen.DashStyle = $dash
    $br  = New-Object System.Drawing.SolidBrush($col)
    $screenPts = @()
    foreach ($pt in $pts) {
        $px = $x0 + [float](($pt[0] - $xmin) / ($xmax - $xmin) * $pw)
        $py = $y0 + $ph - [float](([Math]::Min($ymax, [Math]::Max($ymin, $pt[1])) - $ymin) / ($ymax - $ymin) * $ph)
        $screenPts += (New-Object System.Drawing.PointF($px, $py))
    }
    if ($screenPts.Count -ge 2) {
        $g.DrawLines($pen, $screenPts)
    }
    if ($markers) {
        foreach ($sp in $screenPts) {
            $g.FillEllipse($br, $sp.X - 5, $sp.Y - 5, 10, 10)
            $g.DrawEllipse([System.Drawing.Pens]::White, $sp.X - 5, $sp.Y - 5, 10, 10)
        }
    }
    $pen.Dispose(); $br.Dispose()
}

function Estimate-F($pts) {
    $num = 0.0; $den = 0.0
    foreach ($pt in $pts) {
        $p = [double]$pt[0]; $sp = [double]$pt[1]
        if ($p -le 1 -or $p -gt 6 -or $sp -le 0) { continue }
        $x = 1.0 - 1.0 / $p
        $y = (1.0 / $sp) - (1.0 / $p)
        $num += $x * $y; $den += $x * $x
    }
    if ($den -le 0) { return 0.1 }
    return [Math]::Max(0.005, [Math]::Min(0.95, $num / $den))
}

# =============================================================================
# Загрузка экспериментальных CSV
# =============================================================================
$t31 = Import-Csv "results/task3_1_summary.csv"
$t32 = Import-Csv "results/task3_2_summary.csv"
$r31 = Import-Csv "results/task3_1_raw_samples.csv"
$r32 = Import-Csv "results/task3_2_raw_samples.csv"

# 1. Точечная диаграмма доверительного интервала — Задание 3.1
$ci31 = $r31 | Where-Object { $_.algo -eq "QuickSort_Par" -and [int]$_.threads -eq 4 }
Draw-ScatterCI $ci31 "Доверительный интервал: QuickSort (int, n = 50 000, p = 4, N = 110)" "results/chart_ci_task31.png" 3

# 2. Точечная диаграмма доверительного интервала — Задание 3.2
$ci32 = $r32 | Where-Object { [int]$_.img_id -eq 1 -and $_.algo -eq "Median_Shell" -and [int]$_.ksize -eq 7 -and [int]$_.threads -eq 4 }
Draw-ScatterCI $ci32 "Доверительный интервал: Медианный фильтр Шелла (480x270, K = 7, p = 4, N = 110)" "results/chart_ci_task32.png" 1

# 3. График ускорения Sp(p) для Задания 3.1 + семейство кривых закона Амдала
{
    $c = New-Canvas 1600 960
    $g = $c.G
    $x0 = 130.0; $y0 = 90.0; $pw = 1410.0; $ph = 760.0
    $xmax = 16.0; $ymax = 8.0

    Draw-AxesAndGrid $g $x0 $y0 $pw $ph 1 $xmax 0 $ymax 15 8 "Число потоков, p" "Ускорение, Sp(n)" `
        "Ускорение алгоритмов сортировки (n = 50 000, int и double) и кривые закона Амдала" 0 1 @(1,2,3,4,6,8,12,16)

    # Вертикальная линия 6 физических ядер CPU
    $pCore = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(140, 120, 120, 120), 1.8)
    $pCore.DashStyle = [System.Drawing.Drawing2D.DashStyle]::Dot
    $px6 = $x0 + [float]((6.0 - 1.0) / ($xmax - 1.0) * $pw)
    $g.DrawLine($pCore, $px6, $y0, $px6, $y0 + $ph)
    $fNote = New-Object System.Drawing.Font("Times New Roman", 11.5, [System.Drawing.FontStyle]::Italic)
    $g.DrawString("6 физ. ядер (Ryzen 5 5600)", $fNote, [System.Drawing.Brushes]::DimGray, $px6 + 6, $y0 + 12)
    $pCore.Dispose(); $fNote.Dispose()

    # Семейство кривых Амдала (f = 0.05, 0.15, 0.30, 0.50)
    $amdSpecs = @(
        @{ f=0.05; col=[System.Drawing.Color]::FromArgb(150,150,150); lbl="Закон Амдала (f = 0,05; S* = 20,0)" },
        @{ f=0.15; col=[System.Drawing.Color]::FromArgb(130,170,130); lbl="Закон Амдала (f = 0,15; S* = 6,67)" },
        @{ f=0.30; col=[System.Drawing.Color]::FromArgb(180,150,110); lbl="Закон Амдала (f = 0,30; S* = 3,33)" },
        @{ f=0.50; col=[System.Drawing.Color]::FromArgb(185,130,145); lbl="Закон Амдала (f = 0,50; S* = 2,00)" }
    )
    $leg = @()
    foreach ($as in $amdSpecs) {
        $pts = @()
        for ($pv = 1.0; $pv -le $xmax; $pv += 0.25) {
            $sp = 1.0 / ($as.f + (1.0 - $as.f) / $pv)
            $pts += ,@($pv, $sp)
        }
        Draw-Polyline $g $pts $x0 $y0 $pw $ph 1 $xmax 0 $ymax $as.col 2.0 ([System.Drawing.Drawing2D.DashStyle]::Dash) $false
        $leg += @{ Kind="line"; Color=$as.col; Width=2.0; Dash=[System.Drawing.Drawing2D.DashStyle]::Dash; Marker=$false; Label=$as.lbl }
    }

    # Экспериментальные кривые (Shell int/double, QuickSort int/double на n=50000)
    $expSpecs = @(
        @{ dt="int";    a="QuickSort"; col=[System.Drawing.Color]::FromArgb(31,119,180);  lbl="QuickSort (эксп., int, n = 50 000)" },
        @{ dt="double"; a="QuickSort"; col=[System.Drawing.Color]::FromArgb(100,175,230); lbl="QuickSort (эксп., double, n = 50 000)" },
        @{ dt="int";    a="Shell";     col=[System.Drawing.Color]::FromArgb(214,39,40);   lbl="Шелла (эксп., int, n = 50 000)" },
        @{ dt="double"; a="Shell";     col=[System.Drawing.Color]::FromArgb(240,135,45);  lbl="Шелла (эксп., double, n = 50 000)" }
    )
    foreach ($es in $expSpecs) {
        $pts = @()
        foreach ($p in @(1,2,3,4,6,8,12,16)) {
            $aname = if ($p -eq 1) { "$($es.a)_Seq" } else { "$($es.a)_Par" }
            $row = $t31 | Where-Object { $_.dtype -eq $es.dt -and [int]$_.n -eq 50000 -and $_.algo -eq $aname -and [int]$_.threads -eq $p } | Select-Object -First 1
            if ($row) { $pts += ,@([double]$p, [double]$row.speedup) }
        }
        Draw-Polyline $g $pts $x0 $y0 $pw $ph 1 $xmax 0 $ymax $es.col 3.2 ([System.Drawing.Drawing2D.DashStyle]::Solid) $true
        $leg += @{ Kind="line"; Color=$es.col; Width=3.2; Dash=[System.Drawing.Drawing2D.DashStyle]::Solid; Marker=$true; Label=$es.lbl }
    }

    Draw-Legend $g ($x0 + $pw - 445) ($y0 + 18) $leg
    $c.Bmp.Save("results/chart_speedup_amdahl_task31.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $c.Bmp.Dispose()
}.Invoke()

# 4. График ускорения Sp(p) для Задания 3.2 (НД4: 1280x720, K=7,11,15) + Законы Амдала и Густавсона
{
    $c = New-Canvas 1600 960
    $g = $c.G
    $x0 = 130.0; $y0 = 90.0; $pw = 1410.0; $ph = 760.0
    $xmax = 16.0; $ymax = 14.0

    Draw-AxesAndGrid $g $x0 $y0 $pw $ph 1 $xmax 0 $ymax 15 7 "Число потоков, p" "Ускорение, Sp(n)" `
        "Ускорение медианного фильтра (1280x720) и сравнение с законами Амдала / Густавсона–Барсиса" 0 0 @(1,2,3,4,6,8,12,16)

    $pCore = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(140,120,120,120), 1.8)
    $pCore.DashStyle = [System.Drawing.Drawing2D.DashStyle]::Dot
    $px6 = $x0 + [float]((6.0 - 1.0) / ($xmax - 1.0) * $pw)
    $px12 = $x0 + [float]((12.0 - 1.0) / ($xmax - 1.0) * $pw)
    $g.DrawLine($pCore, $px6, $y0, $px6, $y0 + $ph)
    $g.DrawLine($pCore, $px12, $y0, $px12, $y0 + $ph)
    $fNote = New-Object System.Drawing.Font("Times New Roman", 11.5, [System.Drawing.FontStyle]::Italic)
    $g.DrawString("6 физ. ядер", $fNote, [System.Drawing.Brushes]::DimGray, $px6 + 6, $y0 + 16)
    $g.DrawString("12 лог. потоков (SMT)", $fNote, [System.Drawing.Brushes]::DimGray, $px12 + 6, $y0 + 360)
    $pCore.Dispose(); $fNote.Dispose()

    # Экспериментальные серии на НД 4 (1280x720)
    $mSpecs = @(
        @{ a="Median_Shell";     k=15; col=[System.Drawing.Color]::FromArgb(31,119,180);  lbl="Медианный (Шелла), K = 15" },
        @{ a="Median_Shell";     k=11; col=[System.Drawing.Color]::FromArgb(44,160,44);   lbl="Медианный (Шелла), K = 11" },
        @{ a="Median_Shell";     k=7;  col=[System.Drawing.Color]::FromArgb(23,190,207);  lbl="Медианный (Шелла), K = 7" },
        @{ a="Median_QuickSort"; k=15; col=[System.Drawing.Color]::FromArgb(214,39,40);   lbl="Медианный (QuickSort), K = 15" },
        @{ a="Median_QuickSort"; k=7;  col=[System.Drawing.Color]::FromArgb(255,127,14);  lbl="Медианный (QuickSort), K = 7" }
    )
    $leg = @()
    $bestPts = @()
    foreach ($ms in $mSpecs) {
        $pts = @()
        foreach ($p in @(1,2,3,4,6,8,12,16)) {
            $row = $t32 | Where-Object { [int]$_.img_id -eq 4 -and $_.algo -eq $ms.a -and [int]$_.ksize -eq $ms.k -and [int]$_.threads -eq $p } | Select-Object -First 1
            if ($row) { $pts += ,@([double]$p, [double]$row.speedup) }
        }
        if ($ms.a -eq "Median_Shell" -and $ms.k -eq 15) { $bestPts = $pts }
        Draw-Polyline $g $pts $x0 $y0 $pw $ph 1 $xmax 0 $ymax $ms.col 3.0 ([System.Drawing.Drawing2D.DashStyle]::Solid) $true
        $leg += @{ Kind="line"; Color=$ms.col; Width=3.0; Dash=[System.Drawing.Drawing2D.DashStyle]::Solid; Marker=$true; Label=$ms.lbl }
    }

    # Оцениваем f для Медианного фильтра Шелла K=15 по p=2..12 и строим Амдала и Густавсона–Барсиса
    $numF = 0.0; $denF = 0.0
    foreach ($pt in $bestPts) {
        $pv = [double]$pt[0]; $spv = [double]$pt[1]
        if ($pv -lt 2 -or $pv -gt 12) { continue }
        $x = 1.0 - 1.0 / $pv; $y = (1.0 / $spv) - (1.0 / $pv)
        $numF += $x * $y; $denF += $x * $x
    }
    $fEst = [Math]::Max(0.005, $numF / $denF)
    $amdPts = @(); $gusPts = @()
    for ($pv = 1.0; $pv -le $xmax; $pv += 0.25) {
        $amdVal = 1.0 / ($fEst + (1.0 - $fEst) / $pv)
        if ($amdVal -le $ymax) { $amdPts += ,@($pv, $amdVal) }
        $gusVal = $fEst + (1.0 - $fEst) * $pv
        if ($gusVal -le $ymax) { $gusPts += ,@($pv, $gusVal) }
    }
    Draw-Polyline $g $amdPts $x0 $y0 $pw $ph 1 $xmax 0 $ymax ([System.Drawing.Color]::FromArgb(75,0,130)) 2.4 ([System.Drawing.Drawing2D.DashStyle]::Dash) $false
    Draw-Polyline $g $gusPts $x0 $y0 $pw $ph 1 $xmax 0 $ymax ([System.Drawing.Color]::FromArgb(110,110,110)) 2.2 ([System.Drawing.Drawing2D.DashStyle]::DashDot) $false
    $leg += @{ Kind="line"; Color=[System.Drawing.Color]::FromArgb(75,0,130); Width=2.4; Dash=[System.Drawing.Drawing2D.DashStyle]::Dash; Marker=$false; Label="Закон Амдала (f = $(Format-Ru $fEst 4))" }
    $leg += @{ Kind="line"; Color=[System.Drawing.Color]::FromArgb(110,110,110); Width=2.2; Dash=[System.Drawing.Drawing2D.DashStyle]::DashDot; Marker=$false; Label="Закон Густавсона–Барсиса (f = $(Format-Ru $fEst 4))" }

    Draw-Legend $g ($x0 + 25) ($y0 + 18) $leg
    $c.Bmp.Save("results/chart_speedup_task32.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $c.Bmp.Dispose()
}.Invoke()

# 5. Эффективность Ep(p) и нормированная стоимость Cp(p)/T1 (двухпанельный график)
{
    $c = New-Canvas 1680 780
    $g = $c.G
    $pw = 670.0; $ph = 560.0; $y0 = 95.0
    $xL = 115.0; $xR = 955.0
    $xmax = 16.0

    Draw-AxesAndGrid $g $xL $y0 $pw $ph 1 $xmax 0 1.2 15 6 "Число потоков, p" "Эффективность, Ep(n)" `
        "Эффективность параллельного выполнения Ep(p)" 0 2 @(1,2,3,4,6,8,12,16)
    Draw-AxesAndGrid $g $xR $y0 $pw $ph 1 $xmax 0 5.0 15 5 "Число потоков, p" "Отношение Cp(n) / T1(n)" `
        "Проверка стоимостной оптимальности Cp / T1" 0 1 @(1,2,3,4,6,8,12,16)

    # Ideal horizontal lines: Ep = 1.0 and Cp/T1 = 1.0
    Draw-Polyline $g @(@(1,1),@($xmax,1)) $xL $y0 $pw $ph 1 $xmax 0 1.2 ([System.Drawing.Color]::Gray) 1.8 ([System.Drawing.Drawing2D.DashStyle]::Dash) $false
    Draw-Polyline $g @(@(1,1),@($xmax,1)) $xR $y0 $pw $ph 1 $xmax 0 5.0 ([System.Drawing.Color]::Gray) 1.8 ([System.Drawing.Drawing2D.DashStyle]::Dash) $false

    $series = @(
        @{ src="31"; a="Shell";            col=[System.Drawing.Color]::FromArgb(214,39,40);  lbl="Зад. 3.1: Шелла (n=50 000)" },
        @{ src="31"; a="QuickSort";        col=[System.Drawing.Color]::FromArgb(255,127,14); lbl="Зад. 3.1: QuickSort (n=50 000)" },
        @{ src="32"; a="Median_Shell";     col=[System.Drawing.Color]::FromArgb(31,119,180); lbl="Зад. 3.2: Медиан. Шелла (K=15)" },
        @{ src="32"; a="Median_QuickSort"; col=[System.Drawing.Color]::FromArgb(44,160,44);  lbl="Зад. 3.2: Медиан. QuickSort (K=15)" }
    )
    $leg = @()
    foreach ($s in $series) {
        $ePts = @(); $cPts = @()
        $t1 = 1.0
        foreach ($p in @(1,2,3,4,6,8,12,16)) {
            if ($s.src -eq "31") {
                $an = if ($p -eq 1) { "$($s.a)_Seq" } else { "$($s.a)_Par" }
                $row = $t31 | Where-Object { $_.dtype -eq "int" -and [int]$_.n -eq 50000 -and $_.algo -eq $an -and [int]$_.threads -eq $p } | Select-Object -First 1
            } else {
                $row = $t32 | Where-Object { [int]$_.img_id -eq 4 -and $_.algo -eq $s.a -and [int]$_.ksize -eq 15 -and [int]$_.threads -eq $p } | Select-Object -First 1
            }
            if ($row) {
                if ($p -eq 1) { $t1 = [double]$row.trusted_avg_med_ms }
                $ePts += ,@([double]$p, [double]$row.efficiency)
                $cPts += ,@([double]$p, ([double]$row.cost_ms / $t1))
            }
        }
        Draw-Polyline $g $ePts $xL $y0 $pw $ph 1 $xmax 0 1.2 $s.col 2.8 ([System.Drawing.Drawing2D.DashStyle]::Solid) $true
        Draw-Polyline $g $cPts $xR $y0 $pw $ph 1 $xmax 0 5.0 $s.col 2.8 ([System.Drawing.Drawing2D.DashStyle]::Solid) $true
        $leg += @{ Kind="line"; Color=$s.col; Width=2.8; Dash=[System.Drawing.Drawing2D.DashStyle]::Solid; Marker=$true; Label=$s.lbl }
    }
    $leg += @{ Kind="line"; Color=[System.Drawing.Color]::Gray; Width=1.8; Dash=[System.Drawing.Drawing2D.DashStyle]::Dash; Marker=$false; Label="Идеальное значение (1,0)" }

    Draw-Legend $g ($xL + $pw - 355) ($y0 + 14) $leg
    Draw-Legend $g ($xR + 18) ($y0 + 14) $leg
    $c.Bmp.Save("results/chart_eff_cost.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $c.Bmp.Dispose()
}.Invoke()

# 6. Влияние размера окна Ksize и разрешения на время медианной фильтрации (p = 4)
{
    $c = New-Canvas 1600 900
    $g = $c.G
    $x0 = 135.0; $y0 = 90.0; $pw = 1405.0; $ph = 700.0

    # Сгруппированные столбцы по 4 изображениям (НД1..НД4) для K=7,11,15 (Shell vs QuickSort на p=4)
    $rowsP4 = $t32 | Where-Object { [int]$_.threads -eq 4 }
    $maxT = ($rowsP4 | ForEach-Object { [double]$_.trusted_avg_med_ms } | Measure-Object -Maximum).Maximum
    $ymax = [Math]::Ceiling($maxT / 200.0) * 200.0

    $fTitle = New-Object System.Drawing.Font("Times New Roman", 20, [System.Drawing.FontStyle]::Bold)
    $fAxis  = New-Object System.Drawing.Font("Times New Roman", 15, [System.Drawing.FontStyle]::Bold)
    $fTick  = New-Object System.Drawing.Font("Times New Roman", 12.5)
    $fVal   = New-Object System.Drawing.Font("Times New Roman", 10.5)
    $sfC = New-Object System.Drawing.StringFormat; $sfC.Alignment = [System.Drawing.StringAlignment]::Center
    $sfR = New-Object System.Drawing.StringFormat; $sfR.Alignment = [System.Drawing.StringAlignment]::Far; $sfR.LineAlignment = [System.Drawing.StringAlignment]::Center
    $pAxis = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(40,40,40), 2.0)
    $pGrid = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(215,215,215), 1.0); $pGrid.DashStyle = [System.Drawing.Drawing2D.DashStyle]::Dash

    $g.DrawString("Время медианной фильтрации при p = 4 потоках для разных Ksize и разрешений", $fTitle, [System.Drawing.Brushes]::Black, ($x0 + $pw/2), ($y0 - 52), $sfC)

    for ($i = 0; $i -le 6; $i++) {
        $yv = $ymax * $i / 6.0
        $py = $y0 + $ph - [float]($yv / $ymax * $ph)
        if ($i -gt 0 -and $i -lt 6) { $g.DrawLine($pGrid, $x0, $py, $x0 + $pw, $py) }
        $g.DrawLine($pAxis, $x0 - 6, $py, $x0, $py)
        $g.DrawString((Format-Ru $yv 0), $fTick, [System.Drawing.Brushes]::Black, ($x0 - 10), $py, $sfR)
    }

    $barSpecs = @(
        @{ a="Median_Shell";     k=7;  col=[System.Drawing.Color]::FromArgb(148,196,232); lbl="Шелла, K=7" },
        @{ a="Median_QuickSort"; k=7;  col=[System.Drawing.Color]::FromArgb(140,214,140); lbl="QuickSort, K=7" },
        @{ a="Median_Shell";     k=11; col=[System.Drawing.Color]::FromArgb(55,135,198);  lbl="Шелла, K=11" },
        @{ a="Median_QuickSort"; k=11; col=[System.Drawing.Color]::FromArgb(55,168,70);   lbl="QuickSort, K=11" },
        @{ a="Median_Shell";     k=15; col=[System.Drawing.Color]::FromArgb(20,75,135);   lbl="Шелла, K=15" },
        @{ a="Median_QuickSort"; k=15; col=[System.Drawing.Color]::FromArgb(20,108,35);   lbl="QuickSort, K=15" }
    )
    $dsLabels = @("НД 1 (480x270)", "НД 2 (640x360)", "НД 3 (960x540)", "НД 4 (1280x720)")
    $grpW = $pw / 4.0
    $barW = ($grpW - 60.0) / 6.0

    for ($gi = 0; $gi -lt 4; $gi++) {
        $iid = $gi + 1
        $gx = $x0 + $gi * $grpW + 30.0
        for ($bi = 0; $bi -lt 6; $bi++) {
            $bs = $barSpecs[$bi]
            $row = $rowsP4 | Where-Object { [int]$_.img_id -eq $iid -and $_.algo -eq $bs.a -and [int]$_.ksize -eq $bs.k } | Select-Object -First 1
            $val = [double]$row.trusted_avg_med_ms
            $bh = [float]($val / $ymax * $ph)
            $bx = $gx + $bi * $barW
            $by = $y0 + $ph - $bh
            $br = New-Object System.Drawing.SolidBrush($bs.col)
            $g.FillRectangle($br, $bx + 2, $by, $barW - 4, $bh)
            $g.DrawRectangle([System.Drawing.Pens]::DimGray, $bx + 2, $by, $barW - 4, $bh)
            $br.Dispose()
            if ($bh -gt 18) {
                $g.DrawString((Format-Ru $val 0), $fVal, [System.Drawing.Brushes]::Black, ($bx + $barW / 2), ($by - 18), $sfC)
            }
        }
        $g.DrawString($dsLabels[$gi], $fTick, [System.Drawing.Brushes]::Black, ($x0 + ($gi + 0.5) * $grpW), ($y0 + $ph + 12), $sfC)
    }

    $g.DrawRectangle($pAxis, $x0, $y0, $pw, $ph)
    $g.DrawString("Набор данных (разрешение BMP-изображения)", $fAxis, [System.Drawing.Brushes]::Black, ($x0 + $pw/2), ($y0 + $ph + 46), $sfC)
    $st = $g.Save(); $g.TranslateTransform($x0 - 85, $y0 + $ph/2); $g.RotateTransform(-90)
    $g.DrawString("Время выполнения, мс", $fAxis, [System.Drawing.Brushes]::Black, 0, 0, $sfC)
    $g.Restore($st)

    $leg = @()
    foreach ($bs in $barSpecs) {
        $leg += @{ Kind="line"; Color=$bs.col; Width=10.0; Dash=[System.Drawing.Drawing2D.DashStyle]::Solid; Marker=$false; Label=$bs.lbl }
    }
    Draw-Legend $g ($x0 + 25) ($y0 + 18) $leg

    $c.Bmp.Save("results/chart_time_ksize_task32.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $fTitle.Dispose(); $fAxis.Dispose(); $fTick.Dispose(); $fVal.Dispose(); $pAxis.Dispose(); $pGrid.Dispose()
    $g.Dispose(); $c.Bmp.Dispose()
}.Invoke()

# 7. Визуальное сравнение исходных и обработанных BMP-изображений (Задание 3.2)
{
    $c = New-Canvas 1680 980
    $g = $c.G
    $fT = New-Object System.Drawing.Font("Times New Roman", 17, [System.Drawing.FontStyle]::Bold)
    $fC = New-Object System.Drawing.Font("Times New Roman", 13)
    $sf = New-Object System.Drawing.StringFormat; $sf.Alignment = [System.Drawing.StringAlignment]::Center
    $pB = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(120,120,120), 1.5)

    $g.DrawString("Примеры исходных (зашумлённых) и отфильтрованных изображений (Задание 3.2)", $fT, [System.Drawing.Brushes]::Black, 840, 14, $sf)

    $panels = @(
        @{ path="images/input_2.bmp";          cap="НД 2 (640x360): исходное (шум 8%)" },
        @{ path="images/output_2_shell_k7.bmp";  cap="НД 2: медианный фильтр, Ksize = 7" },
        @{ path="images/output_2_shell_k11.bmp"; cap="НД 2: медианный фильтр, Ksize = 11" },
        @{ path="images/output_2_shell_k15.bmp"; cap="НД 2: медианный фильтр, Ksize = 15" },
        @{ path="images/input_3.bmp";          cap="НД 3 (960x540): исходное (шум 8%)" },
        @{ path="images/output_3_qsort_k7.bmp"; cap="НД 3: медианный фильтр (qsort), K = 7" },
        @{ path="images/input_4.bmp";          cap="НД 4 (1280x720): исходное (шум 8%)" },
        @{ path="images/output_4_qsort_k7.bmp"; cap="НД 4: медианный фильтр (qsort), K = 7" }
    )

    $cellW = 392; $cellH = 221; $gapX = 24; $startX = 20
    $rowYs = @(66, 354)
    for ($i = 0; $i -lt 8; $i++) {
        $row = [int][Math]::Floor($i / 4)
        $col = $i % 4
        $x = $startX + $col * ($cellW + $gapX)
        $y = $rowYs[$row]
        $img = [System.Drawing.Image]::FromFile((Resolve-Path $panels[$i].path).Path)
        $g.DrawImage($img, $x, $y, $cellW, $cellH)
        $g.DrawRectangle($pB, $x, $y, $cellW, $cellH)
        $img.Dispose()
        $g.DrawString($panels[$i].cap, $fC, [System.Drawing.Brushes]::Black, ($x + $cellW / 2), ($y + $cellH + 8), $sf)
    }

    $c.Bmp.Save("results/visual_median_compare_tmp.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $c.Bmp.Dispose()

    # Обрезаем лишнее пустое поле снизу (итоговая высота 620)
    $srcB = [System.Drawing.Bitmap]::FromFile((Resolve-Path "results/visual_median_compare_tmp.png").Path)
    $crop = $srcB.Clone((New-Object System.Drawing.Rectangle(0, 0, 1680, 620)), $srcB.PixelFormat)
    $crop.Save("results/visual_median_compare.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $crop.Dispose(); $srcB.Dispose()
    Remove-Item -Force "results/visual_median_compare_tmp.png"
    $fT.Dispose(); $fC.Dispose(); $pB.Dispose()
}.Invoke()

# 8. Увеличенный фрагмент (пиксель-в-пиксель 4x) НД 4 — подавление шума и влияние Ksize
{
    $c = New-Canvas 1640 480
    $g = $c.G
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::NearestNeighbor
    $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::Half
    $fT = New-Object System.Drawing.Font("Times New Roman", 17, [System.Drawing.FontStyle]::Bold)
    $fC = New-Object System.Drawing.Font("Times New Roman", 13)
    $sf = New-Object System.Drawing.StringFormat; $sf.Alignment = [System.Drawing.StringAlignment]::Center
    $pB = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(100,100,100), 1.5)

    $g.DrawString("Увеличенный фрагмент (4x) изображения НД 4 (1280x720): влияние размера окна Ksize", $fT, [System.Drawing.Brushes]::Black, 820, 12, $sf)

    $zPanels = @(
        @{ path="images/input_4.bmp";           cap="Исходное (импульсный шум 8%)" },
        @{ path="images/output_4_shell_k7.bmp"; cap="Медианный фильтр, Ksize = 7" },
        @{ path="images/output_4_shell_k11.bmp";cap="Медианный фильтр, Ksize = 11" },
        @{ path="images/output_4_shell_k15.bmp";cap="Медианный фильтр, Ksize = 15" }
    )
    $srcRect = New-Object System.Drawing.Rectangle(540, 260, 96, 92) # центральная граница кольца и квадрата
    for ($i = 0; $i -lt 4; $i++) {
        $x = 24 + $i * 402
        $y = 58
        $dstRect = New-Object System.Drawing.Rectangle($x, $y, 384, 368)
        $img = [System.Drawing.Image]::FromFile((Resolve-Path $zPanels[$i].path).Path)
        $g.DrawImage($img, $dstRect, $srcRect, [System.Drawing.GraphicsUnit]::Pixel)
        $g.DrawRectangle($pB, $dstRect)
        $img.Dispose()
        $g.DrawString($zPanels[$i].cap, $fC, [System.Drawing.Brushes]::Black, ($x + 192), ($y + 378), $sf)
    }
    $c.Bmp.Save("results/visual_median_zoom.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $fT.Dispose(); $fC.Dispose(); $pB.Dispose(); $g.Dispose(); $c.Bmp.Dispose()
}.Invoke()

Write-Host "All charts and visual comparisons generated successfully."
