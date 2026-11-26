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
    foreach ($it in $items) {
        $sz = $g.MeasureString($it.Label, $fL)
        if ($sz.Width -gt $maxW) { $maxW = $sz.Width }
    }
    $rh = 25.0; $bw = $maxW + 60.0; $bh = $items.Count * $rh + 12.0
    $bBg = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(242,255,255,255))
    $pBr = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(140,140,140), 1.2)
    $g.FillRectangle($bBg, $lx, $ly, $bw, $bh)
    $g.DrawRectangle($pBr, $lx, $ly, $bw, $bh)
    for ($i = 0; $i -lt $items.Count; $i++) {
        $it = $items[$i]; $cy = $ly + 6 + $i * $rh + $rh / 2
        if ($it.Kind -eq "dot") {
            $b = New-Object System.Drawing.SolidBrush($it.Color); $g.FillEllipse($b, $lx + 18, $cy - 5, 10, 10); $b.Dispose()
        } elseif ($it.Kind -eq "cross") {
            $p = New-Object System.Drawing.Pen($it.Color, 2.5)
            $g.DrawLine($p, $lx + 18, $cy - 5, $lx + 28, $cy + 5); $g.DrawLine($p, $lx + 18, $cy + 5, $lx + 28, $cy - 5); $p.Dispose()
        } else {
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
        $xVal = [double]$pt[0]
        $yVal = [double]$pt[1]
        $px = $x0 + [float](($xVal - $xmin) / ($xmax - $xmin) * $pw)
        $py = $y0 + $ph - [float](([Math]::Min([double]$ymax, [Math]::Max([double]$ymin, $yVal)) - $ymin) / ($ymax - $ymin) * $ph)
        $sp += (New-Object System.Drawing.PointF($px, $py))
    }
    if ($sp.Count -ge 2) { $g.DrawLines($pen, [System.Drawing.PointF[]]$sp) }
    if ($mark) {
        foreach ($p in $sp) {
            $g.FillEllipse($br, $p.X - 5, $p.Y - 5, 10, 10)
            $g.DrawEllipse([System.Drawing.Pens]::White, $p.X - 5, $p.Y - 5, 10, 10)
        }
    }
    $pen.Dispose(); $br.Dispose()
}

$t41 = Import-Csv "Lab4/results/task4_1_summary.csv"
$t42 = Import-Csv "Lab4/results/task4_2_summary.csv"
$t43 = Import-Csv "Lab4/results/task4_3_summary.csv"
$r41 = Import-Csv "Lab4/results/task4_1_raw_samples.csv"

