import csv
import math
import zipfile
from xml.sax.saxutils import escape


def col_letter(idx: int) -> str:
    res = []
    while idx > 0:
        idx, rem = divmod(idx - 1, 26)
        res.append(chr(65 + rem))
    return "".join(reversed(res))


def cell_ref(row: int, col: int) -> str:
    return f"{col_letter(col)}{row}"


class SimpleSheet:
    def __init__(self, name: str):
        self.name = name
        self.rows = {}
        self.merges = []
        self.col_widths = {}

    def set_str(self, r: int, c: int, text: str, style: int = 0):
        self.rows.setdefault(r, {})[c] = ("str", str(text), style, None)

    def set_num(self, r: int, c: int, val: float, style: int = 4):
        self.rows.setdefault(r, {})[c] = ("num", float(val), style, None)

    def set_int(self, r: int, c: int, val: int, style: int = 5):
        self.rows.setdefault(r, {})[c] = ("int", int(val), style, None)

    def set_formula(self, r: int, c: int, formula: str, cached_val, style: int = 4):
        self.rows.setdefault(r, {})[c] = ("formula", cached_val, style, formula)

    def merge(self, r1: int, c1: int, r2: int, c2: int):
        self.merges.append(f"{cell_ref(r1, c1)}:{cell_ref(r2, c2)}")

    def set_col_width(self, c: int, w: float):
        self.col_widths[c] = max(self.col_widths.get(c, 0.0), w)

    def to_xml(self) -> str:
        out = [
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>',
            '<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">',
        ]
        max_col = 1
        for r, cols in self.rows.items():
            for c, (kind, val, style, formula) in cols.items():
                max_col = max(max_col, c)
                est = len(str(val)) + 3 if kind == "str" else 13
                if c not in self.col_widths:
                    self.col_widths[c] = min(max(est, 9), 36)
        out.append("<cols>")
        for c in range(1, max_col + 1):
            out.append(f'<col min="{c}" max="{c}" width="{self.col_widths.get(c, 12.0):.2f}" customWidth="1"/>')
        out.append("</cols><sheetData>")
        for r in sorted(self.rows.keys()):
            out.append(f'<row r="{r}">')
            for c in sorted(self.rows[r].keys()):
                kind, val, style, formula = self.rows[r][c]
                ref = cell_ref(r, c)
                if kind == "str":
                    out.append(f'<c r="{ref}" s="{style}" t="inlineStr"><is><t>{escape(val)}</t></is></c>')
                elif kind == "num":
                    out.append(f'<c r="{ref}" s="{style}"><v>{val:.6f}</v></c>')
                elif kind == "int":
                    out.append(f'<c r="{ref}" s="{style}"><v>{val}</v></c>')
                elif kind == "formula":
                    out.append(f'<c r="{ref}" s="{style}"><f>{escape(formula)}</f><v>{float(val):.6f}</v></c>')
            out.append("</row>")
        out.append("</sheetData>")
        if self.merges:
            out.append(f'<mergeCells count="{len(self.merges)}">')
            for m in self.merges:
                out.append(f'<mergeCell ref="{m}"/>')
            out.append("</mergeCells>")
        out.append('<pageSetup orientation="landscape" paperSize="9"/></worksheet>')
        return "\n".join(out)


