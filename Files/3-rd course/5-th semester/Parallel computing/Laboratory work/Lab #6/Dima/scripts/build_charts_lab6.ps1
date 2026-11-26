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

function Draw-Axes($g, [float]$x0, [float]$y0, [float]$pw, [float]$ph,
                   [double]$xmin, [double]$xmax, [double]$ymin, [double]$ymax,
                   [int]$nx, [int]$ny, [string]$xLbl, [string]$yLbl, [string]$title,
                   [int]$xDec = 0, [int]$yDec = 2, [double[]]$xTicks = $null) {
    $fT = New-Object System.Drawing.Font("Times New Roman", 18.5, [System.Drawing.FontStyle]::Bold)
    $fA = New-Object System.Drawing.Font("Times New Roman", 14.5, [System.Drawing.FontStyle]::Bold)
    $fK = New-Object System.Drawing.Font("Times New Roman", 12.5)
    $sfC = New-Object System.Drawing.StringFormat; $sfC.Alignment = [System.Drawing.StringAlignment]::Center
    $sfR = New-Object System.Drawing.StringFormat; $sfR.Alignment = [System.Drawing.StringAlignment]::Far; $sfR.LineAlignment = [System.Drawing.StringAlignment]::Center
    $pAx = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(40,40,40), 2.0)
    $pGr = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(215,215,215), 1.0)
    $pGr.DashStyle = [System.Drawing.Drawing2D.DashStyle]::Dash

    $g.DrawString($title, $fT, [System.Drawing.Brushes]::Black, ($x0 + $pw / 2), ($y0 - 46), $sfC)
    for ($i = 0; $i -le $ny; $i++) {
        $yv = $ymin + ($ymax - $ymin) * ($i / [double]$ny)
        $py = $y0 + $ph - [float](($yv - $ymin) / ($ymax - $ymin) * $ph)
        if ($i -gt 0 -and $i -lt $ny) { $g.DrawLine($pGr, $x0, $py, $x0 + $pw, $py) }
        $g.DrawLine($pAx, $x0 - 6, $py, $x0, $py)
        $g.DrawString((Format-Ru $yv $yDec), $fK, [System.Drawing.Brushes]::Black, ($x0 - 10), $py, $sfR)
    }
    $tvs = if ($null -ne $xTicks) { $xTicks } else { 0..$nx | ForEach-Object { $xmin + ($xmax - $xmin) * ($_ / [double]$nx) } }
    foreach ($xv in $tvs) {
        if ($xv -lt $xmin - 1e-9 -or $xv -gt $xmax + 1e-9) { continue }
        $px = $x0 + [float](($xv - $xmin) / ($xmax - $xmin) * $pw)
        if ($xv -gt $xmin + 1e-9 -and $xv -lt $xmax - 1e-9) { $g.DrawLine($pGr, $px, $y0, $px, $y0 + $ph) }
        $g.DrawLine($pAx, $px, $y0 + $ph, $px, $y0 + $ph + 6)
        $g.DrawString((Format-Ru $xv $xDec), $fK, [System.Drawing.Brushes]::Black, $px, ($y0 + $ph + 10), $sfC)
    }
    $g.DrawRectangle($pAx, $x0, $y0, $pw, $ph)
    $g.DrawString($xLbl, $fA, [System.Drawing.Brushes]::Black, ($x0 + $pw / 2), ($y0 + $ph + 38), $sfC)
    $st = $g.Save(); $g.TranslateTransform($x0 - 76, $y0 + $ph / 2); $g.RotateTransform(-90)
    $g.DrawString($yLbl, $fA, [System.Drawing.Brushes]::Black, 0, 0, $sfC)
    $g.Restore($st)
    $fT.Dispose(); $fA.Dispose(); $fK.Dispose(); $pAx.Dispose(); $pGr.Dispose()
}