# 1. Точечная диаграмма доверительного интервала (Задание 4.1, Range_TBB_Red_Struct, 8000x8000, p=4, N=100)
{
    $rows = $r41 | Where-Object { $_.impl_name -eq "Range_TBB_Red_Struct" }
    $c = New-Canvas 1600 900; $g = $c.G
    $x0 = 130.0; $y0 = 85.0; $pw = 1410.0; $ph = 710.0
    $times = $rows | ForEach-Object { [double]$_.time_ms }
    $med = [double]$rows[0].median_ms; $sd = [double]$rows[0].sd_ms
    $kept = $rows | Where-Object { [int]$_.in_interval -eq 1 } | ForEach-Object { [double]$_.time_ms }
    $trusted = ($kept | Measure-Object -Average).Average
    $n = $rows.Count
    $ymin = [Math]::Max(0.0, [Math]::Min(($times | Measure-Object -Minimum).Minimum, $med - $sd) - $sd * 0.7)
    $ymax = [Math]::Max(($times | Measure-Object -Maximum).Maximum, $med + $sd) + $sd * 1.4

    Draw-Axes $g $x0 $y0 $pw $ph 0 ($n + 2) $ymin $ymax 10 6 "Номер замера" "Время выполнения, мс" `
        "Доверительный интервал: Задание 4.1, Val_range oneTBB структуры (8000x8000, p = 4, N = 100)" 0 2

    $yL = $y0 + $ph - [float](([Math]::Max($ymin, $med - $sd) - $ymin) / ($ymax - $ymin) * $ph)
    $yH = $y0 + $ph - [float](([Math]::Min($ymax, $med + $sd) - $ymin) / ($ymax - $ymin) * $ph)
    $bB = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(30, 34, 139, 34))
    $g.FillRectangle($bB, $x0, $yH, $pw, ($yL - $yH)); $bB.Dispose()

    $pM = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(20,130,50), 2.4)
    $pD = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(20,130,50), 1.8); $pD.DashStyle = [System.Drawing.Drawing2D.DashStyle]::Dash
    $pA = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(215,110,0), 2.4); $pA.DashStyle = [System.Drawing.Drawing2D.DashStyle]::DashDot
    $pyM = $y0 + $ph - [float](($med - $ymin) / ($ymax - $ymin) * $ph)
    $pyA = $y0 + $ph - [float](($trusted - $ymin) / ($ymax - $ymin) * $ph)
    $g.DrawLine($pM, $x0, $pyM, $x0 + $pw, $pyM)
    $g.DrawLine($pD, $x0, $yH,  $x0 + $pw, $yH)
    $g.DrawLine($pD, $x0, $yL,  $x0 + $pw, $yL)
    $g.DrawLine($pA, $x0, $pyA, $x0 + $pw, $pyA)

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
    $c.Bmp.Save("Lab4/results/chart_ci_task41.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $c.Bmp.Dispose()
}.Invoke()

# 2. График сравнения ускорения OpenMP и Intel oneTBB в Задании 4.1 (НД 3: 8000x8000, p=4)
{
    $c = New-Canvas 1620 900; $g = $c.G
    $x0 = 125.0; $y0 = 85.0; $pw = 1435.0; $ph = 670.0
    $fT = New-Object System.Drawing.Font("Times New Roman", 19, [System.Drawing.FontStyle]::Bold)
    $fA = New-Object System.Drawing.Font("Times New Roman", 14.5, [System.Drawing.FontStyle]::Bold)
    $fK = New-Object System.Drawing.Font("Times New Roman", 11.5)
    $fV = New-Object System.Drawing.Font("Times New Roman", 11, [System.Drawing.FontStyle]::Bold)
    $sfC = New-Object System.Drawing.StringFormat; $sfC.Alignment = [System.Drawing.StringAlignment]::Center
    $sfR = New-Object System.Drawing.StringFormat; $sfR.Alignment = [System.Drawing.StringAlignment]::Far; $sfR.LineAlignment = [System.Drawing.StringAlignment]::Center
    $pAx = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(40,40,40), 2.0)
    $pGr = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(215,215,215), 1.0); $pGr.DashStyle = [System.Drawing.Drawing2D.DashStyle]::Dash

    $g.DrawString("Ускорение Sp(4) реализаций OpenMP и Intel oneTBB в Задании 4.1 (8000x8000)", $fT, [System.Drawing.Brushes]::Black, ($x0 + $pw/2), ($y0 - 48), $sfC)

    $items = @(
        @{ code="Add_OMP_For";           lbl="Слож.`nOMP for";       col=[System.Drawing.Color]::FromArgb(55,126,184) },
        @{ code="Add_OMP_Sections";      lbl="Слож.`nOMP sect";      col=[System.Drawing.Color]::FromArgb(107,174,214) },
        @{ code="Add_TBB_TaskGroup";     lbl="Слож.`nTBB tg";        col=[System.Drawing.Color]::FromArgb(44,160,44) },
        @{ code="Add_TBB_ParFor_Lambda"; lbl="Слож.`nTBB λ";         col=[System.Drawing.Color]::FromArgb(90,190,90) },
        @{ code="Add_TBB_ParFor_Class";  lbl="Слож.`nTBB кл.";       col=[System.Drawing.Color]::FromArgb(140,215,140) },
        @{ code="Mul_OMP_For";           lbl="Умн.`nOMP for";        col=[System.Drawing.Color]::FromArgb(55,126,184) },
        @{ code="Mul_TBB_ParFor_Lambda"; lbl="Умн.`nTBB λ";          col=[System.Drawing.Color]::FromArgb(44,160,44) },
        @{ code="Sum_OMP_For_Red";       lbl="Сумма`nOMP red";       col=[System.Drawing.Color]::FromArgb(230,126,34) },
        @{ code="Sum_TBB_Red_Class";     lbl="Сумма`nTBB кл.";       col=[System.Drawing.Color]::FromArgb(214,39,40) },
        @{ code="Sum_TBB_Red_Lambda";    lbl="Сумма`nTBB λ";         col=[System.Drawing.Color]::FromArgb(240,100,100) },
        @{ code="Range_OMP_For_Red";     lbl="Range`nOMP red";       col=[System.Drawing.Color]::FromArgb(148,103,189) },
        @{ code="Range_TBB_Red_Class";   lbl="Range`nTBB кл.";       col=[System.Drawing.Color]::FromArgb(140,86,75) },
        @{ code="Range_TBB_Red_Struct";  lbl="Range`nTBB стр.";      col=[System.Drawing.Color]::FromArgb(196,156,148) }
    )
    $vals = @()
    foreach ($it in $items) {
        $code = $it.code
        $row = $t41 | Where-Object { [int]$_.ds_id -eq 3 -and $_.impl_name -eq $code -and [int]$_.threads -eq 4 } | Select-Object -First 1
        $vals += [double]$row.speedup
    }
    $ymax = [Math]::Max(2.2, [Math]::Ceiling((($vals | Measure-Object -Maximum).Maximum + 0.3) * 4) / 4.0)

    for ($i = 0; $i -le 8; $i++) {
        $yv = $ymax * $i / 8.0
        $py = $y0 + $ph - [float]($yv / $ymax * $ph)
        if ($i -gt 0 -and $i -lt 8) { $g.DrawLine($pGr, $x0, $py, $x0 + $pw, $py) }
        $g.DrawLine($pAx, $x0 - 6, $py, $x0, $py)
        $g.DrawString((Format-Ru $yv 2), $fK, [System.Drawing.Brushes]::Black, ($x0 - 10), $py, $sfR)
    }

    $slotW = $pw / $items.Count
    for ($i = 0; $i -lt $items.Count; $i++) {
        $v = $vals[$i]
        $bh = [float]($v / $ymax * $ph)
        $bx = $x0 + $i * $slotW + 12
        $bw = $slotW - 24
        $by = $y0 + $ph - $bh
        $br = New-Object System.Drawing.SolidBrush($items[$i].col)
        $g.FillRectangle($br, $bx, $by, $bw, $bh)
        $g.DrawRectangle([System.Drawing.Pens]::DimGray, $bx, $by, $bw, $bh)
        $br.Dispose()
        $g.DrawString((Format-Ru $v 2), $fV, [System.Drawing.Brushes]::Black, ($bx + $bw/2), ($by - 22), $sfC)
        $g.DrawString($items[$i].lbl, $fK, [System.Drawing.Brushes]::Black, ($bx + $bw/2), ($y0 + $ph + 10), $sfC)
    }
    $g.DrawRectangle($pAx, $x0, $y0, $pw, $ph)
    $st = $g.Save(); $g.TranslateTransform($x0 - 75, $y0 + $ph/2); $g.RotateTransform(-90)
    $g.DrawString("Ускорение Sp(4)", $fA, [System.Drawing.Brushes]::Black, 0, 0, $sfC)
    $g.Restore($st)
    $c.Bmp.Save("Lab4/results/chart_task41_speedup.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $fT.Dispose(); $fA.Dispose(); $fK.Dispose(); $fV.Dispose(); $pAx.Dispose(); $pGr.Dispose(); $g.Dispose(); $c.Bmp.Dispose()
}.Invoke()

