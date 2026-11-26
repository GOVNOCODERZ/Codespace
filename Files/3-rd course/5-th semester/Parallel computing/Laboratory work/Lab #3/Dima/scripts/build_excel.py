import csv
import math
import os
import zipfile
from xml.sax.saxutils import escape

def col_letter(idx: int) -> str:
    """1-based column index to Excel column letters (1->A, 27->AA)."""
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
        self.rows = {}       # r -> {c: (kind, val, style, formula)}
        self.merges = []     # list of "A1:C1"
        self.col_widths = {} # col -> width

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
        # Auto-fit column widths based on content if not explicitly set
        max_col = 1
        for r, cols in self.rows.items():
            for c, (kind, val, style, formula) in cols.items():
                max_col = max(max_col, c)
                est = len(str(val)) + 3 if kind == "str" else 13
                if c not in self.col_widths:
                    self.col_widths[c] = min(max(est, 9), 34)

        out.append("<cols>")
        for c in range(1, max_col + 1):
            w = self.col_widths.get(c, 12.0)
            out.append(f'<col min="{c}" max="{c}" width="{w:.2f}" customWidth="1"/>')
        out.append("</cols>")

        out.append("<sheetData>")
        for r in sorted(self.rows.keys()):
            out.append(f'<row r="{r}">')
            for c in sorted(self.rows[r].keys()):
                kind, val, style, formula = self.rows[r][c]
                ref = cell_ref(r, c)
                if kind == "str":
                    out.append(
                        f'<c r="{ref}" s="{style}" t="inlineStr"><is><t>{escape(val)}</t></is></c>'
                    )
                elif kind == "num":
                    out.append(f'<c r="{ref}" s="{style}"><v>{val:.6f}</v></c>')
                elif kind == "int":
                    out.append(f'<c r="{ref}" s="{style}"><v>{val}</v></c>')
                elif kind == "formula":
                    f_esc = escape(formula)
                    if isinstance(val, str):
                        out.append(
                            f'<c r="{ref}" s="{style}" t="str"><f>{f_esc}</f><v>{escape(val)}</v></c>'
                        )
                    else:
                        out.append(
                            f'<c r="{ref}" s="{style}"><f>{f_esc}</f><v>{float(val):.6f}</v></c>'
                        )
            out.append("</row>")
        out.append("</sheetData>")

        if self.merges:
            out.append(f'<mergeCells count="{len(self.merges)}">')
            for m in self.merges:
                out.append(f'<mergeCell ref="{m}"/>')
            out.append("</mergeCells>")

        out.append('<pageSetup orientation="landscape" paperSize="9"/>')
        out.append("</worksheet>")
        return "\n".join(out)