function Draw-Legend($g, [float]$lx, [float]$ly, [array]$items) {
    $fL = New-Object System.Drawing.Font("Times New Roman", 12)
    $maxW = 0.0
    foreach ($it in $items) { $sz = $g.MeasureString($it.Label, $fL); if ($sz.Width -gt $maxW) { $maxW = $sz.Width } }
    $rh = 24.0; $bw = $maxW + 60.0; $bh = $items.Count * $rh + 12.0
    $bBg = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(244,255,255,255))
    $pBr = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(140,140,140), 1.2)
    $g.FillRectangle($bBg, $lx, $ly, $bw, $bh); $g.DrawRectangle($pBr, $lx, $ly, $bw, $bh)
    for ($i = 0; $i -lt $items.Count; $i++) {
        $it = $items[$i]; $cy = $ly + 6 + $i * $rh + $rh / 2
        if ($it.Kind -eq "dot") { $b = New-Object System.Drawing.SolidBrush($it.Color); $g.FillEllipse($b, $lx + 18, $cy - 5, 10, 10); $b.Dispose() }
        elseif ($it.Kind -eq "cross") { $p = New-Object System.Drawing.Pen($it.Color, 2.5); $g.DrawLine($p, $lx + 18, $cy - 5, $lx + 28, $cy + 5); $g.DrawLine($p, $lx + 18, $cy + 5, $lx + 28, $cy - 5); $p.Dispose() }
        elseif ($it.Kind -eq "box") { $b = New-Object System.Drawing.SolidBrush($it.Color); $g.FillRectangle($b, $lx + 12, $cy - 6, 26, 12); $b.Dispose() }
        else {
            $p = New-Object System.Drawing.Pen($it.Color, $it.Width); $p.DashStyle = $it.Dash
            $g.DrawLine($p, $lx + 10, $cy, $lx + 40, $cy)
            if ($it.Marker) { $b = New-Object System.Drawing.SolidBrush($it.Color); $g.FillEllipse($b, $lx + 21, $cy - 4, 8, 8); $b.Dispose() }
            $p.Dispose()
        }
        $g.DrawString($it.Label, $fL, [System.Drawing.Brushes]::Black, ($lx + 48), ($cy - 10))
    }
    $fL.Dispose(); $bBg.Dispose(); $pBr.Dispose()
}

function Draw-Line($g, $pts, [float]$x0, [float]$y0, [float]$pw, [float]$ph,
                   [double]$xmin, [double]$xmax, [double]$ymin, [double]$ymax,
                   [System.Drawing.Color]$col, [float]$w, $dash, [bool]$mark) {
    $pen = New-Object System.Drawing.Pen($col, $w); $pen.DashStyle = $dash
    $br  = New-Object System.Drawing.SolidBrush($col)
    $sp = @()
    foreach ($pt in $pts) {
        $xv = [double]$pt[0]; $yv = [double]$pt[1]
        $px = $x0 + [float](($xv - $xmin) / ($xmax - $xmin) * $pw)
        $py = $y0 + $ph - [float](([Math]::Min([double]$ymax, [Math]::Max([double]$ymin, $yv)) - $ymin) / ($ymax - $ymin) * $ph)
        $sp += (New-Object System.Drawing.PointF($px, $py))
    }
    if ($sp.Count -ge 2) { $g.DrawLines($pen, [System.Drawing.PointF[]]$sp) }
    if ($mark) {
        foreach ($p in $sp) { $g.FillEllipse($br, $p.X - 5, $p.Y - 5, 10, 10); $g.DrawEllipse([System.Drawing.Pens]::White, $p.X - 5, $p.Y - 5, 10, 10) }
    }
    $pen.Dispose(); $br.Dispose()
}

$df61 = Import-Csv "Lab6/results/lab6_table61.csv"
$df62 = Import-Csv "Lab6/results/lab6_table62.csv"
$raw  = Import-Csv "Lab6/results/lab6_raw_samples.csv"