# 3. Ускорение Заданий 4.2 (медианный) и 4.3 (текстуры) — OpenMP vs Intel oneTBB + Амдал/Густавсон (2 панели)
{
    $c = New-Canvas 1680 800; $g = $c.G
    $pw = 675.0; $ph = 580.0; $y0 = 90.0; $xL = 110.0; $xR = 955.0

    Draw-Axes $g $xL $y0 $pw $ph 1 4 0 4.5 3 9 "Число потоков, p" "Ускорение, Sp(n)" "Задание 4.2: Медианный фильтр (2560x1440)" 0 1 @(1,2,3,4)
    Draw-Axes $g $xR $y0 $pw $ph 1 4 0 4.5 3 9 "Число потоков, p" "Ускорение, Sp(n)" "Задание 4.3: Текстурные признаки (2560x1440)" 0 1 @(1,2,3,4)

    # Идеальное ускорение (f=0) и Амдал f=0.10
    $idPts = @(@(1,1),@(2,2),@(3,3),@(4,4))
    $amPts = 1..4 | ForEach-Object { ,@($_, (1.0 / (0.10 + 0.90 / $_))) }
    Draw-Line $g $idPts $xL $y0 $pw $ph 1 4 0 4.5 ([System.Drawing.Color]::Gray) 2.0 ([System.Drawing.Drawing2D.DashStyle]::Dash) $false
    Draw-Line $g $amPts $xL $y0 $pw $ph 1 4 0 4.5 ([System.Drawing.Color]::FromArgb(140,110,160)) 1.8 ([System.Drawing.Drawing2D.DashStyle]::Dot) $false
    Draw-Line $g $idPts $xR $y0 $pw $ph 1 4 0 4.5 ([System.Drawing.Color]::Gray) 2.0 ([System.Drawing.Drawing2D.DashStyle]::Dash) $false
    Draw-Line $g $amPts $xR $y0 $pw $ph 1 4 0 4.5 ([System.Drawing.Color]::FromArgb(140,110,160)) 1.8 ([System.Drawing.Drawing2D.DashStyle]::Dot) $false

    $s42 = @(
        @{ impl="Median_OpenMP"; k=13; col=[System.Drawing.Color]::FromArgb(31,119,180);  lbl="OpenMP, K = 13" },
        @{ impl="Median_oneTBB"; k=13; col=[System.Drawing.Color]::FromArgb(214,39,40);   lbl="Intel oneTBB, K = 13" },
        @{ impl="Median_OpenMP"; k=5;  col=[System.Drawing.Color]::FromArgb(100,175,230); lbl="OpenMP, K = 5" },
        @{ impl="Median_oneTBB"; k=5;  col=[System.Drawing.Color]::FromArgb(255,127,14);  lbl="Intel oneTBB, K = 5" }
    )
    $legL = @()
    foreach ($s in $s42) {
        $pts = New-Object System.Collections.ArrayList
        [void]$pts.Add(@(1.0, 1.0))
        foreach ($p in @(2,3,4)) {
            $row = $t42 | Where-Object { [int]$_.img_id -eq 3 -and $_.impl_name -eq $s.impl -and [int]$_.ksize -eq $s.k -and [int]$_.threads -eq $p } | Select-Object -First 1
            if ($row) { [void]$pts.Add(@([double]$p, [double]$row.speedup)) }
        }
        Draw-Line $g $pts $xL $y0 $pw $ph 1 4 0 4.5 $s.col 2.8 ([System.Drawing.Drawing2D.DashStyle]::Solid) $true
        $legL += @{ Kind="line"; Color=$s.col; Width=2.8; Dash=[System.Drawing.Drawing2D.DashStyle]::Solid; Marker=$true; Label=$s.lbl }
    }
    $legL += @{ Kind="line"; Color=[System.Drawing.Color]::Gray; Width=2.0; Dash=[System.Drawing.Drawing2D.DashStyle]::Dash; Marker=$false; Label="Идеальное ускорение (Густавсон, f=0)" }
    $legL += @{ Kind="line"; Color=[System.Drawing.Color]::FromArgb(140,110,160); Width=1.8; Dash=[System.Drawing.Drawing2D.DashStyle]::Dot; Marker=$false; Label="Закон Амдала (f = 0,10)" }
    Draw-Legend $g ($xL + 16) ($y0 + 14) $legL

    $s43 = @(
        @{ impl="Texture_OpenMP";    k=9; col=[System.Drawing.Color]::FromArgb(31,119,180);  lbl="OpenMP, 9×9" },
        @{ impl="Texture_oneTBB_1D"; k=9; col=[System.Drawing.Color]::FromArgb(214,39,40);   lbl="Intel oneTBB 1D, 9×9" },
        @{ impl="Texture_oneTBB_2D"; k=9; col=[System.Drawing.Color]::FromArgb(44,160,44);   lbl="Intel oneTBB 2D (range2d), 9×9" },
        @{ impl="Texture_oneTBB_2D"; k=5; col=[System.Drawing.Color]::FromArgb(255,127,14);  lbl="Intel oneTBB 2D (range2d), 5×5" }
    )
    $legR = @()
    foreach ($s in $s43) {
        $pts = New-Object System.Collections.ArrayList
        [void]$pts.Add(@(1.0, 1.0))
        foreach ($p in @(2,3,4)) {
            $row = $t43 | Where-Object { [int]$_.img_id -eq 2 -and $_.impl_name -eq $s.impl -and [int]$_.ksize -eq $s.k -and [int]$_.threads -eq $p } | Select-Object -First 1
            if ($row) { [void]$pts.Add(@([double]$p, [double]$row.speedup)) }
        }
        Draw-Line $g $pts $xR $y0 $pw $ph 1 4 0 4.5 $s.col 2.8 ([System.Drawing.Drawing2D.DashStyle]::Solid) $true
        $legR += @{ Kind="line"; Color=$s.col; Width=2.8; Dash=[System.Drawing.Drawing2D.DashStyle]::Solid; Marker=$true; Label=$s.lbl }
    }
    $legR += @{ Kind="line"; Color=[System.Drawing.Color]::Gray; Width=2.0; Dash=[System.Drawing.Drawing2D.DashStyle]::Dash; Marker=$false; Label="Идеальное ускорение (f=0)" }
    $legR += @{ Kind="line"; Color=[System.Drawing.Color]::FromArgb(140,110,160); Width=1.8; Dash=[System.Drawing.Drawing2D.DashStyle]::Dot; Marker=$false; Label="Закон Амдала (f = 0,10)" }
    Draw-Legend $g ($xR + 16) ($y0 + 14) $legR

    $c.Bmp.Save("Lab4/results/chart_task42_43_speedup.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $c.Bmp.Dispose()
}.Invoke()

# 4. Визуальное сравнение Задания 4.2 (Медианный фильтр OpenMP / oneTBB: до и после K=5,9,13)
{
    $c = New-Canvas 1680 620; $g = $c.G
    $fT = New-Object System.Drawing.Font("Times New Roman", 17, [System.Drawing.FontStyle]::Bold)
    $fC = New-Object System.Drawing.Font("Times New Roman", 12.5)
    $sf = New-Object System.Drawing.StringFormat; $sf.Alignment = [System.Drawing.StringAlignment]::Center
    $pB = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(120,120,120), 1.5)
    $g.DrawString("Задание 4.2: примеры исходных (зашумлённых) и обработанных медианным фильтром BMP", $fT, [System.Drawing.Brushes]::Black, 840, 12, $sf)

    $panels = @(
        @{ p="Lab4/images/input_1.bmp";        cap="НД 1 (1280×720): исходное (шум 7%)" },
        @{ p="Lab4/images/output_1_tbb_k5.bmp";  cap="НД 1 (oneTBB): Ksize = 5" },
        @{ p="Lab4/images/output_1_tbb_k9.bmp";  cap="НД 1 (oneTBB): Ksize = 9" },
        @{ p="Lab4/images/output_1_tbb_k13.bmp"; cap="НД 1 (oneTBB): Ksize = 13" },
        @{ p="Lab4/images/input_2.bmp";        cap="НД 2 (1920×1080): исходное (шум 7%)" },
        @{ p="Lab4/images/output_2_omp_k5.bmp";  cap="НД 2 (OpenMP): Ksize = 5" },
        @{ p="Lab4/images/input_3.bmp";        cap="НД 3 (2560×1440): исходное (шум 7%)" },
        @{ p="Lab4/images/output_3_tbb_k5.bmp";  cap="НД 3 (oneTBB): Ksize = 5" }
    )
    $cw = 392; $ch = 221; $gx = 24; $sx = 20; $rys = @(62, 344)
    for ($i = 0; $i -lt 8; $i++) {
        $x = $sx + ($i % 4) * ($cw + $gx); $y = $rys[[int][Math]::Floor($i / 4)]
        $img = [System.Drawing.Image]::FromFile((Resolve-Path $panels[$i].p).Path)
        $g.DrawImage($img, $x, $y, $cw, $ch); $g.DrawRectangle($pB, $x, $y, $cw, $ch); $img.Dispose()
        $g.DrawString($panels[$i].cap, $fC, [System.Drawing.Brushes]::Black, ($x + $cw/2), ($y + $ch + 6), $sf)
    }
    $c.Bmp.Save("Lab4/results/visual_task42_median.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $fT.Dispose(); $fC.Dispose(); $pB.Dispose(); $g.Dispose(); $c.Bmp.Dispose()
}.Invoke()

# 5. Визуальное сравнение Задания 4.3: исходное изображение + 4 цветные карты текстурных признаков (M2, R, U, E)
{
    $c = New-Canvas 1680 760; $g = $c.G
    $fT = New-Object System.Drawing.Font("Times New Roman", 17, [System.Drawing.FontStyle]::Bold)
    $fC = New-Object System.Drawing.Font("Times New Roman", 12.5)
    $sf = New-Object System.Drawing.StringFormat; $sf.Alignment = [System.Drawing.StringAlignment]::Center
    $pB = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(110,110,110), 1.5)
    $g.DrawString("Задание 4.3: исходное изображение НД 1 (1280×720) и карты текстурных признаков (окно 7×7, k1=0,30, k2=0,70)", $fT, [System.Drawing.Brushes]::Black, 840, 12, $sf)

    # Легенда цветов порогов под заголовком
    $fL = New-Object System.Drawing.Font("Times New Roman", 12)
    $bG = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(0,200,0))
    $bY = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(255,220,0))
    $bR = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(220,30,30))
    $g.FillRectangle($bG, 360, 48, 22, 16); $g.DrawRectangle($pB, 360, 48, 22, 16); $g.DrawString("TF ≤ T1 (низкий)", $fL, [System.Drawing.Brushes]::Black, 388, 46)
    $g.FillRectangle($bY, 680, 48, 22, 16); $g.DrawRectangle($pB, 680, 48, 22, 16); $g.DrawString("T1 < TF < T2 (средний)", $fL, [System.Drawing.Brushes]::Black, 708, 46)
    $g.FillRectangle($bR, 1050, 48, 22, 16); $g.DrawRectangle($pB, 1050, 48, 22, 16); $g.DrawString("TF ≥ T2 (высокий)", $fL, [System.Drawing.Brushes]::Black, 1078, 46)
    $bG.Dispose(); $bY.Dispose(); $bR.Dispose(); $fL.Dispose()

    # Верхний ряд: 3 панели (Исходное, M2, R); Нижний ряд: 3 панели (U, E, Энтропия окно 9x9 для сравнения)
    $panels = @(
        @{ p="Lab4/images/input_tex_1.bmp";            cap="Исходное изображение (1280×720)" },
        @{ p="Lab4/images/output_tex_1_tbb_7_M2.bmp";  cap="Карта момента 2-го порядка μ2 (7×7)" },
        @{ p="Lab4/images/output_tex_1_tbb_7_R.bmp";   cap="Карта относительной гладкости R (7×7)" },
        @{ p="Lab4/images/output_tex_1_tbb_7_U.bmp";   cap="Карта однородности U (7×7)" },
        @{ p="Lab4/images/output_tex_1_tbb_7_E.bmp";   cap="Карта средней энтропии e (7×7)" },
        @{ p="Lab4/images/output_tex_1_tbb_9_E.bmp";   cap="Карта средней энтропии e (9×9)" }
    )
    $cw = 520; $ch = 292; $gx = 32; $sx = 28; $rys = @(82, 422)
    for ($i = 0; $i -lt 6; $i++) {
        $x = $sx + ($i % 3) * ($cw + $gx); $y = $rys[[int][Math]::Floor($i / 3)]
        $img = [System.Drawing.Image]::FromFile((Resolve-Path $panels[$i].p).Path)
        $g.DrawImage($img, $x, $y, $cw, $ch); $g.DrawRectangle($pB, $x, $y, $cw, $ch); $img.Dispose()
        $g.DrawString($panels[$i].cap, $fC, [System.Drawing.Brushes]::Black, ($x + $cw/2), ($y + $ch + 6), $sf)
    }
    $c.Bmp.Save("Lab4/results/visual_task43_textures.png", [System.Drawing.Imaging.ImageFormat]::Png)
    $fT.Dispose(); $fC.Dispose(); $pB.Dispose(); $g.Dispose(); $c.Bmp.Dispose()
}.Invoke()

Write-Host "All Lab 4 charts and visual maps generated successfully."