def build_styles_xml() -> str:
    return """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
  <numFmts count="1"><numFmt numFmtId="164" formatCode="0.0000"/></numFmts>
  <fonts count="4">
    <font><sz val="11"/><name val="Times New Roman"/></font>
    <font><b/><sz val="13"/><name val="Times New Roman"/></font>
    <font><b/><sz val="11"/><name val="Times New Roman"/></font>
    <font><i/><sz val="10"/><name val="Times New Roman"/></font>
  </fonts>
  <fills count="5">
    <fill><patternFill patternType="none"/></fill>
    <fill><patternFill patternType="gray125"/></fill>
    <fill><patternFill patternType="solid"><fgColor rgb="FFD9E1F2"/><bgColor indexed="64"/></patternFill></fill>
    <fill><patternFill patternType="solid"><fgColor rgb="FFEDEDED"/><bgColor indexed="64"/></patternFill></fill>
    <fill><patternFill patternType="solid"><fgColor rgb="FFEAF3FA"/><bgColor indexed="64"/></patternFill></fill>
  </fills>
  <borders count="2">
    <border><left/><right/><top/><bottom/><diagonal/></border>
    <border>
      <left style="thin"><color rgb="FF595959"/></left>
      <right style="thin"><color rgb="FF595959"/></right>
      <top style="thin"><color rgb="FF595959"/></top>
      <bottom style="thin"><color rgb="FF595959"/></bottom>
      <diagonal/>
    </border>
  </borders>
  <cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
  <cellXfs count="10">
    <xf numFmtId="0" fontId="0" fillId="0" borderId="1" applyBorder="1" applyAlignment="1"><alignment horizontal="left" vertical="center" wrapText="1"/></xf>
    <xf numFmtId="0" fontId="1" fillId="0" borderId="0" applyFont="1" applyAlignment="1"><alignment horizontal="left" vertical="center"/></xf>
    <xf numFmtId="0" fontId="2" fillId="2" borderId="1" applyFont="1" applyFill="1" applyBorder="1" applyAlignment="1"><alignment horizontal="center" vertical="center" wrapText="1"/></xf>
    <xf numFmtId="0" fontId="2" fillId="3" borderId="1" applyFont="1" applyFill="1" applyBorder="1" applyAlignment="1"><alignment horizontal="center" vertical="center" wrapText="1"/></xf>
    <xf numFmtId="164" fontId="0" fillId="0" borderId="1" applyNumberFormat="1" applyBorder="1" applyAlignment="1"><alignment horizontal="right" vertical="center"/></xf>
    <xf numFmtId="1" fontId="0" fillId="0" borderId="1" applyNumberFormat="1" applyBorder="1" applyAlignment="1"><alignment horizontal="center" vertical="center"/></xf>
    <xf numFmtId="0" fontId="0" fillId="0" borderId="1" applyBorder="1" applyAlignment="1"><alignment horizontal="center" vertical="center" wrapText="1"/></xf>
    <xf numFmtId="164" fontId="0" fillId="0" borderId="1" applyNumberFormat="1" applyBorder="1" applyAlignment="1"><alignment horizontal="right" vertical="center"/></xf>
    <xf numFmtId="164" fontId="2" fillId="4" borderId="1" applyFont="1" applyFill="1" applyNumberFormat="1" applyBorder="1" applyAlignment="1"><alignment horizontal="right" vertical="center"/></xf>
    <xf numFmtId="0" fontId="3" fillId="0" borderId="0" applyFont="1" applyAlignment="1"><alignment horizontal="left" vertical="center"/></xf>
  </cellXfs>
</styleSheet>
"""


def write_xlsx(path: str, sheets: list):
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zf:
        ct = [
            '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>',
            '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>',
        ] + [
            f'<Override PartName="/xl/worksheets/sheet{i}.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
            for i in range(1, len(sheets) + 1)
        ]
        zf.writestr("[Content_Types].xml",
                    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">\n'
                    '  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>\n'
                    '  <Default Extension="xml" ContentType="application/xml"/>\n' + "\n".join("  " + x for x in ct) + "\n</Types>")
        zf.writestr("_rels/.rels",
                    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">\n'
                    '  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>\n</Relationships>')
        sh_e = [f'<sheet name="{escape(s.name)}" sheetId="{i}" r:id="rId{i}"/>' for i, s in enumerate(sheets, 1)]
        rel_e = ['<Relationship Id="rIdStyles" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>'] + [
            f'<Relationship Id="rId{i}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet{i}.xml"/>'
            for i in range(1, len(sheets) + 1)
        ]
        zf.writestr("xl/workbook.xml",
                    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" '
                    'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">\n  <sheets>\n    ' + "\n    ".join(sh_e) + "\n  </sheets>\n</workbook>")
        zf.writestr("xl/_rels/workbook.xml.rels",
                    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">\n'
                    + "\n".join("  " + x for x in rel_e) + "\n</Relationships>")
        zf.writestr("xl/styles.xml", build_styles_xml())
        for i, s in enumerate(sheets, 1):
            zf.writestr(f"xl/worksheets/sheet{i}.xml", s.to_xml())