def build_styles_xml() -> str:
    # Style indices:
    # 0: Default text left, thin border
    # 1: Title bold 13pt, no border
    # 2: Table Header bold 11pt centered, wrap, blue-gray fill, thin border
    # 3: Subheader bold 11pt centered, light gray fill, thin border
    # 4: Number 0.0000 right, thin border
    # 5: Integer 0 centered, thin border
    # 6: Text centered, thin border
    # 7: Percent 0.00% right, thin border
    # 8: Highlighted final value bold 0.0000, soft cream/blue tint, thin border
    # 9: Note / caption italic 10pt left, no border
    return """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
  <numFmts count="2">
    <numFmt numFmtId="164" formatCode="0.0000"/>
    <numFmt numFmtId="165" formatCode="0.00%"/>
  </numFmts>
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
  <cellStyleXfs count="1">
    <xf numFmtId="0" fontId="0" fillId="0" borderId="0"/>
  </cellStyleXfs>
  <cellXfs count="10">
    <xf numFmtId="0" fontId="0" fillId="0" borderId="1" applyBorder="1" applyAlignment="1"><alignment horizontal="left" vertical="center" wrapText="1"/></xf>
    <xf numFmtId="0" fontId="1" fillId="0" borderId="0" applyFont="1" applyAlignment="1"><alignment horizontal="left" vertical="center"/></xf>
    <xf numFmtId="0" fontId="2" fillId="2" borderId="1" applyFont="1" applyFill="1" applyBorder="1" applyAlignment="1"><alignment horizontal="center" vertical="center" wrapText="1"/></xf>
    <xf numFmtId="0" fontId="2" fillId="3" borderId="1" applyFont="1" applyFill="1" applyBorder="1" applyAlignment="1"><alignment horizontal="center" vertical="center" wrapText="1"/></xf>
    <xf numFmtId="164" fontId="0" fillId="0" borderId="1" applyNumberFormat="1" applyBorder="1" applyAlignment="1"><alignment horizontal="right" vertical="center"/></xf>
    <xf numFmtId="1" fontId="0" fillId="0" borderId="1" applyNumberFormat="1" applyBorder="1" applyAlignment="1"><alignment horizontal="center" vertical="center"/></xf>
    <xf numFmtId="0" fontId="0" fillId="0" borderId="1" applyBorder="1" applyAlignment="1"><alignment horizontal="center" vertical="center" wrapText="1"/></xf>
    <xf numFmtId="165" fontId="0" fillId="0" borderId="1" applyNumberFormat="1" applyBorder="1" applyAlignment="1"><alignment horizontal="right" vertical="center"/></xf>
    <xf numFmtId="164" fontId="2" fillId="4" borderId="1" applyFont="1" applyFill="1" applyNumberFormat="1" applyBorder="1" applyAlignment="1"><alignment horizontal="right" vertical="center"/></xf>
    <xf numFmtId="0" fontId="3" fillId="0" borderId="0" applyFont="1" applyAlignment="1"><alignment horizontal="left" vertical="center"/></xf>
  </cellXfs>
</styleSheet>
"""


def write_xlsx(path: str, sheets: list):
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zf:
        # [Content_Types].xml
        ct_overrides = [
            '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>',
            '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>',
        ]
        for i in range(1, len(sheets) + 1):
            ct_overrides.append(
                f'<Override PartName="/xl/worksheets/sheet{i}.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>'
            )
        ct_xml = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">\n'
            '  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>\n'
            '  <Default Extension="xml" ContentType="application/xml"/>\n'
            + "\n".join("  " + x for x in ct_overrides)
            + "\n</Types>"
        )
        zf.writestr("[Content_Types].xml", ct_xml)

        # _rels/.rels
        zf.writestr(
            "_rels/.rels",
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">\n'
            '  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>\n'
            '</Relationships>',
        )

        # xl/workbook.xml
        sh_entries = []
        rel_entries = [
            '<Relationship Id="rIdStyles" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>'
        ]
        for i, sh in enumerate(sheets, start=1):
            sh_entries.append(
                f'<sheet name="{escape(sh.name)}" sheetId="{i}" r:id="rId{i}"/>'
            )
            rel_entries.append(
                f'<Relationship Id="rId{i}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet{i}.xml"/>'
            )
        wb_xml = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" '
            'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">\n'
            '  <sheets>\n    ' + "\n    ".join(sh_entries) + "\n  </sheets>\n</workbook>"
        )
        zf.writestr("xl/workbook.xml", wb_xml)

        rels_xml = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">\n'
            + "\n".join("  " + x for x in rel_entries)
            + "\n</Relationships>"
        )
        zf.writestr("xl/_rels/workbook.xml.rels", rels_xml)
        zf.writestr("xl/styles.xml", build_styles_xml())

        for i, sh in enumerate(sheets, start=1):
            zf.writestr(f"xl/worksheets/sheet{i}.xml", sh.to_xml())