# 1. Доверительный интервал (Шаг 2: градиенты oneTBB, НД 3 2560x1440, p=4, N=100)
& {
    $vals = $raw | ForEach-Object { [double]$_.time_ms }
    $n = $vals.Count
    $sv = $vals | Sort-Object
    $med = if ($n % 2 -eq 1) { $sv[[int]($n / 2)] } else { 0.5 * ($sv[$n/2 - 1] + $sv[$n/2]) }
    $avg = ($vals | Measure-Object -Average).Average
    $var = 0.0; foreach ($v in $vals) { $var += ($v - $avg) * ($v - $avg) }
    $sd = [Math]::Sqrt($var / ($n - 1))
    $lo = $med - $sd; $hi = $med + $sd
    $kept = New-Object System.Collections.ArrayList
    $rej  = New-Object System.Collections.ArrayList
    for ($i = 0; $i -lt $n; $i++) {
        $idx1 = $i + 1
        if ($vals[$i] -ge $lo -and $vals[$i] -le $hi) { [void]$kept.Add(@($idx1, $vals[$i])) }
        else { [void]$rej.Add(@($idx1, $vals[$i])) }
    }
    $clean = ($kept | ForEach-Object { $_[1] } | Measure-Object -Average).Average

    $ymin = [Math]::Floor(([Math]::Min($lo, ($sv[0])) - 0.5) * 2) / 2.0
    $ymax = [Math]::Ceiling(([Math]::Max($hi, ($sv[$n-1])) + 0.8) * 2) / 2.0
    if ($ymax - $ymin -lt 3.0) { $ymin = [Math]::Max(0.0, $med - 2.0); $ymax = $med + 2.5 }

    $cv = New-Canvas 1440 780; $g = $cv.G
    $x0 = 115; $y0 = 75; $pw = 1270; $ph = 615

    # Зелёная полоса доверительного интервала
    $pyHi = $y0 + $ph - [float](([Math]::Min($ymax, $hi) - $ymin) / ($ymax - $ymin) * $ph)
    $pyLo = $y0 + $ph - [float](([Math]::Max($ymin, $lo) - $ymin) / ($ymax - $ymin) * $ph)
    $bBand = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(40, 44, 160, 44))
    $g.FillRectangle($bBand, $x0, $pyHi, $pw, ($pyLo - $pyHi)); $bBand.Dispose()

    Draw-Axes $g $x0 $y0 $pw $ph 0 102 $ymin $ymax 10 6 "Номер замера" "Время выполнения, мс" `
        "Доверительный интервал: Шаг 2 (градиенты oneTBB, 2560×1440, p = 4, N = 100)" 0 1

    Draw-Line $g @(@(0, $med), @(102, $med)) $x0 $y0 $pw $ph 0 102 $ymin $ymax ([System.Drawing.Color]::FromArgb(30,130,30)) 2.2 ([System.Drawing.Drawing2D.DashStyle]::Solid) $false
    Draw-Line $g @(@(0, $hi), @(102, $hi))   $x0 $y0 $pw $ph 0 102 $ymin $ymax ([System.Drawing.Color]::FromArgb(30,130,30)) 1.8 ([System.Drawing.Drawing2D.DashStyle]::Dash) $false
    Draw-Line $g @(@(0, $lo), @(102, $lo))   $x0 $y0 $pw $ph 0 102 $ymin $ymax ([System.Drawing.Color]::FromArgb(30,130,30)) 1.8 ([System.Drawing.Drawing2D.DashStyle]::Dash) $false
    Draw-Line $g @(@(0, $clean), @(102, $clean)) $x0 $y0 $pw $ph 0 102 $ymin $ymax ([System.Drawing.Color]::FromArgb(230,120,0)) 2.2 ([System.Drawing.Drawing2D.DashStyle]::DashDot) $false

    $bDot = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(31,119,180))
    foreach ($pt in $kept) {
        $px = $x0 + [float](($pt[0]) / 102.0 * $pw)
        $py = $y0 + $ph - [float](($pt[1] - $ymin) / ($ymax - $ymin) * $ph)
        $g.FillEllipse($bDot, $px - 4.5, $py - 4.5, 9, 9)
    }
    $bDot.Dispose()
    $penCross = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(214,39,40), 2.4)
    foreach ($pt in $rej) {
        $px = $x0 + [float](([double]$pt[0]) / 102.0 * $pw)
        $py = $y0 + $ph - [float](([Math]::Min([double]$ymax, [Math]::Max([double]$ymin, [double]$pt[1])) - $ymin) / ($ymax - $ymin) * $ph)
        $g.DrawLine($penCross, [float]($px - 5), [float]($py - 5), [float]($px + 5), [float]($py + 5))
        $g.DrawLine($penCross, [float]($px - 5), [float]($py + 5), [float]($px + 5), [float]($py - 5))
    }
    $penCross.Dispose()

    Draw-Legend $g ($x0 + $pw - 425) ($y0 + 14) @(
        @{ Kind="dot";   Color=[System.Drawing.Color]::FromArgb(31,119,180); Label="Принятые замеры ($($kept.Count) из $n)" },
        @{ Kind="cross"; Color=[System.Drawing.Color]::FromArgb(214,39,40);  Label="Отброшенные выбросы ($($rej.Count))" },
        @{ Kind="line";  Color=[System.Drawing.Color]::FromArgb(30,130,30);  Width=2.2; Dash=[System.Drawing.Drawing2D.DashStyle]::Solid;   Marker=$false; Label="Медиана med = $(Format-Ru $med 4) мс" },
        @{ Kind="line";  Color=[System.Drawing.Color]::FromArgb(30,130,30);  Width=1.8; Dash=[System.Drawing.Drawing2D.DashStyle]::Dash;    Marker=$false; Label="Границы med ± SD (SD = $(Format-Ru $sd 4) мс)" },
        @{ Kind="line";  Color=[System.Drawing.Color]::FromArgb(230,120,0);  Width=2.2; Dash=[System.Drawing.Drawing2D.DashStyle]::DashDot; Marker=$false; Label="Итог AvgTrustedIntervalMed = $(Format-Ru $clean 4) мс" }
    )
    $cv.Bmp.Save("Lab6/results/chart_ci_lab6.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $cv.Bmp.Dispose()
}

# 2. Ускорение Sp(p) полных реализаций детектора Харриса (НД 1 1280x720 и НД 3 2560x1440)
& {
    $cv = New-Canvas 1560 740; $g = $cv.G
    $panels = @(
        @{ Ds="1"; X0=105; Title="Полный конвейер Харриса (НД 1: 1280×720)" },
        @{ Ds="3"; X0=895; Title="Полный конвейер Харриса (НД 3: 2560×1440)" }
    )
    $seriesDefs = @(
        @{ Id="OpenMP_STL";    Col=[System.Drawing.Color]::FromArgb(31,119,180);  Lbl="OpenMP (STL vector)" },
        @{ Id="OpenMP_TBBVec"; Col=[System.Drawing.Color]::FromArgb(100,175,230); Lbl="OpenMP (oneTBB conc_vec)" },
        @{ Id="oneTBB_STL";    Col=[System.Drawing.Color]::FromArgb(44,160,44);   Lbl="Intel oneTBB (STL vector)" },
        @{ Id="oneTBB_TBBVec"; Col=[System.Drawing.Color]::FromArgb(145,210,120); Lbl="Intel oneTBB (conc_vec)" },
        @{ Id="Hybrid_STL";    Col=[System.Drawing.Color]::FromArgb(214,39,40);   Lbl="Гибрид OpenMP+oneTBB (STL)" }
    )
    foreach ($pn in $panels) {
        $dsId = $pn.Ds
        Draw-Axes $g $pn.X0 80 610 565 1 4 0 4.5 3 9 "Число потоков, p" "Ускорение, Sp(n)" $pn.Title 0 1 @(1,2,3,4)
        Draw-Line $g @(@(1,1),@(2,2),@(3,3),@(4,4)) $pn.X0 80 610 565 1 4 0 4.5 ([System.Drawing.Color]::Gray) 1.8 ([System.Drawing.Drawing2D.DashStyle]::Dash) $false
        $amdPts = @(@(1,1.0), @(2,(1.0/(0.15+0.85/2))), @(3,(1.0/(0.15+0.85/3))), @(4,(1.0/(0.15+0.85/4))))
        Draw-Line $g $amdPts $pn.X0 80 610 565 1 4 0 4.5 ([System.Drawing.Color]::FromArgb(148,103,189)) 1.8 ([System.Drawing.Drawing2D.DashStyle]::Dot) $false

        foreach ($sd in $seriesDefs) {
            $idCur = $sd.Id
            $al = New-Object System.Collections.ArrayList
            [void]$al.Add(@(1.0, 1.0))
            foreach ($p in 2,3,4) {
                $row = $df62 | Where-Object { $_.ds_id -eq $dsId -and $_.impl_id -eq $idCur -and [int]$_.threads -eq $p } | Select-Object -First 1
                if ($row) { [void]$al.Add(@([double]$p, [double]$row.speedup)) }
            }
            Draw-Line $g $al $pn.X0 80 610 565 1 4 0 4.5 $sd.Col 2.8 ([System.Drawing.Drawing2D.DashStyle]::Solid) $true
        }

        $legItems = @()
        foreach ($sd in $seriesDefs) {
            $legItems += @{ Kind="line"; Color=$sd.Col; Width=2.6; Dash=[System.Drawing.Drawing2D.DashStyle]::Solid; Marker=$true; Label=$sd.Lbl }
        }
        $legItems += @{ Kind="line"; Color=[System.Drawing.Color]::Gray; Width=1.8; Dash=[System.Drawing.Drawing2D.DashStyle]::Dash; Marker=$false; Label="Идеальное ускорение (f = 0)" }
        $legItems += @{ Kind="line"; Color=[System.Drawing.Color]::FromArgb(148,103,189); Width=1.8; Dash=[System.Drawing.Drawing2D.DashStyle]::Dot; Marker=$false; Label="Закон Амдала (f = 0,15)" }
        Draw-Legend $g ($pn.X0 + 14) 94 $legItems
    }
    $cv.Bmp.Save("Lab6/results/chart_speedup_lab6.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $cv.Bmp.Dispose()
}

# 3. Столбчатая диаграмма: распределение времени по шагам (НД 3: 2560x1440) + STL vs concurrent_vector на Шаге 4
& {
    $cv = New-Canvas 1560 760; $g = $cv.G
    $fT = New-Object System.Drawing.Font("Times New Roman", 18, [System.Drawing.FontStyle]::Bold)
    $fA = New-Object System.Drawing.Font("Times New Roman", 13.5, [System.Drawing.FontStyle]::Bold)
    $fK = New-Object System.Drawing.Font("Times New Roman", 11.5)
    $fV = New-Object System.Drawing.Font("Times New Roman", 10.5, [System.Drawing.FontStyle]::Bold)
    $sfC = New-Object System.Drawing.StringFormat; $sfC.Alignment = [System.Drawing.StringAlignment]::Center
    $sfR = New-Object System.Drawing.StringFormat; $sfR.Alignment = [System.Drawing.StringAlignment]::Far; $sfR.LineAlignment = [System.Drawing.StringAlignment]::Center
    $pAx = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(40,40,40), 2.0)
    $pGr = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(220,220,220), 1.0); $pGr.DashStyle = [System.Drawing.Drawing2D.DashStyle]::Dash

    # Левая панель: вклад шагов (НД 3, p=1 vs p=4 OpenMP vs p=4 oneTBB)
    $x0 = 95; $y0 = 80; $pw = 670; $ph = 570
    $steps = @(
        @{ Id="Step1_GrayGauss"; Lbl="Шаг 1`n(Гаусс)" },
        @{ Id="Step2_Gradients"; Lbl="Шаг 2`n(Град.)" },
        @{ Id="Step3_ResponseR"; Lbl="Шаг 3`n(Откл. R)" },
        @{ Id="Step4_1_Collect"; Lbl="Шаг 4.1`n(Сбор)" },
        @{ Id="Step4_2_Sort";    Lbl="Шаг 4.2`n(Сорт.)" },
        @{ Id="Step4_3_NMS";     Lbl="Шаг 4.3`n(Отсеч.)" },
        @{ Id="Step5_Visualize"; Lbl="Шаг 5`n(Визуал.)" }
    )
    $maxY1 = 30.0
    foreach ($st in $steps) {
        $sid = $st.Id
        $rSeq = $df61 | Where-Object { $_.ds_id -eq "3" -and $_.step_id -eq $sid -and $_.backend -eq "Seq" } | Select-Object -First 1
        if ([double]$rSeq.trusted_avg_med_ms -gt $maxY1) { $maxY1 = [Math]::Ceiling([double]$rSeq.trusted_avg_med_ms / 5.0) * 5.0 + 5.0 }
    }
    $g.DrawString("Время по шагам (НД 3: 2560×1440)", $fT, [System.Drawing.Brushes]::Black, ($x0 + $pw / 2), ($y0 - 46), $sfC)
    for ($i = 0; $i -le 6; $i++) {
        $yv = $maxY1 * $i / 6.0
        $py = $y0 + $ph - [float]($yv / $maxY1 * $ph)
        if ($i -gt 0 -and $i -lt 6) { $g.DrawLine($pGr, $x0, $py, $x0 + $pw, $py) }
        $g.DrawString((Format-Ru $yv 0), $fK, [System.Drawing.Brushes]::Black, ($x0 - 8), $py, $sfR)
    }
    $g.DrawRectangle($pAx, $x0, $y0, $pw, $ph)
    $stRot = $g.Save(); $g.TranslateTransform($x0 - 58, $y0 + $ph / 2); $g.RotateTransform(-90)
    $g.DrawString("Время выполнения, мс", $fA, [System.Drawing.Brushes]::Black, 0, 0, $sfC); $g.Restore($stRot)

    $cols3 = @([System.Drawing.Color]::FromArgb(120,120,120), [System.Drawing.Color]::FromArgb(31,119,180), [System.Drawing.Color]::FromArgb(44,160,44))
    $gw = $pw / [double]$steps.Count
    for ($gi = 0; $gi -lt $steps.Count; $gi++) {
        $sid = $steps[$gi].Id
        $cont = if ($sid -like "Step4*") { "STL" } else { "None" }
        $rS = $df61 | Where-Object { $_.ds_id -eq "3" -and $_.step_id -eq $sid -and $_.backend -eq "Seq" } | Select-Object -First 1
        $rO = $df61 | Where-Object { $_.ds_id -eq "3" -and $_.step_id -eq $sid -and $_.backend -eq "OpenMP" -and $_.container -eq $cont -and $_.threads -eq "4" } | Select-Object -First 1
        $rT = $df61 | Where-Object { $_.ds_id -eq "3" -and $_.step_id -eq $sid -and $_.backend -eq "oneTBB" -and $_.container -eq $cont -and $_.threads -eq "4" } | Select-Object -First 1
        $vals3 = @([double]$rS.trusted_avg_med_ms, [double]$rO.trusted_avg_med_ms, [double]$rT.trusted_avg_med_ms)
        $gx = $x0 + $gi * $gw + 10
        $bw = ($gw - 24) / 3.0
        for ($bi = 0; $bi -lt 3; $bi++) {
            $bh = [float]([Math]::Max(2.0, ($vals3[$bi] / $maxY1) * $ph))
            $bx = [float]($gx + $bi * $bw)
            $by = [float]($y0 + $ph - $bh)
            $br = New-Object System.Drawing.SolidBrush($cols3[$bi])
            $g.FillRectangle($br, $bx, $by, [float]($bw - 3), $bh)
            $g.DrawRectangle([System.Drawing.Pens]::Black, $bx, $by, [float]($bw - 3), $bh)
            $br.Dispose()
            if ($vals3[$bi] -ge 0.8) {
                $g.DrawString((Format-Ru $vals3[$bi] 1), $fV, [System.Drawing.Brushes]::Black, ($bx + ($bw-3)/2), ($by - 18), $sfC)
            }
        }
        $g.DrawString($steps[$gi].Lbl, $fK, [System.Drawing.Brushes]::Black, [float]($x0 + ($gi + 0.5) * $gw), ($y0 + $ph + 8), $sfC)
    }
    Draw-Legend $g ($x0 + $pw - 260) ($y0 + 14) @(
        @{ Kind="box"; Color=$cols3[0]; Label="Последоват. (p = 1)" },
        @{ Kind="box"; Color=$cols3[1]; Label="OpenMP (p = 4)" },
        @{ Kind="box"; Color=$cols3[2]; Label="Intel oneTBB (p = 4)" }
    )

    # Правая панель: Шаг 4 (STL std::vector vs oneTBB concurrent_vector) на НД 3 (2560x1440, p = 4)
    $x1 = 870; $pw2 = 635
    $s4List = @(
        @{ Id="Step4_1_Collect"; Lbl="Шаг 4.1`n(порог и сбор)" },
        @{ Id="Step4_2_Sort";    Lbl="Шаг 4.2`n(сортировка)" },
        @{ Id="Step4_3_NMS";     Lbl="Шаг 4.3`n(отсечение)" }
    )
    $maxY2 = 2.0
    foreach ($st in $s4List) {
        $sid = $st.Id
        $rowsS4 = $df61 | Where-Object { $_.ds_id -eq "3" -and $_.step_id -eq $sid -and $_.threads -eq "4" }
        foreach ($rr in $rowsS4) { if ([double]$rr.trusted_avg_med_ms -gt $maxY2) { $maxY2 = [double]$rr.trusted_avg_med_ms } }
    }
    $maxY2 = [Math]::Ceiling($maxY2 * 4.0) / 4.0 + 0.25
    $g.DrawString("Шаг 4 (НД 3, p = 4): STL vs concurrent_vector", $fT, [System.Drawing.Brushes]::Black, ($x1 + $pw2 / 2), ($y0 - 46), $sfC)
    for ($i = 0; $i -le 5; $i++) {
        $yv = $maxY2 * $i / 5.0
        $py = $y0 + $ph - [float]($yv / $maxY2 * $ph)
        if ($i -gt 0 -and $i -lt 5) { $g.DrawLine($pGr, $x1, $py, $x1 + $pw2, $py) }
        $g.DrawString((Format-Ru $yv 2), $fK, [System.Drawing.Brushes]::Black, ($x1 - 8), $py, $sfR)
    }
    $g.DrawRectangle($pAx, $x1, $y0, $pw2, $ph)
    $stRot2 = $g.Save(); $g.TranslateTransform($x1 - 62, $y0 + $ph / 2); $g.RotateTransform(-90)
    $g.DrawString("Время выполнения, мс", $fA, [System.Drawing.Brushes]::Black, 0, 0, $sfC); $g.Restore($stRot2)

    $cols4 = @(
        [System.Drawing.Color]::FromArgb(31,119,180),
        [System.Drawing.Color]::FromArgb(140,195,235),
        [System.Drawing.Color]::FromArgb(44,160,44),
        [System.Drawing.Color]::FromArgb(255,140,30)
    )
    $gw2 = $pw2 / 3.0
    for ($gi = 0; $gi -lt 3; $gi++) {
        $sid = $s4List[$gi].Id
        $vOS = [double]($df61 | Where-Object { $_.ds_id -eq "3" -and $_.step_id -eq $sid -and $_.backend -eq "OpenMP" -and $_.container -eq "STL"    -and $_.threads -eq "4" } | Select-Object -First 1).trusted_avg_med_ms
        $vOT = [double]($df61 | Where-Object { $_.ds_id -eq "3" -and $_.step_id -eq $sid -and $_.backend -eq "OpenMP" -and $_.container -eq "TBBVec" -and $_.threads -eq "4" } | Select-Object -First 1).trusted_avg_med_ms
        $vTS = [double]($df61 | Where-Object { $_.ds_id -eq "3" -and $_.step_id -eq $sid -and $_.backend -eq "oneTBB" -and $_.container -eq "STL"    -and $_.threads -eq "4" } | Select-Object -First 1).trusted_avg_med_ms
        $vTT = [double]($df61 | Where-Object { $_.ds_id -eq "3" -and $_.step_id -eq $sid -and $_.backend -eq "oneTBB" -and $_.container -eq "TBBVec" -and $_.threads -eq "4" } | Select-Object -First 1).trusted_avg_med_ms
        $vals4 = @($vOS, $vOT, $vTS, $vTT)
        $gx = $x1 + $gi * $gw2 + 16
        $bw = ($gw2 - 36) / 4.0
        for ($bi = 0; $bi -lt 4; $bi++) {
            $bh = [float]([Math]::Max(3.0, ($vals4[$bi] / $maxY2) * $ph))
            $bx = [float]($gx + $bi * $bw)
            $by = [float]($y0 + $ph - $bh)
            $br = New-Object System.Drawing.SolidBrush($cols4[$bi])
            $g.FillRectangle($br, $bx, $by, [float]($bw - 4), $bh)
            $g.DrawRectangle([System.Drawing.Pens]::Black, $bx, $by, [float]($bw - 4), $bh)
            $br.Dispose()
            $g.DrawString((Format-Ru $vals4[$bi] 2), $fV, [System.Drawing.Brushes]::Black, ($bx + ($bw-4)/2), ($by - 18), $sfC)
        }
        $g.DrawString($s4List[$gi].Lbl, $fK, [System.Drawing.Brushes]::Black, [float]($x1 + ($gi + 0.5) * $gw2), ($y0 + $ph + 8), $sfC)
    }
    Draw-Legend $g ($x1 + $pw2 - 285) ($y0 + 14) @(
        @{ Kind="box"; Color=$cols4[0]; Label="OpenMP (STL vector)" },
        @{ Kind="box"; Color=$cols4[1]; Label="OpenMP (concurrent_vector)" },
        @{ Kind="box"; Color=$cols4[2]; Label="oneTBB (STL vector)" },
        @{ Kind="box"; Color=$cols4[3]; Label="oneTBB (concurrent_vector)" }
    )
    $cv.Bmp.Save("Lab6/results/chart_steps_stl_vs_tbb_lab6.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $cv.Bmp.Dispose()
}

