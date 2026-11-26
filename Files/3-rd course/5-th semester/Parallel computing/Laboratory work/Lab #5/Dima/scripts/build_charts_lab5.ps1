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
    $fT = New-Object System.Drawing.Font("Times New Roman", 19, [System.Drawing.FontStyle]::Bold)
    $fA = New-Object System.Drawing.Font("Times New Roman", 14.5, [System.Drawing.FontStyle]::Bold)
    $fK = New-Object System.Drawing.Font("Times New Roman", 12.5)
    $sfC = New-Object System.Drawing.StringFormat; $sfC.Alignment = [System.Drawing.StringAlignment]::Center
    $sfR = New-Object System.Drawing.StringFormat; $sfR.Alignment = [System.Drawing.StringAlignment]::Far; $sfR.LineAlignment = [System.Drawing.StringAlignment]::Center
    $pAx = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(40,40,40), 2.0)
    $pGr = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(215,215,215), 1.0)
    $pGr.DashStyle = [System.Drawing.Drawing2D.DashStyle]::Dash

    $g.DrawString($title, $fT, [System.Drawing.Brushes]::Black, ($x0 + $pw / 2), ($y0 - 48), $sfC)
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
    $g.DrawString($xLbl, $fA, [System.Drawing.Brushes]::Black, ($x0 + $pw / 2), ($y0 + $ph + 40), $sfC)
    $st = $g.Save(); $g.TranslateTransform($x0 - 78, $y0 + $ph / 2); $g.RotateTransform(-90)
    $g.DrawString($yLbl, $fA, [System.Drawing.Brushes]::Black, 0, 0, $sfC)
    $g.Restore($st)
    $fT.Dispose(); $fA.Dispose(); $fK.Dispose(); $pAx.Dispose(); $pGr.Dispose()
}