def load_csv(path: str):
    with open(path, "r", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def estimate_f(pairs):
    num = den = 0.0
    for p, sp in pairs:
        if p <= 1 or sp <= 0:
            continue
        x = 1.0 - 1.0 / p
        y = (1.0 / sp) - (1.0 / p)
        num += x * y
        den += x * x
    return max(0.0, min(1.0, num / den)) if den > 0 else 0.0


def build_filter_sheet(name: str, title: str, subtitle: str, ftype: str, idx: dict) -> SimpleSheet:
    s = SimpleSheet(name)
    s.set_str(1, 1, title, 1)
    s.set_str(2, 1, subtitle, 9)

    s.set_str(3, 1, "Функция (реализация)", 2); s.merge(3, 1, 4, 1)
    s.set_str(3, 2, "Поток (p)", 2);            s.merge(3, 2, 4, 2)
    s.set_str(3, 3, "Ksize", 2);                s.merge(3, 3, 4, 3)

    ds_lbls = ["НД 1 (1280×720)", "НД 2 (1600×900)", "НД 3 (1920×1080)", "НД 4 (2560×1440)"]
    for di, lbl in enumerate(ds_lbls):
        c0 = 4 + di * 4
        s.set_str(3, c0, lbl, 2)
        for k in range(1, 4): s.set_str(3, c0 + k, "", 2)
        s.merge(3, c0, 3, c0 + 3)
        s.set_str(4, c0 + 0, "Время (мс)", 2)
        s.set_str(4, c0 + 1, "Sp(n)", 2)
        s.set_str(4, c0 + 2, "Ep(n)", 2)
        s.set_str(4, c0 + 3, "Cp(мс)", 2)

    s.set_col_width(1, 34); s.set_col_width(2, 10); s.set_col_width(3, 8)
    for c in range(4, 20): s.set_col_width(c, 12.0)

    specs = [
        ("Фильтр классический (посл.)", "Classic", "Seq", [1]),
        ("Фильтр классический OpenMP",  "Classic", "OpenMP", [2, 3, 4]),
        ("Фильтр классический oneTBB",  "Classic", "oneTBB", [2, 3, 4]),
        ("Фильтр быстрый (посл.)",      "Fast",    "Seq", [1]),
        ("Фильтр быстрый OpenMP",       "Fast",    "OpenMP", [2, 3, 4]),
        ("Фильтр быстрый oneTBB",       "Fast",    "oneTBB", [2, 3, 4]),
    ]

    r = 5
    seq_rows = {}
    for lbl, var, bk, ths in specs:
        for p in ths:
            for ksz in [5, 9, 13]:
                if p == 1:
                    seq_rows[(var, ksz)] = r
                s.set_str(r, 1, lbl, 0)
                s.set_int(r, 2, p, 5)
                s.set_int(r, 3, ksz, 5)
                br = seq_rows[(var, ksz)]
                for di in range(4):
                    c0 = 4 + di * 4
                    rec = idx[(di + 1, ftype, var, bk, ksz, p)]
                    t_ms = float(rec["trusted_avg_med_ms"])
                    s.set_num(r, c0, t_ms, 8 if p == 1 else 4)
                    s.set_formula(r, c0 + 1, f"{cell_ref(br, c0)}/{cell_ref(r, c0)}", float(rec["speedup_own"]), 4)
                    s.set_formula(r, c0 + 2, f"{cell_ref(r, c0 + 1)}/{cell_ref(r, 2)}", float(rec["efficiency"]), 4)
                    s.set_formula(r, c0 + 3, f"{cell_ref(r, 2)}*{cell_ref(r, c0)}", float(rec["cost_ms"]), 4)
                r += 1
    return s


def main():
    rows = load_csv("Lab5/results/lab5_summary.csv")
    raw = load_csv("Lab5/results/lab5_raw_samples.csv")
    idx = {(int(r["img_id"]), r["filter_type"], r["variant"], r["backend"], int(r["ksize"]), int(r["threads"])): r for r in rows}

    s1 = build_filter_sheet(
        "Табл. 5.1 (Гаусс)",
        "Таблица 5.1 (часть 1) – Фильтр Гаусса: классический 2D и быстрый 2×1D с транспонированием (15 запусков)",
        "Метод фильтрации выбросов: AvgTrustedIntervalMed (med ± SD); σ = (Ksize − 1) / 4; единицы времени: мс",
        "Gauss", idx
    )
    s2 = build_filter_sheet(
        "Табл. 5.1 (Среднеарифм.)",
        "Таблица 5.1 (часть 2) – Среднеарифметический фильтр: классический 2D и быстрый рекурсивный (15 запусков)",
        "Метод фильтрации выбросов: AvgTrustedIntervalMed (med ± SD); единицы времени: мс",
        "Mean", idx
    )

    # Лист 3: Сравнение классического и быстрого алгоритмов (алгоритмическое + параллельное ускорение на НД 4)
    s3 = SimpleSheet("Классич. vs Быстрый")
    s3.set_str(1, 1, "Сравнение классической 2D-свёртки и быстрого алгоритма на НД 4 (2560×1440)", 1)
    hdr3 = ["Фильтр", "Ksize", "Классич. посл. (мс)", "Быстрый посл. (мс)", "Выигрыш алг. (раз)", "Быстрый OpenMP p=4 (мс)", "Быстрый oneTBB p=4 (мс)", "Итоговое ускор. (раз)"]
    for ci, h in enumerate(hdr3, 1): s3.set_str(3, ci, h, 2)
    s3.set_col_width(1, 28)
    for c in range(2, 9): s3.set_col_width(c, 18)

    r = 4
    for ft, f_ru in [("Gauss", "Фильтр Гаусса (2×1D)"), ("Mean", "Среднеарифметический (рекурс.)")]:
        for ksz in [5, 9, 13]:
            tc1 = float(idx[(4, ft, "Classic", "Seq", ksz, 1)]["trusted_avg_med_ms"])
            tf1 = float(idx[(4, ft, "Fast", "Seq", ksz, 1)]["trusted_avg_med_ms"])
            tf_o = float(idx[(4, ft, "Fast", "OpenMP", ksz, 4)]["trusted_avg_med_ms"])
            tf_t = float(idx[(4, ft, "Fast", "oneTBB", ksz, 4)]["trusted_avg_med_ms"])
            s3.set_str(r, 1, f_ru, 0)
            s3.set_int(r, 2, ksz, 5)
            s3.set_num(r, 3, tc1, 4)
            s3.set_num(r, 4, tf1, 4)
            s3.set_formula(r, 5, f"{cell_ref(r,3)}/{cell_ref(r,4)}", tc1 / tf1, 8)
            s3.set_num(r, 6, tf_o, 4)
            s3.set_num(r, 7, tf_t, 4)
            s3.set_formula(r, 8, f"{cell_ref(r,3)}/MIN({cell_ref(r,6)},{cell_ref(r,7)})", tc1 / min(tf_o, tf_t), 8)
            r += 1

    # Лист 4: Доверительный интервал (100 замеров)
    s4 = SimpleSheet("Доверительный интервал")
    s4.set_str(1, 1, "Проверка расчёта доверительного интервала (AvgTrustedIntervalMed: med ± SD, N = 100, НД 4, K = 13, p = 4)", 1)
    ser_g = [x for x in raw if x["filter_type"] == "Gauss"]
    ser_m = [x for x in raw if x["filter_type"] == "Mean"]

    def write_ci(sh, sc, title, ser):
        sh.set_str(3, sc, title, 3)
        for k in range(1, 5): sh.set_str(3, sc + k, "", 3)
        sh.merge(3, sc, 3, sc + 4)
        vals = [float(x["time_ms"]) for x in ser]
        n = len(vals)
        avg = sum(vals) / n
        sd = math.sqrt(sum((v - avg) ** 2 for v in vals) / (n - 1))
        sv = sorted(vals)
        med = 0.5 * (sv[n // 2 - 1] + sv[n // 2]) if n % 2 == 0 else sv[n // 2]
        kept = [v for v in vals if med - sd <= v <= med + sd]
        tr = sum(kept) / max(1, len(kept))
        r0, r1 = 12, 12 + n - 1
        vr = f"{cell_ref(r0, sc + 1)}:{cell_ref(r1, sc + 1)}"
        fr = f"{cell_ref(r0, sc + 4)}:{cell_ref(r1, sc + 4)}"
        for ii, (lb, cv, fm, st) in enumerate([
            ("Число замеров N", n, f"COUNT({vr})", 5),
            ("Исходное среднее (мс)", avg, f"AVERAGE({vr})", 4),
            ("Медиана med (мс)", med, f"MEDIAN({vr})", 4),
            ("Станд. отклонение SD (мс)", sd, f"STDEV({vr})", 4),
            ("Отброшено выбросов", n - len(kept), f"COUNTIF({fr},0)", 5),
            ("Итог AvgTrustedIntervalMed (мс)", tr, f"AVERAGEIF({fr},1,{vr})", 8),
        ]):
            rr = 4 + ii
            sh.set_str(rr, sc, lb, 0); sh.set_str(rr, sc + 1, "", 0); sh.merge(rr, sc, rr, sc + 1)
            sh.set_formula(rr, sc + 2, fm, cv, st)
            sh.set_str(rr, sc + 3, "", 0); sh.set_str(rr, sc + 4, "", 0)
        mc, sdc = f"${col_letter(sc+2)}$6", f"${col_letter(sc+2)}$7"
        for k, h in enumerate(["№ замера", "Время Xi (мс)", "med - SD", "med + SD", "В интервале (1/0)"]):
            sh.set_str(11, sc + k, h, 2)
        for iv, v in enumerate(vals):
            rr = r0 + iv
            sh.set_int(rr, sc, iv + 1, 5)
            sh.set_num(rr, sc + 1, v, 4)
            sh.set_formula(rr, sc + 2, f"{mc}-{sdc}", med - sd, 4)
            sh.set_formula(rr, sc + 3, f"{mc}+{sdc}", med + sd, 4)
            sh.set_formula(rr, sc + 4, f"IF(AND({cell_ref(rr,sc+1)}>={cell_ref(rr,sc+2)},{cell_ref(rr,sc+1)}<={cell_ref(rr,sc+3)}),1,0)", 1 if (med-sd<=v<=med+sd) else 0, 5)

    write_ci(s4, 1, "Гаусс быстрый 2×1D oneTBB (2560×1440, K=13, p=4)", ser_g)
    write_ci(s4, 7, "Среднеарифм. рекурсивный oneTBB (2560×1440, K=13, p=4)", ser_m)
    for c in [1, 7]:
        s4.set_col_width(c, 12); s4.set_col_width(c+1, 15); s4.set_col_width(c+2, 14); s4.set_col_width(c+3, 14); s4.set_col_width(c+4, 17)

    # Лист 5: Амдал и Густавсон–Барсис
    s5 = SimpleSheet("Амдал и Густавсон")
    s5.set_str(1, 1, "Оценка доли последовательных вычислений f и сравнение с законами Амдала и Густавсона–Барсиса (НД 4, K = 13)", 1)
    for ci, h in enumerate(["Алгоритм", "Оценка f", "S* = 1/f", "Sp(4) эксп.", "Амдал (4)", "Густавсон (4)"], 1):
        s5.set_str(3, ci, h, 2)
    s5.set_col_width(1, 44)
    for c in range(2, 7): s5.set_col_width(c, 16)

    models = [
        ("Гаусс классич. 2D OpenMP (2560×1440, K=13)", "Gauss", "Classic", "OpenMP"),
        ("Гаусс классич. 2D oneTBB (2560×1440, K=13)", "Gauss", "Classic", "oneTBB"),
        ("Гаусс быстрый 2×1D OpenMP (2560×1440, K=13)", "Gauss", "Fast", "OpenMP"),
        ("Гаусс быстрый 2×1D oneTBB (2560×1440, K=13)", "Gauss", "Fast", "oneTBB"),
        ("Среднеарифм. классич. 2D OpenMP (2560×1440, K=13)", "Mean", "Classic", "OpenMP"),
        ("Среднеарифм. классич. 2D oneTBB (2560×1440, K=13)", "Mean", "Classic", "oneTBB"),
        ("Среднеарифм. рекурс. OpenMP (2560×1440, K=13)", "Mean", "Fast", "OpenMP"),
        ("Среднеарифм. рекурс. oneTBB (2560×1440, K=13)", "Mean", "Fast", "oneTBB"),
    ]
    r = 4
    for title, ft, var, bk in models:
        pts = [(1, 1.0)] + [(p, float(idx[(4, ft, var, bk, 13, p)]["speedup_own"])) for p in [2, 3, 4]]
        fe = estimate_f(pts)
        sp4 = dict(pts)[4]
        s5.set_str(r, 1, title, 0)
        s5.set_num(r, 2, fe, 4)
        s5.set_formula(r, 3, f"IF({cell_ref(r,2)}>0.0001,1/{cell_ref(r,2)},999)", (1.0 / fe) if fe > 1e-4 else 999.0, 4)
        s5.set_num(r, 4, sp4, 4)
        s5.set_formula(r, 5, f"1/({cell_ref(r,2)}+(1-{cell_ref(r,2)})/4)", 1.0 / (fe + (1.0 - fe) / 4.0), 4)
        s5.set_formula(r, 6, f"{cell_ref(r,2)}+(1-{cell_ref(r,2)})*4", fe + (1.0 - fe) * 4.0, 4)
        r += 1

    out_path = "Lab5/Результаты_исследования_Лаб5_OpenMP_oneTBB.xlsx"
    write_xlsx(out_path, [s1, s2, s3, s4, s5])
    print(f"Successfully generated {out_path}")


if __name__ == "__main__":
    main()