# 4. Визуальное сравнение: исходные и обработанные изображения (НД 1, НД 2, НД 3 + увеличенный фрагмент с углами)
& {
    $cv = New-Canvas 1680 980; $g = $cv.G
    $fT = New-Object System.Drawing.Font("Times New Roman", 18, [System.Drawing.FontStyle]::Bold)
    $fC = New-Object System.Drawing.Font("Times New Roman", 13)
    $sfC = New-Object System.Drawing.StringFormat; $sfC.Alignment = [System.Drawing.StringAlignment]::Center
    $g.DrawString("Примеры исходных и обработанных изображений детектором Харриса (красные маркеры особых точек)", $fT, [System.Drawing.Brushes]::Black, 840, 14, $sfC)

    $items = @(
        @{ Path="Lab6/images/input_1.bmp";            Cap="НД 1 (1280×720): исходное";                         Crop=$null },
        @{ Path="Lab6/images/output_1_tbb_tbbvec.bmp";Cap="НД 1: особые точки Харриса (124 точки)";            Crop=$null },
        @{ Path="Lab6/images/output_1_tbb_tbbvec.bmp";Cap="НД 1: увеличенный фрагмент 3× (углы шахматки)";     Crop=(New-Object System.Drawing.Rectangle(120, 180, 360, 202)) },
        @{ Path="Lab6/images/input_2.bmp";            Cap="НД 2 (1920×1080): исходное";                        Crop=$null },
        @{ Path="Lab6/images/output_2_tbb_tbbvec.bmp";Cap="НД 2: особые точки Харриса (495 точек)";            Crop=$null },
        @{ Path="Lab6/images/output_3_tbb_tbbvec.bmp";Cap="НД 3 (2560×1440): увеличенный фрагмент (углы окон)";Crop=(New-Object System.Drawing.Rectangle(1120, 140, 640, 360)) }
    )
    # Актуализируем подписи с реальным числом точек из CSV
    $p1 = ($df62 | Where-Object { $_.ds_id -eq "1" -and $_.impl_id -eq "oneTBB_TBBVec" -and $_.threads -eq "4" } | Select-Object -First 1).pts_final
    $p2 = ($df62 | Where-Object { $_.ds_id -eq "2" -and $_.impl_id -eq "oneTBB_TBBVec" -and $_.threads -eq "4" } | Select-Object -First 1).pts_final
    $p3 = ($df62 | Where-Object { $_.ds_id -eq "3" -and $_.impl_id -eq "oneTBB_TBBVec" -and $_.threads -eq "4" } | Select-Object -First 1).pts_final
    $items[1].Cap = "НД 1: особые точки Харриса ($p1 точки)"
    $items[4].Cap = "НД 2: особые точки Харриса ($p2 точек)"
    $items[5].Cap = "НД 3 (2560×1440, $p3 точек): увеличенный фрагмент"

    $tw = 520; $th = 292; $gapX = 28; $gapY = 85; $sx = 32; $sy = 68
    for ($i = 0; $i -lt 6; $i++) {
        $col = $i % 3; $row = [int][Math]::Floor($i / 3)
        $dx = $sx + $col * ($tw + $gapX)
        $dy = $sy + $row * ($th + $gapY)
        $src = [System.Drawing.Bitmap]::FromFile((Resolve-Path $items[$i].Path).Path)
        if ($null -ne $items[$i].Crop) {
            $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::NearestNeighbor
            $g.DrawImage($src, (New-Object System.Drawing.Rectangle($dx, $dy, $tw, $th)), $items[$i].Crop, [System.Drawing.GraphicsUnit]::Pixel)
            $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
        } else {
            $g.DrawImage($src, $dx, $dy, $tw, $th)
        }
        $src.Dispose()
        $g.DrawRectangle([System.Drawing.Pens]::Gray, $dx, $dy, $tw, $th)
        $g.DrawString($items[$i].Cap, $fC, [System.Drawing.Brushes]::Black, ($dx + $tw / 2), ($dy + $th + 10), $sfC)
    }
    $cv.Bmp.Save("Lab6/results/visual_lab6_harris.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $cv.Bmp.Dispose()
}

Write-Host "All Lab 6 charts and visual comparisons generated successfully."