function Draw-Legend($g, [float]$lx, [float]$ly, [array]$items) {
    $fL = New-Object System.Drawing.Font("Times New Roman", 12)
    $maxW = 0.0
    foreach ($it in $items) { $sz = $g.MeasureString($it.Label, $fL); if ($sz.Width -gt $maxW) { $maxW = $sz.Width } }
    $rh = 25.0; $bw = $maxW + 60.0; $bh = $items.Count * $rh + 12.0
    $bBg = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(242,255,255,255))
    $pBr = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(140,140,140), 1.2)
    $g.FillRectangle($bBg, $lx, $ly, $bw, $bh); $g.DrawRectangle($pBr, $lx, $ly, $bw, $bh)
    for ($i = 0; $i -lt $items.Count; $i++) {
        $it = $items[$i]; $cy = $ly + 6 + $i * $rh + $rh / 2
        if ($it.Kind -eq "dot") { $b = New-Object System.Drawing.SolidBrush($it.Color); $g.FillEllipse($b, $lx + 18, $cy - 5, 10, 10); $b.Dispose() }
        elseif ($it.Kind -eq "cross") { $p = New-Object System.Drawing.Pen($it.Color, 2.5); $g.DrawLine($p, $lx + 18, $cy - 5, $lx + 28, $cy + 5); $g.DrawLine($p, $lx + 18, $cy + 5, $lx + 28, $cy - 5); $p.Dispose() }
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

$df  = Import-Csv "Lab5/results/lab5_summary.csv"
$raw = Import-Csv "Lab5/results/lab5_raw_samples.csv"

# 1. Точечная диаграмма доверительного интервала (Гаусс быстрый 2x1D oneTBB, 2560x1440, K=13, p=4, N=100)
{
    $rows = $raw | Where-Object { $_.filter_type -eq "Gauss" }
    $c = New-Canvas 1600 900; $g = $c.G
    $x0 = 130.0; $y0 = 85.0; $pw = 1410.0; $ph = 710.0
    $times = $rows | ForEach-Object { [double]$_.time_ms }
    $med = [double]$rows[0].median_ms; $sd = [double]$rows[0].sd_ms
    $kept = $rows | Where-Object { [int]$_.in_interval -eq 1 } | ForEach-Object { [double]$_.time_ms }
    $trusted = ($kept | Measure-Object -Average).Average
    $n = $rows.Count
    $ymin = [Math]::Max(0.0, [Math]::Min(($times | Measure-Object -Minimum).Minimum, $med - $sd) - $sd * 0.7)
    $ymax = [Math]::Max(($times | Measure-Object -Maximum).Maximum, $med + $sd) + $sd * 1.5

    Draw-Axes $g $x0 $y0 $pw $ph 0 ($n + 2) $ymin $ymax 10 6 "Номер замера" "Время выполнения, мс" `
        "Доверительный интервал: Гаусс быстрый 2×1D oneTBB (2560×1440, K = 13, p = 4, N = 100)" 0 1

    $yL = $y0 + $ph - [float](([Math]::Max($ymin, $med - $sd) - $ymin) / ($ymax - $ymin) * $ph)
    $yH = $y0 + $ph - [float](([Math]::Min($ymax, $med + $sd) - $ymin) / ($ymax - $ymin) * $ph)
    $bB = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(30, 34, 139, 34))
    $g.FillRectangle($bB, $x0, $yH, $pw, ($yL - $yH)); $bB.Dispose()

    $pM = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(20,130,50), 2.4)
    $pD = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(20,130,50), 1.8); $pD.DashStyle = [System.Drawing.Drawing2D.DashStyle]::Dash
    $pA = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(215,110,0), 2.4); $pA.DashStyle = [System.Drawing.Drawing2D.DashStyle]::DashDot
    $pyM = $y0 + $ph - [float](($med - $ymin) / ($ymax - $ymin) * $ph)
    $pyA = $y0 + $ph - [float](($trusted - $ymin) / ($ymax - $ymin) * $ph)
    $g.DrawLine($pM, $x0, $pyM, $x0 + $pw, $pyM); $g.DrawLine($pD, $x0, $yH, $x0 + $pw, $yH)
    $g.DrawLine($pD, $x0, $yL, $x0 + $pw, $yL); $g.DrawLine($pA, $x0, $pyA, $x0 + $pw, $pyA)

    $bIn = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(220,31,119,180))
    $pOu = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(220,214,39,40), 2.6)
    foreach ($r in $rows) {
        $px = $x0 + [float]([double]$r.run_idx / ($n + 2.0) * $pw)
        $py = $y0 + $ph - [float](([double]$r.time_ms - $ymin) / ($ymax - $ymin) * $ph)
        if ([int]$r.in_interval -eq 1) { $g.FillEllipse($bIn, $px - 5, $py - 5, 10, 10) }
        else { $g.DrawLine($pOu, $px - 6, $py - 6, $px + 6, $py + 6); $g.DrawLine($pOu, $px - 6, $py + 6, $px + 6, $py - 6) }
    }
    Draw-Legend $g ($x0 + $pw - 470) ($y0 + 14) @(
        @{ Kind="dot";   Color=[System.Drawing.Color]::FromArgb(31,119,180); Label="Принятые замеры ($($kept.Count) из $n)" },
        @{ Kind="cross"; Color=[System.Drawing.Color]::FromArgb(214,39,40);  Label="Отброшенные выбросы ($($n - $kept.Count))" },
        @{ Kind="line";  Color=[System.Drawing.Color]::FromArgb(20,130,50);  Width=2.4; Dash=[System.Drawing.Drawing2D.DashStyle]::Solid;   Marker=$false; Label="Медиана med = $(Format-Ru $med 4) мс" },
        @{ Kind="line";  Color=[System.Drawing.Color]::FromArgb(20,130,50);  Width=1.8; Dash=[System.Drawing.Drawing2D.DashStyle]::Dash;    Marker=$false; Label="Границы med ± SD (SD = $(Format-Ru $sd 4) мс)" },
        @{ Kind="line";  Color=[System.Drawing.Color]::FromArgb(215,110,0);  Width=2.4; Dash=[System.Drawing.Drawing2D.DashStyle]::DashDot; Marker=$false; Label="Итог AvgTrustedIntervalMed = $(Format-Ru $trusted 4) мс" }
    )
    $pM.Dispose(); $pD.Dispose(); $pA.Dispose(); $bIn.Dispose(); $pOu.Dispose()
    $c.Bmp.Save("Lab5/results/chart_ci_lab5.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $c.Bmp.Dispose()
}.Invoke()

# 2. Двухпанельный график ускорения Sp(p): слева Гаусс, справа Среднеарифметический (НД 4: 2560x1440, K=13)
{
    $c = New-Canvas 1680 800; $g = $c.G
    $pw = 675.0; $ph = 580.0; $y0 = 90.0; $xL = 110.0; $xR = 955.0

    Draw-Axes $g $xL $y0 $pw $ph 1 4 0 4.5 3 9 "Число потоков, p" "Ускорение, Sp(n)" "Фильтр Гаусса (2560×1440, K = 13)" 0 1 @(1,2,3,4)
    Draw-Axes $g $xR $y0 $pw $ph 1 4 0 4.5 3 9 "Число потоков, p" "Ускорение, Sp(n)" "Среднеарифметический фильтр (2560×1440, K = 13)" 0 1 @(1,2,3,4)

    $idPts = New-Object System.Collections.ArrayList; foreach ($p in 1..4) { [void]$idPts.Add(@([double]$p, [double]$p)) }
    $amPts = New-Object System.Collections.ArrayList; foreach ($p in 1..4) { [void]$amPts.Add(@([double]$p, (1.0 / (0.10 + 0.90 / $p)))) }
    Draw-Line $g $idPts $xL $y0 $pw $ph 1 4 0 4.5 ([System.Drawing.Color]::Gray) 2.0 ([System.Drawing.Drawing2D.DashStyle]::Dash) $false
    Draw-Line $g $amPts $xL $y0 $pw $ph 1 4 0 4.5 ([System.Drawing.Color]::FromArgb(140,110,160)) 1.8 ([System.Drawing.Drawing2D.DashStyle]::Dot) $false
    Draw-Line $g $idPts $xR $y0 $pw $ph 1 4 0 4.5 ([System.Drawing.Color]::Gray) 2.0 ([System.Drawing.Drawing2D.DashStyle]::Dash) $false
    Draw-Line $g $amPts $xR $y0 $pw $ph 1 4 0 4.5 ([System.Drawing.Color]::FromArgb(140,110,160)) 1.8 ([System.Drawing.Drawing2D.DashStyle]::Dot) $false

    $specs = @(
        @{ var="Classic"; bk="OpenMP"; col=[System.Drawing.Color]::FromArgb(31,119,180);  lbl="Классический 2D (OpenMP)" },
        @{ var="Classic"; bk="oneTBB"; col=[System.Drawing.Color]::FromArgb(214,39,40);   lbl="Классический 2D (oneTBB)" },
        @{ var="Fast";    bk="OpenMP"; col=[System.Drawing.Color]::FromArgb(44,160,44);   lbl="Быстрый (OpenMP)" },
        @{ var="Fast";    bk="oneTBB"; col=[System.Drawing.Color]::FromArgb(255,127,14);  lbl="Быстрый (oneTBB)" }
    )
    foreach ($side in @(@{ ft="Gauss"; x=$xL }, @{ ft="Mean"; x=$xR })) {
        $leg = @()
        foreach ($s in $specs) {
            $pts = New-Object System.Collections.ArrayList
            [void]$pts.Add(@(1.0, 1.0))
            foreach ($p in @(2,3,4)) {
                $row = $df | Where-Object { [int]$_.img_id -eq 4 -and $_.filter_type -eq $side.ft -and $_.variant -eq $s.var -and $_.backend -eq $s.bk -and [int]$_.ksize -eq 13 -and [int]$_.threads -eq $p } | Select-Object -First 1
                if ($row) { [void]$pts.Add(@([double]$p, [double]$row.speedup_own)) }
            }
            Draw-Line $g $pts $side.x $y0 $pw $ph 1 4 0 4.5 $s.col 2.8 ([System.Drawing.Drawing2D.DashStyle]::Solid) $true
            $leg += @{ Kind="line"; Color=$s.col; Width=2.8; Dash=[System.Drawing.Drawing2D.DashStyle]::Solid; Marker=$true; Label=$s.lbl }
        }
        $leg += @{ Kind="line"; Color=[System.Drawing.Color]::Gray; Width=2.0; Dash=[System.Drawing.Drawing2D.DashStyle]::Dash; Marker=$false; Label="Идеальное ускорение (f=0)" }
        $leg += @{ Kind="line"; Color=[System.Drawing.Color]::FromArgb(140,110,160); Width=1.8; Dash=[System.Drawing.Drawing2D.DashStyle]::Dot; Marker=$false; Label="Закон Амдала (f = 0,10)" }
        Draw-Legend $g ($side.x + 16) ($y0 + 14) $leg
    }
    $c.Bmp.Save("Lab5/results/chart_speedup_lab5.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $c.Bmp.Dispose()
}.Invoke()

# 3. Столбчатая диаграмма: Классический 2D vs Быстрый (2x1D / Рекурсия) на НД 4 (2560x1440, p = 4)
{
    $c = New-Canvas 1600 880; $g = $c.G
    $x0 = 135.0; $y0 = 85.0; $pw = 1405.0; $ph = 670.0
    $fT = New-Object System.Drawing.Font("Times New Roman", 19, [System.Drawing.FontStyle]::Bold)
    $fA = New-Object System.Drawing.Font("Times New Roman", 14.5, [System.Drawing.FontStyle]::Bold)
    $fK = New-Object System.Drawing.Font("Times New Roman", 12)
    $fV = New-Object System.Drawing.Font("Times New Roman", 10.5, [System.Drawing.FontStyle]::Bold)
    $sfC = New-Object System.Drawing.StringFormat; $sfC.Alignment = [System.Drawing.StringAlignment]::Center
    $sfR = New-Object System.Drawing.StringFormat; $sfR.Alignment = [System.Drawing.StringAlignment]::Far; $sfR.LineAlignment = [System.Drawing.StringAlignment]::Center
    $pAx = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(40,40,40), 2.0)
    $pGr = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(215,215,215), 1.0); $pGr.DashStyle = [System.Drawing.Drawing2D.DashStyle]::Dash

    $g.DrawString("Время фильтрации НД 4 (2560×1440, p = 4): классический 2D против быстрого алгоритма", $fT, [System.Drawing.Brushes]::Black, ($x0 + $pw/2), ($y0 - 48), $sfC)

    $bSpecs = @(
        @{ ft="Gauss"; var="Classic"; bk="OpenMP"; col=[System.Drawing.Color]::FromArgb(31,119,180);  lbl="Гаусс 2D OpenMP" },
        @{ ft="Gauss"; var="Classic"; bk="oneTBB"; col=[System.Drawing.Color]::FromArgb(100,175,230); lbl="Гаусс 2D oneTBB" },
        @{ ft="Gauss"; var="Fast";    bk="OpenMP"; col=[System.Drawing.Color]::FromArgb(44,160,44);   lbl="Гаусс 2×1D OpenMP" },
        @{ ft="Gauss"; var="Fast";    bk="oneTBB"; col=[System.Drawing.Color]::FromArgb(135,210,135); lbl="Гаусс 2×1D oneTBB" },
        @{ ft="Mean";  var="Classic"; bk="OpenMP"; col=[System.Drawing.Color]::FromArgb(214,39,40);   lbl="Средн. 2D OpenMP" },
        @{ ft="Mean";  var="Classic"; bk="oneTBB"; col=[System.Drawing.Color]::FromArgb(245,120,120); lbl="Средн. 2D oneTBB" },
        @{ ft="Mean";  var="Fast";    bk="OpenMP"; col=[System.Drawing.Color]::FromArgb(255,127,14);  lbl="Средн. рекурс. OpenMP" },
        @{ ft="Mean";  var="Fast";    bk="oneTBB"; col=[System.Drawing.Color]::FromArgb(255,187,120); lbl="Средн. рекурс. oneTBB" }
    )
    $ksizes = @(5, 9, 13)
    $maxV = 0.0
    foreach ($k in $ksizes) {
        foreach ($bs in $bSpecs) {
            $row = $df | Where-Object { [int]$_.img_id -eq 4 -and $_.filter_type -eq $bs.ft -and $_.variant -eq $bs.var -and $_.backend -eq $bs.bk -and [int]$_.ksize -eq $k -and [int]$_.threads -eq 4 } | Select-Object -First 1
            if ($row -and [double]$row.trusted_avg_med_ms -gt $maxV) { $maxV = [double]$row.trusted_avg_med_ms }
        }
    }
    $ymax = [Math]::Ceiling(($maxV + 30.0) / 50.0) * 50.0

    for ($i = 0; $i -le 6; $i++) {
        $yv = $ymax * $i / 6.0
        $py = $y0 + $ph - [float]($yv / $ymax * $ph)
        if ($i -gt 0 -and $i -lt 6) { $g.DrawLine($pGr, $x0, $py, $x0 + $pw, $py) }
        $g.DrawLine($pAx, $x0 - 6, $py, $x0, $py)
        $g.DrawString((Format-Ru $yv 0), $fK, [System.Drawing.Brushes]::Black, ($x0 - 10), $py, $sfR)
    }

    $grpW = $pw / 3.0; $barW = ($grpW - 56.0) / 8.0
    for ($gi = 0; $gi -lt 3; $gi++) {
        $k = $ksizes[$gi]; $gx = $x0 + $gi * $grpW + 28.0
        for ($bi = 0; $bi -lt 8; $bi++) {
            $bs = $bSpecs[$bi]
            $row = $df | Where-Object { [int]$_.img_id -eq 4 -and $_.filter_type -eq $bs.ft -and $_.variant -eq $bs.var -and $_.backend -eq $bs.bk -and [int]$_.ksize -eq $k -and [int]$_.threads -eq 4 } | Select-Object -First 1
            $v = [double]$row.trusted_avg_med_ms
            $bh = [float]($v / $ymax * $ph)
            $bx = $gx + $bi * $barW; $by = $y0 + $ph - $bh
            $br = New-Object System.Drawing.SolidBrush($bs.col)
            $g.FillRectangle($br, $bx + 2, $by, $barW - 4, $bh)
            $g.DrawRectangle([System.Drawing.Pens]::DimGray, $bx + 2, $by, $barW - 4, $bh)
            $br.Dispose()
            $g.DrawString((Format-Ru $v 0), $fV, [System.Drawing.Brushes]::Black, ($bx + $barW/2), ($by - 19), $sfC)
        }
        $g.DrawString("Ksize = $k (${k}×${k})", $fK, [System.Drawing.Brushes]::Black, ($x0 + ($gi + 0.5) * $grpW), ($y0 + $ph + 12), $sfC)
    }
    $g.DrawRectangle($pAx, $x0, $y0, $pw, $ph)
    $g.DrawString("Размер ядра свёртки Ksize", $fA, [System.Drawing.Brushes]::Black, ($x0 + $pw/2), ($y0 + $ph + 44), $sfC)
    $st = $g.Save(); $g.TranslateTransform($x0 - 84, $y0 + $ph/2); $g.RotateTransform(-90)
    $g.DrawString("Время выполнения, мс", $fA, [System.Drawing.Brushes]::Black, 0, 0, $sfC)
    $g.Restore($st)

    $leg = $bSpecs | ForEach-Object { @{ Kind="line"; Color=$_.col; Width=10.0; Dash=[System.Drawing.Drawing2D.DashStyle]::Solid; Marker=$false; Label=$_.lbl } }
    Draw-Legend $g ($x0 + 20) ($y0 + 14) $leg
    $c.Bmp.Save("Lab5/results/chart_classic_vs_fast_lab5.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $fT.Dispose(); $fA.Dispose(); $fK.Dispose(); $fV.Dispose(); $pAx.Dispose(); $pGr.Dispose(); $g.Dispose(); $c.Bmp.Dispose()
}.Invoke()

# 4. Визуальное сравнение исходных и отфильтрованных изображений (Гаусс vs Среднеарифметический, K=5,9,13)
{
    $c = New-Canvas 1680 620; $g = $c.G
    $fT = New-Object System.Drawing.Font("Times New Roman", 17, [System.Drawing.FontStyle]::Bold)
    $fC = New-Object System.Drawing.Font("Times New Roman", 12.5)
    $sf = New-Object System.Drawing.StringFormat; $sf.Alignment = [System.Drawing.StringAlignment]::Center
    $pB = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(120,120,120), 1.5)
    $g.DrawString("Примеры исходного (НД 1: 1280×720) и обработанного изображений: фильтр Гаусса и среднеарифметический", $fT, [System.Drawing.Brushes]::Black, 840, 12, $sf)

    $panels = @(
        @{ p="Lab5/images/input_1.bmp";          cap="Исходное НД 1 (аддитивный шум)" },
        @{ p="Lab5/images/output_1_gauss_k5.bmp";  cap="Фильтр Гаусса (2×1D), Ksize = 5" },
        @{ p="Lab5/images/output_1_gauss_k9.bmp";  cap="Фильтр Гаусса (2×1D), Ksize = 9" },
        @{ p="Lab5/images/output_1_gauss_k13.bmp"; cap="Фильтр Гаусса (2×1D), Ksize = 13" },
        @{ p="Lab5/images/input_2.bmp";          cap="Исходное НД 2 (1600×900, мишень)" },
        @{ p="Lab5/images/output_1_mean_k5.bmp";   cap="Среднеарифметический, Ksize = 5" },
        @{ p="Lab5/images/output_1_mean_k9.bmp";   cap="Среднеарифметический, Ksize = 9" },
        @{ p="Lab5/images/output_2_gauss_k9.bmp";  cap="НД 2: фильтр Гаусса, Ksize = 9" }
    )
    $cw = 392; $ch = 221; $gx = 24; $sx = 20; $rys = @(62, 344)
    for ($i = 0; $i -lt 8; $i++) {
        $x = $sx + ($i % 4) * ($cw + $gx); $y = $rys[[int][Math]::Floor($i / 4)]
        $img = [System.Drawing.Image]::FromFile((Resolve-Path $panels[$i].p).Path)
        $g.DrawImage($img, $x, $y, $cw, $ch); $g.DrawRectangle($pB, $x, $y, $cw, $ch); $img.Dispose()
        $g.DrawString($panels[$i].cap, $fC, [System.Drawing.Brushes]::Black, ($x + $cw/2), ($y + $ch + 6), $sf)
    }
    $c.Bmp.Save("Lab5/results/visual_lab5_compare.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $fT.Dispose(); $fC.Dispose(); $pB.Dispose(); $g.Dispose(); $c.Bmp.Dispose()
}.Invoke()

Write-Host "All Lab 5 charts and visual comparisons generated successfully."