def load_csv(path: str):
    with open(path, "r", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def estimate_f_amdahl(pairs):
    """Estimate sequential fraction f from list of (p, Sp) with p > 1 via least-squares on 1/Sp = f + (1-f)/p."""
    # Let x = 1 - 1/p, y = 1/Sp - 1/p => y = f * x => f = sum(x*y) / sum(x*x)
    num = 0.0
    den = 0.0
    for p, sp in pairs:
        if p <= 1 or sp <= 0:
            continue
        x = 1.0 - 1.0 / p
        y = (1.0 / sp) - (1.0 / p)
        num += x * y
        den += x * x
    if den <= 0:
        return 0.0
    return max(0.0, min(1.0, num / den))


def main():
    t31 = load_csv("results/task3_1_summary.csv")
    t32 = load_csv("results/task3_2_summary.csv")
    raw31 = load_csv("results/task3_1_raw_samples.csv")
    raw32 = load_csv("results/task3_2_raw_samples.csv")

    # Index task 3.1 by (dtype, n, algo, threads)
    idx31 = {}
    for row in t31:
        key = (row["dtype"], int(row["n"]), row["algo"], int(row["threads"]))
        idx31[key] = row

    # Index task 3.2 by (img_id, algo, ksize, threads)
    idx32 = {}
    resolutions = {}
    for row in t32:
        iid = int(row["img_id"])
        resolutions[iid] = row["resolution"]
        key = (iid, row["algo"], int(row["ksize"]), int(row["threads"]))
        idx32[key] = row

    # =========================================================================
    # Лист 1: Таблица 3.1 (формат методических указаний + Cp)
    # =========================================================================
    s1 = SimpleSheet("Таблица 3.1 (Сортировка)")
    s1.set_str(1, 1, "Таблица 3.1 – Результаты экспериментального исследования алгоритмов сортировки (Задание 3.1)", 1)
    s1.set_str(2, 1, "Метод фильтрации выбросов: AvgTrustedIntervalMed (доверительный интервал med ± SD); единицы времени: мс", 9)

    # Header rows 3-4
    s1.set_str(3, 1, "Функция сортировки", 2)
    s1.merge(3, 1, 4, 1)
    s1.set_str(3, 2, "Поток (p)", 2)
    s1.merge(3, 2, 4, 2)
    sizes = [20000, 35000, 50000]
    for si, sz in enumerate(sizes):
        c0 = 3 + si * 4
        s1.set_str(3, c0, f"НД{si+1}: {sz:,} эл.".replace(",", " "), 2)
        for k in range(1, 4):
            s1.set_str(3, c0 + k, "", 2)
        s1.merge(3, c0, 3, c0 + 3)
        s1.set_str(4, c0 + 0, "Время (мс)", 2)
        s1.set_str(4, c0 + 1, "Sp(n)", 2)
        s1.set_str(4, c0 + 2, "Ep(n)", 2)
        s1.set_str(4, c0 + 3, "Cp(мс)", 2)

    s1.set_col_width(1, 34)
    s1.set_col_width(2, 11)
    for c in range(3, 15):
        s1.set_col_width(c, 12.5)

    r = 5
    algo_specs = [
        ("Классический алгоритм пузырька (посл.)", "Bubble_Classic_Seq", 1, "Bubble_Classic_Seq"),
        ("Чет-нечетной перестановки (посл.)", "OddEven_Seq", 1, "OddEven_Seq"),
        ("Чет-нечетной перестановки (парал.)", "OddEven_Par", 2, "OddEven_Seq"),
        ("Чет-нечетной перестановки (парал.)", "OddEven_Par", 3, "OddEven_Seq"),
        ("Чет-нечетной перестановки (парал.)", "OddEven_Par", 4, "OddEven_Seq"),
        ("Сортировка Шелла (посл.)", "Shell_Seq", 1, "Shell_Seq"),
        ("Сортировка Шелла (парал.)", "Shell_Par", 2, "Shell_Seq"),
        ("Сортировка Шелла (парал.)", "Shell_Par", 3, "Shell_Seq"),
        ("Сортировка Шелла (парал.)", "Shell_Par", 4, "Shell_Seq"),
        ("Быстрая сортировка qsort (посл.)", "QuickSort_Seq", 1, "QuickSort_Seq"),
        ("Быстрая сортировка qsort (парал.)", "QuickSort_Par", 2, "QuickSort_Seq"),
        ("Быстрая сортировка qsort (парал.)", "QuickSort_Par", 3, "QuickSort_Seq"),
        ("Быстрая сортировка qsort (парал.)", "QuickSort_Par", 4, "QuickSort_Seq"),
    ]

    for dtype, dtype_label in [("int", "Тип данных: int"), ("double", "Тип данных: double")]:
        s1.set_str(r, 1, dtype_label, 3)
        for c in range(2, 15):
            s1.set_str(r, c, "", 3)
        s1.merge(r, 1, r, 14)
        r += 1

        seq_row_map = {}
        for label, algo, p, base_algo in algo_specs:
            if p == 1:
                seq_row_map[base_algo] = r
            s1.set_str(r, 1, label, 0)
            s1.set_int(r, 2, p, 5)
            base_r = seq_row_map[base_algo]
            for si, sz in enumerate(sizes):
                c0 = 3 + si * 4
                rec = idx31[(dtype, sz, algo, p)]
                t_ms = float(rec["trusted_avg_med_ms"])
                sp = float(rec["speedup"])
                ep = float(rec["efficiency"])
                cp = float(rec["cost_ms"])
                s1.set_num(r, c0, t_ms, 8 if p == 1 else 4)
                t1_ref = cell_ref(base_r, c0)
                tp_ref = cell_ref(r, c0)
                p_ref = cell_ref(r, 2)
                sp_ref = cell_ref(r, c0 + 1)
                s1.set_formula(r, c0 + 1, f"{t1_ref}/{tp_ref}", sp, 4)
                s1.set_formula(r, c0 + 2, f"{sp_ref}/{p_ref}", ep, 4)
                s1.set_formula(r, c0 + 3, f"{p_ref}*{tp_ref}", cp, 4)
            r += 1

    # =========================================================================
    # Лист 2: Расширенная таблица Задания 3.1 (1..32 потока + статистика ДИ)
    # =========================================================================
    s2 = SimpleSheet("Зад. 3.1 (Расшир. и ДИ)")
    s2.set_str(1, 1, "Полные результаты эксперимента и статистической обработки доверительного интервала (Задание 3.1)", 1)
    headers2 = [
        "Тип", "n (эл.)", "Алгоритм", "Потоки (p)", "Замеры (N)", "Пакет",
        "Исх. среднее (мс)", "Медиана (мс)", "SD (мс)", "Принято", "Отброшено",
        "Итог med±SD (мс)", "Сравн. avg±SD (мс)", "Sp(n)", "Ep(n)", "Cp(мс)"
    ]
    for ci, h in enumerate(headers2, start=1):
        s2.set_str(3, ci, h, 2)
    s2.set_col_width(3, 22)

    r = 4
    seq_rows2 = {}
    for row in t31:
        dt = row["dtype"]
        n = int(row["n"])
        algo = row["algo"]
        p = int(row["threads"])
        base_algo = algo.replace("_Par", "_Seq")
        if p == 1:
            seq_rows2[(dt, n, base_algo)] = r

        s2.set_str(r, 1, dt, 6)
        s2.set_int(r, 2, n, 5)
        s2.set_str(r, 3, algo, 0)
        s2.set_int(r, 4, p, 5)
        s2.set_int(r, 5, int(row["runs"]), 5)
        s2.set_int(r, 6, int(row["batch_size"]), 5)
        s2.set_num(r, 7, float(row["raw_avg_ms"]), 4)
        s2.set_num(r, 8, float(row["median_ms"]), 4)
        s2.set_num(r, 9, float(row["sd_ms"]), 4)
        s2.set_int(r, 10, int(row["kept_runs"]), 5)
        s2.set_int(r, 11, int(row["discarded_runs"]), 5)
        s2.set_num(r, 12, float(row["trusted_avg_med_ms"]), 8)
        s2.set_num(r, 13, float(row["trusted_avg_mean_ms"]), 4)

        b_r = seq_rows2[(dt, n, base_algo)]
        s2.set_formula(r, 14, f"{cell_ref(b_r, 12)}/{cell_ref(r, 12)}", float(row["speedup"]), 4)
        s2.set_formula(r, 15, f"{cell_ref(r, 14)}/{cell_ref(r, 4)}", float(row["efficiency"]), 4)
        s2.set_formula(r, 16, f"{cell_ref(r, 4)}*{cell_ref(r, 12)}", float(row["cost_ms"]), 4)
        r += 1

    # =========================================================================
    # Лист 3: Таблица 3.2 (формат методических указаний)
    # =========================================================================
    s3 = SimpleSheet("Таблица 3.2 (Медианный)")
    s3.set_str(1, 1, "Таблица 3.2 – Результаты исследования медианной фильтрации BMP-изображений (Задание 3.2)", 1)
    s3.set_str(2, 1, "Сортировка 1: метод Шелла (Shell); Сортировка 2: быстрая сортировка (QuickSort); единицы времени: мс", 9)

    s3.set_str(3, 1, "Функция (реализация)", 2); s3.merge(3, 1, 4, 1)
    s3.set_str(3, 2, "Поток (p)", 2);            s3.merge(3, 2, 4, 2)
    s3.set_str(3, 3, "Ksize", 2);                s3.merge(3, 3, 4, 3)

    for iid in [1, 2, 3, 4]:
        c0 = 4 + (iid - 1) * 2
        s3.set_str(3, c0, f"НД {iid} ({resolutions[iid]})", 2)
        s3.set_str(3, c0 + 1, "", 2)
        s3.merge(3, c0, 3, c0 + 1)
        s3.set_str(4, c0, "Время (мс)", 2)
        s3.set_str(4, c0 + 1, "Sp(n)", 2)

    s3.set_col_width(1, 34)
    s3.set_col_width(2, 11)
    s3.set_col_width(3, 9)
    for c in range(4, 12):
        s3.set_col_width(c, 13)

    r = 5
    for alg_code, alg_ru in [("Median_Shell", "Медианный фильтр — Шелла"), ("Median_QuickSort", "Медианный фильтр — QuickSort")]:
        seq_row_by_k = {}
        for p in [1, 2, 3, 4]:
            mode_lbl = f"{alg_ru} (посл.)" if p == 1 else f"{alg_ru} (парал.)"
            for ksz in [7, 11, 15]:
                if p == 1:
                    seq_row_by_k[ksz] = r
                s3.set_str(r, 1, mode_lbl, 0)
                s3.set_int(r, 2, p, 5)
                s3.set_int(r, 3, ksz, 5)
                base_r = seq_row_by_k[ksz]
                for iid in [1, 2, 3, 4]:
                    c0 = 4 + (iid - 1) * 2
                    rec = idx32[(iid, alg_code, ksz, p)]
                    t_ms = float(rec["trusted_avg_med_ms"])
                    sp = float(rec["speedup"])
                    s3.set_num(r, c0, t_ms, 8 if p == 1 else 4)
                    s3.set_formula(r, c0 + 1, f"{cell_ref(base_r, c0)}/{cell_ref(r, c0)}", sp, 4)
                r += 1

    # =========================================================================
    # Лист 4: Расширенная таблица Задания 3.2 (вкл. Ep, Cp и p=6,8,12,16 на НД4)
    # =========================================================================
    s4 = SimpleSheet("Зад. 3.2 (Расшир. и ДИ)")
    s4.set_str(1, 1, "Полные результаты медианной фильтрации и масштабируемость до 16 потоков (Задание 3.2)", 1)
    headers4 = [
        "НД", "Разрешение", "Пикселей", "Алгоритм", "Ksize", "Потоки (p)", "Замеры (N)",
        "Исх. среднее (мс)", "Медиана (мс)", "SD (мс)", "Принято", "Отброшено",
        "Итог med±SD (мс)", "Sp(n)", "Ep(n)", "Cp(мс)"
    ]
    for ci, h in enumerate(headers4, start=1):
        s4.set_str(3, ci, h, 2)
    s4.set_col_width(4, 20)

    r = 4
    seq_rows4 = {}
    for row in t32:
        iid = int(row["img_id"])
        algo = row["algo"]
        ksz = int(row["ksize"])
        p = int(row["threads"])
        if p == 1:
            seq_rows4[(iid, algo, ksz)] = r

        s4.set_str(r, 1, f"НД {iid}", 6)
        s4.set_str(r, 2, row["resolution"], 6)
        s4.set_int(r, 3, int(row["pixels"]), 5)
        s4.set_str(r, 4, algo, 0)
        s4.set_int(r, 5, ksz, 5)
        s4.set_int(r, 6, p, 5)
        s4.set_int(r, 7, int(row["runs"]), 5)
        s4.set_num(r, 8, float(row["raw_avg_ms"]), 4)
        s4.set_num(r, 9, float(row["median_ms"]), 4)
        s4.set_num(r, 10, float(row["sd_ms"]), 4)
        s4.set_int(r, 11, int(row["kept_runs"]), 5)
        s4.set_int(r, 12, int(row["discarded_runs"]), 5)
        s4.set_num(r, 13, float(row["trusted_avg_med_ms"]), 8)

        b_r = seq_rows4[(iid, algo, ksz)]
        s4.set_formula(r, 14, f"{cell_ref(b_r, 13)}/{cell_ref(r, 13)}", float(row["speedup"]), 4)
        s4.set_formula(r, 15, f"{cell_ref(r, 14)}/{cell_ref(r, 6)}", float(row["efficiency"]), 4)
        s4.set_formula(r, 16, f"{cell_ref(r, 6)}*{cell_ref(r, 13)}", float(row["cost_ms"]), 4)
        r += 1

    # =========================================================================
    # Лист 5: Доверительный интервал (сырые замеры + живые формулы Excel)
    # =========================================================================
    s5 = SimpleSheet("Доверительный интервал")
    s5.set_str(1, 1, "Проверка расчёта доверительного интервала (AvgTrustedIntervalMed: med ± SD) по сырым замерам", 1)

    # Extract two 110-run series: Task 3.1 QuickSort_Par (int, 50000, p=4) and Task 3.2 Median_Shell (НД1, K=7, p=4)
    ser1 = [x for x in raw31 if x["algo"] == "QuickSort_Par" and int(x["threads"]) == 4]
    ser2 = [x for x in raw32 if int(x["img_id"]) == 1 and x["algo"] == "Median_Shell" and int(x["ksize"]) == 7 and int(x["threads"]) == 4]

    def write_ci_block(sheet, start_col: int, title: str, series: list):
        sheet.set_str(3, start_col, title, 3)
        for k in range(1, 5):
            sheet.set_str(3, start_col + k, "", 3)
        sheet.merge(3, start_col, 3, start_col + 4)

        vals = [float(x["time_ms"]) for x in series]
        n_pts = len(vals)
        avg = sum(vals) / n_pts
        sd = math.sqrt(sum((v - avg) ** 2 for v in vals) / (n_pts - 1))
        sv = sorted(vals)
        med = 0.5 * (sv[n_pts // 2 - 1] + sv[n_pts // 2]) if n_pts % 2 == 0 else sv[n_pts // 2]
        kept_vals = [v for v in vals if med - sd <= v <= med + sd]
        trusted = sum(kept_vals) / max(1, len(kept_vals))

        first_data_r = 12
        last_data_r = first_data_r + n_pts - 1
        val_rng = f"{cell_ref(first_data_r, start_col + 1)}:{cell_ref(last_data_r, start_col + 1)}"
        flag_rng = f"{cell_ref(first_data_r, start_col + 4)}:{cell_ref(last_data_r, start_col + 4)}"

        summary_rows = [
            ("Число замеров N", n_pts, f"COUNT({val_rng})", 5),
            ("Исходное среднее (мс)", avg, f"AVERAGE({val_rng})", 4),
            ("Медиана med (мс)", med, f"MEDIAN({val_rng})", 4),
            ("Станд. отклонение SD (мс)", sd, f"STDEV({val_rng})", 4),
            ("Отброшено выбросов", n_pts - len(kept_vals), f'COUNTIF({flag_rng},0)', 5),
            ("Итог AvgTrustedIntervalMed (мс)", trusted, f'AVERAGEIF({flag_rng},1,{val_rng})', 8),
        ]
        for i_s, (lbl, cval, fml, st) in enumerate(summary_rows):
            rr = 4 + i_s
            sheet.set_str(rr, start_col, lbl, 0)
            sheet.set_str(rr, start_col + 1, "", 0)
            sheet.merge(rr, start_col, rr, start_col + 1)
            sheet.set_formula(rr, start_col + 2, fml, cval, st)
            sheet.set_str(rr, start_col + 3, "", 0)
            sheet.set_str(rr, start_col + 4, "", 0)

        med_cell = f"${col_letter(start_col + 2)}$6"
        sd_cell = f"${col_letter(start_col + 2)}$7"

        hdrs = ["№ замера", "Время Xi (мс)", "med - SD", "med + SD", "В интервале (1/0)"]
        for k, h in enumerate(hdrs):
            sheet.set_str(11, start_col + k, h, 2)

        for idx_v, v in enumerate(vals):
            rr = first_data_r + idx_v
            in_int = 1 if (med - sd <= v <= med + sd) else 0
            sheet.set_int(rr, start_col, idx_v + 1, 5)
            sheet.set_num(rr, start_col + 1, v, 4)
            sheet.set_formula(rr, start_col + 2, f"{med_cell}-{sd_cell}", med - sd, 4)
            sheet.set_formula(rr, start_col + 3, f"{med_cell}+{sd_cell}", med + sd, 4)
            vref = cell_ref(rr, start_col + 1)
            lref = cell_ref(rr, start_col + 2)
            rref = cell_ref(rr, start_col + 3)
            sheet.set_formula(rr, start_col + 4, f"IF(AND({vref}>={lref},{vref}<={rref}),1,0)", in_int, 5)

    write_ci_block(s5, 1, "Задание 3.1: QuickSort (int, n=50 000, p=4, N=110)", ser1)
    write_ci_block(s5, 7, "Задание 3.2: Медианный фильтр Шелла (480x270, K=7, p=4, N=110)", ser2)
    for c in [1, 7]:
        s5.set_col_width(c, 12)
        s5.set_col_width(c + 1, 15)
        s5.set_col_width(c + 2, 14)
        s5.set_col_width(c + 3, 14)
        s5.set_col_width(c + 4, 17)

    # =========================================================================
    # Лист 6: Сравнение с законами Амдала и Густавсона–Барсиса
    # =========================================================================
    s6 = SimpleSheet("Амдал и Густавсон")
    s6.set_str(1, 1, "Оценка доли последовательных вычислений f и сравнение с законами Амдала и Густавсона–Барсиса", 1)

    # Compute estimated f for key algorithms (using p <= 6 physical cores of Ryzen 5 5600)
    alg_models = [
        ("Задание 3.1: Shell (int, n=50 000)", [(p, float(idx31[("int", 50000, "Shell_Par" if p > 1 else "Shell_Seq", p)]["speedup"])) for p in [1, 2, 3, 4, 6, 8, 12, 16]]),
        ("Задание 3.1: QuickSort (int, n=50 000)", [(p, float(idx31[("int", 50000, "QuickSort_Par" if p > 1 else "QuickSort_Seq", p)]["speedup"])) for p in [1, 2, 3, 4, 6, 8, 12, 16]]),
        ("Задание 3.2: Медианный Шелла (1280x720, K=15)", [(p, float(idx32[(4, "Median_Shell", 15, p)]["speedup"])) for p in [1, 2, 3, 4, 6, 8, 12, 16]]),
        ("Задание 3.2: Медианный QuickSort (1280x720, K=15)", [(p, float(idx32[(4, "Median_QuickSort", 15, p)]["speedup"])) for p in [1, 2, 3, 4, 6, 8, 12, 16]]),
    ]

    s6.set_str(3, 1, "Алгоритм", 2)
    s6.set_str(3, 2, "Оценка f (p=2..6)", 2)
    s6.set_str(3, 3, "Предел Амдала S*=1/f", 2)
    s6.set_col_width(1, 44)
    s6.set_col_width(2, 18)
    s6.set_col_width(3, 20)

    r = 4
    f_map = {}
    for title, pts in alg_models:
        p_lim = 12 if "Задание 3.2" in title else 6
        f_est = estimate_f_amdahl([(p, sp) for p, sp in pts if 2 <= p <= p_lim])
        f_map[title] = (f_est, pts)
        s6.set_str(r, 1, title, 0)
        s6.set_num(r, 2, f_est, 4)
        s6.set_formula(r, 3, f"1/{cell_ref(r, 2)}", (1.0 / f_est) if f_est > 1e-6 else 999.0, 4)
        r += 1

    # Detailed table comparing Exp vs Amdahl vs Gustafson for p=1,2,3,4,6,8,12,16
    r += 1
    s6.set_str(r, 1, "Сравнение экспериментального ускорения Sp с теоретическими пределами (Амдал / Густавсон–Барсис)", 1)
    r += 1
    for title, (f_est, pts) in f_map.items():
        s6.set_str(r, 1, f"{title}  (оценка f = {f_est:.4f})", 3)
        for c in range(2, 6):
            s6.set_str(r, c, "", 3)
        s6.merge(r, 1, r, 5)
        r += 1
        for ci, h in enumerate(["Потоки p", "Sp (эксп.)", "Sp (Амдал)", "Sp (Густавсон–Барсис)", "Отклонение от Амдала"], start=1):
            s6.set_str(r, ci, h, 2)
        r += 1
        for p, sp_exp in pts:
            sp_amd = 1.0 / (f_est + (1.0 - f_est) / p)
            sp_gus = f_est + (1.0 - f_est) * p
            s6.set_int(r, 1, p, 5)
            s6.set_num(r, 2, sp_exp, 4)
            s6.set_formula(r, 3, f"1/({f_est:.6f}+(1-{f_est:.6f})/{cell_ref(r, 1)})", sp_amd, 4)
            s6.set_formula(r, 4, f"{f_est:.6f}+(1-{f_est:.6f})*{cell_ref(r, 1)}", sp_gus, 4)
            s6.set_formula(r, 5, f"{cell_ref(r, 2)}-{cell_ref(r, 3)}", sp_exp - sp_amd, 4)
            r += 1
        r += 1

    for c in range(4, 7):
        s6.set_col_width(c, 22)

    out_xlsx = "Результаты_исследования_Лаб3_OpenMP.xlsx"
    write_xlsx(out_xlsx, [s1, s2, s3, s4, s5, s6])
    print(f"Successfully generated {out_xlsx}")


if __name__ == "__main__":
    main()
