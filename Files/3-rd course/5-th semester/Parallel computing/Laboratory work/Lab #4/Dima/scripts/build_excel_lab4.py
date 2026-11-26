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
  <numFmts count="1">
    <numFmt numFmtId="164" formatCode="0.0000"/>
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
                    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
                    '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">\n'
                    '  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>\n'
                    '  <Default Extension="xml" ContentType="application/xml"/>\n'
                    + "\n".join("  " + x for x in ct) + "\n</Types>")
        zf.writestr("_rels/.rels",
                    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
                    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">\n'
                    '  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/>\n'
                    '</Relationships>')
        sh_e = [f'<sheet name="{escape(s.name)}" sheetId="{i}" r:id="rId{i}"/>' for i, s in enumerate(sheets, 1)]
        rel_e = ['<Relationship Id="rIdStyles" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>'] + [
            f'<Relationship Id="rId{i}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet{i}.xml"/>'
            for i in range(1, len(sheets) + 1)
        ]
        zf.writestr("xl/workbook.xml",
                    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
                    '<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" '
                    'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships">\n'
                    '  <sheets>\n    ' + "\n    ".join(sh_e) + "\n  </sheets>\n</workbook>")
        zf.writestr("xl/_rels/workbook.xml.rels",
                    '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
                    '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">\n'
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


def main():
    t41 = load_csv("Lab4/results/task4_1_summary.csv")
    t42 = load_csv("Lab4/results/task4_2_summary.csv")
    t43 = load_csv("Lab4/results/task4_3_summary.csv")
    raw41 = load_csv("Lab4/results/task4_1_raw_samples.csv")

    idx41 = {(int(r["ds_id"]), r["impl_name"], int(r["threads"])): r for r in t41}
    idx42 = {(int(r["img_id"]), r["impl_name"], int(r["ksize"]), int(r["threads"])): r for r in t42}
    idx43 = {(int(r["img_id"]), r["impl_name"], int(r["ksize"]), int(r["threads"])): r for r in t43}

    # =========================================================================
    # Лист 1: Таблица 4.1 (Матрицы)
    # =========================================================================
    s1 = SimpleSheet("Таблица 4.1 (Матрицы)")
    s1.set_str(1, 1, "Таблица 4.1 – Результаты обработки вещественных матриц (Задание 4.1, 100 запусков)", 1)
    s1.set_str(2, 1, "Метод фильтрации выбросов: AvgTrustedIntervalMed (med ± SD); единицы времени: мс", 9)

    s1.set_str(3, 1, "Функция (реализация)", 2); s1.merge(3, 1, 4, 1)
    s1.set_str(3, 2, "Поток (p)", 2);            s1.merge(3, 2, 4, 2)
    ds41_labels = ["НД 1 (3000×3000)", "НД 2 (5500×5500)", "НД 3 (8000×8000)"]
    for di, lbl in enumerate(ds41_labels):
        c0 = 3 + di * 4
        s1.set_str(3, c0, lbl, 2)
        for k in range(1, 4): s1.set_str(3, c0 + k, "", 2)
        s1.merge(3, c0, 3, c0 + 3)
        s1.set_str(4, c0 + 0, "Время (мс)", 2)
        s1.set_str(4, c0 + 1, "Sp(n)", 2)
        s1.set_str(4, c0 + 2, "Ep(n)", 2)
        s1.set_str(4, c0 + 3, "Cp(мс)", 2)

    s1.set_col_width(1, 38)
    s1.set_col_width(2, 11)
    for c in range(3, 15): s1.set_col_width(c, 12.5)

    groups_41 = [
        ("Сложение матриц: mas3[i][j] = mas1[i][j] + mas2[i][j]", "Add_Seq", [
            ("Сложение (посл.)", "Add_Seq", [1]),
            ("Сложение OpenMP FOR", "Add_OMP_For", [2, 3, 4]),
            ("Сложение OpenMP Sections", "Add_OMP_Sections", [2, 3, 4]),
            ("Сложение oneTBB task_group", "Add_TBB_TaskGroup", [2, 3, 4]),
            ("Сложение oneTBB parallel_for (лямбда)", "Add_TBB_ParFor_Lambda", [2, 3, 4]),
            ("Сложение oneTBB parallel_for (класс)", "Add_TBB_ParFor_Class", [2, 3, 4]),
        ]),
        ("Поэлементное перемножение: mas3[i][j] = mas1[i][j] × mas2[i][j]", "Mul_Seq", [
            ("Перемножение (посл.)", "Mul_Seq", [1]),
            ("Перемножение OpenMP FOR", "Mul_OMP_For", [2, 3, 4]),
            ("Перемножение OpenMP Sections", "Mul_OMP_Sections", [2, 3, 4]),
            ("Перемножение oneTBB task_group", "Mul_TBB_TaskGroup", [2, 3, 4]),
            ("Перемножение oneTBB parallel_for (лямбда)", "Mul_TBB_ParFor_Lambda", [2, 3, 4]),
            ("Перемножение oneTBB parallel_for (класс)", "Mul_TBB_ParFor_Class", [2, 3, 4]),
        ]),
        ("Редукция суммы всех элементов двух матриц (sumEl)", "Sum_Seq", [
            ("Сумма всех элементов (посл.)", "Sum_Seq", [1]),
            ("Сумма OpenMP FOR редукция", "Sum_OMP_For_Red", [2, 3, 4]),
            ("Сумма oneTBB редукция (Вариант 1 — класс)", "Sum_TBB_Red_Class", [2, 3, 4]),
            ("Сумма oneTBB редукция (Вариант 2 — лямбда)", "Sum_TBB_Red_Lambda", [2, 3, 4]),
        ]),
        ("Редукция разницы максимума и минимума (Val_range = maxEl - minEl)", "Range_Seq", [
            ("Val_range (посл.)", "Range_Seq", [1]),
            ("Val_range OpenMP FOR редукция", "Range_OMP_For_Red", [2, 3, 4]),
            ("Val_range oneTBB редукция (Вариант 1 — класс)", "Range_TBB_Red_Class", [2, 3, 4]),
            ("Val_range oneTBB редукция (Вариант 3 — структуры)", "Range_TBB_Red_Struct", [2, 3, 4]),
        ]),
    ]

    r = 5
    for g_title, seq_key, items in groups_41:
        s1.set_str(r, 1, g_title, 3)
        for c in range(2, 15): s1.set_str(r, c, "", 3)
        s1.merge(r, 1, r, 14)
        r += 1
        base_row = r
        for label, code, threads in items:
            for p in threads:
                s1.set_str(r, 1, label, 0)
                s1.set_int(r, 2, p, 5)
                for di in range(3):
                    c0 = 3 + di * 4
                    rec = idx41[(di + 1, code, p)]
                    t_ms = float(rec["trusted_avg_med_ms"])
                    s1.set_num(r, c0, t_ms, 8 if p == 1 else 4)
                    t1_ref = cell_ref(base_row, c0)
                    tp_ref = cell_ref(r, c0)
                    p_ref = cell_ref(r, 2)
                    sp_ref = cell_ref(r, c0 + 1)
                    s1.set_formula(r, c0 + 1, f"{t1_ref}/{tp_ref}", float(rec["speedup"]), 4)
                    s1.set_formula(r, c0 + 2, f"{sp_ref}/{p_ref}", float(rec["efficiency"]), 4)
                    s1.set_formula(r, c0 + 3, f"{p_ref}*{tp_ref}", float(rec["cost_ms"]), 4)
                r += 1

    # =========================================================================
    # Лист 2: Таблица 4.2 (Медианный фильтр OpenMP vs oneTBB)
    # =========================================================================
    s2 = SimpleSheet("Таблица 4.2 (Медианный)")
    s2.set_str(1, 1, "Таблица 4.2 – Медианная фильтрация BMP-изображений (OpenMP vs Intel oneTBB, 15 запусков)", 1)
    s2.set_str(2, 1, "Метод сортировки: 3-сторонний QuickSort (из ЛР №3); планарный формат RGB (SoA); время в мс", 9)

    s2.set_str(3, 1, "Функция (реализация)", 2); s2.merge(3, 1, 4, 1)
    s2.set_str(3, 2, "Поток (p)", 2);            s2.merge(3, 2, 4, 2)
    s2.set_str(3, 3, "Ksize", 2);                s2.merge(3, 3, 4, 3)

    ds42_lbl = ["НД 1 (1280×720)", "НД 2 (1920×1080)", "НД 3 (2560×1440)"]
    for di, lbl in enumerate(ds42_lbl):
        c0 = 4 + di * 4
        s2.set_str(3, c0, lbl, 2)
        for k in range(1, 4): s2.set_str(3, c0 + k, "", 2)
        s2.merge(3, c0, 3, c0 + 3)
        s2.set_str(4, c0 + 0, "Время (мс)", 2)
        s2.set_str(4, c0 + 1, "Sp(n)", 2)
        s2.set_str(4, c0 + 2, "Ep(n)", 2)
        s2.set_str(4, c0 + 3, "Cp(мс)", 2)

    s2.set_col_width(1, 32); s2.set_col_width(2, 10); s2.set_col_width(3, 8)
    for c in range(4, 16): s2.set_col_width(c, 12.5)

    r = 5
    seq_r42 = {}
    for label, code, threads in [
        ("Медианный фильтр (посл.)", "Median_Seq", [1]),
        ("Медианный фильтр OpenMP (парал.)", "Median_OpenMP", [2, 3, 4]),
        ("Медианный фильтр Intel oneTBB (парал.)", "Median_oneTBB", [2, 3, 4]),
    ]:
        for p in threads:
            for ksz in [5, 9, 13]:
                if p == 1: seq_r42[ksz] = r
                s2.set_str(r, 1, label, 0)
                s2.set_int(r, 2, p, 5)
                s2.set_int(r, 3, ksz, 5)
                br = seq_r42[ksz]
                for di in range(3):
                    c0 = 4 + di * 4
                    rec = idx42[(di + 1, code, ksz, p)]
                    t_ms = float(rec["trusted_avg_med_ms"])
                    s2.set_num(r, c0, t_ms, 8 if p == 1 else 4)
                    s2.set_formula(r, c0 + 1, f"{cell_ref(br, c0)}/{cell_ref(r, c0)}", float(rec["speedup"]), 4)
                    s2.set_formula(r, c0 + 2, f"{cell_ref(r, c0 + 1)}/{cell_ref(r, 2)}", float(rec["efficiency"]), 4)
                    s2.set_formula(r, c0 + 3, f"{cell_ref(r, 2)}*{cell_ref(r, c0)}", float(rec["cost_ms"]), 4)
                r += 1

    # =========================================================================
    # Лист 3: Таблица 4.3 (Текстурные признаки)
    # =========================================================================
    s3 = SimpleSheet("Таблица 4.3 (Текстуры)")
    s3.set_str(1, 1, "Таблица 4.3 – Расчёт текстурных признаков M2, R, U, E и построение карт (15 запусков)", 1)
    s3.set_str(2, 1, "Пороги: k1 = 0,30; k2 = 0,70; реализованы OpenMP, Intel oneTBB (1D) и доп. Intel oneTBB 2D (blocked_range2d)", 9)

    s3.set_str(3, 1, "Вид реализации функции", 2); s3.merge(3, 1, 4, 1)
    s3.set_str(3, 2, "Поток (p)", 2);              s3.merge(3, 2, 4, 2)
    s3.set_str(3, 3, "Окрестность", 2);            s3.merge(3, 3, 4, 3)

    for di, lbl in enumerate(["НД 1 (1280×720)", "НД 2 (2560×1440)"]):
        c0 = 4 + di * 4
        s3.set_str(3, c0, lbl, 2)
        for k in range(1, 4): s3.set_str(3, c0 + k, "", 2)
        s3.merge(3, c0, 3, c0 + 3)
        s3.set_str(4, c0 + 0, "Время (мс)", 2)
        s3.set_str(4, c0 + 1, "Sp(n)", 2)
        s3.set_str(4, c0 + 2, "Ep(n)", 2)
        s3.set_str(4, c0 + 3, "Cp(мс)", 2)

    s3.set_col_width(1, 34); s3.set_col_width(2, 10); s3.set_col_width(3, 12)
    for c in range(4, 12): s3.set_col_width(c, 13)

    r = 5
    seq_r43 = {}
    for label, code, threads in [
        ("Реализация последовательная (посл.)", "Texture_Seq", [1]),
        ("Вариант реализации OpenMP (парал.)", "Texture_OpenMP", [2, 3, 4]),
        ("Вариант Intel oneTBB 1D (парал.)", "Texture_oneTBB_1D", [2, 3, 4]),
        ("Вариант Intel oneTBB 2D blocked_range2d", "Texture_oneTBB_2D", [2, 3, 4]),
    ]:
        for p in threads:
            for ksz in [5, 7, 9]:
                if p == 1: seq_r43[ksz] = r
                s3.set_str(r, 1, label, 0)
                s3.set_int(r, 2, p, 5)
                s3.set_str(r, 3, f"{ksz}×{ksz}", 6)
                br = seq_r43[ksz]
                for di in range(2):
                    c0 = 4 + di * 4
                    rec = idx43[(di + 1, code, ksz, p)]
                    t_ms = float(rec["trusted_avg_med_ms"])
                    s3.set_num(r, c0, t_ms, 8 if p == 1 else 4)
                    s3.set_formula(r, c0 + 1, f"{cell_ref(br, c0)}/{cell_ref(r, c0)}", float(rec["speedup"]), 4)
                    s3.set_formula(r, c0 + 2, f"{cell_ref(r, c0 + 1)}/{cell_ref(r, 2)}", float(rec["efficiency"]), 4)
                    s3.set_formula(r, c0 + 3, f"{cell_ref(r, 2)}*{cell_ref(r, c0)}", float(rec["cost_ms"]), 4)
                r += 1

    # =========================================================================
    # Лист 4: Доверительный интервал (сырые 100 замеров + живые формулы)
    # =========================================================================
    s4 = SimpleSheet("Доверительный интервал")
    s4.set_str(1, 1, "Проверка расчёта доверительного интервала (AvgTrustedIntervalMed: med ± SD, 100 замеров)", 1)

    ser1 = [x for x in raw41 if x["impl_name"] == "Add_TBB_ParFor_Lambda"]
    ser2 = [x for x in raw41 if x["impl_name"] == "Range_TBB_Red_Struct"]

    def write_ci(sheet, sc: int, title: str, ser: list):
        sheet.set_str(3, sc, title, 3)
        for k in range(1, 5): sheet.set_str(3, sc + k, "", 3)
        sheet.merge(3, sc, 3, sc + 4)

        vals = [float(x["time_ms"]) for x in ser]
        n = len(vals)
        avg = sum(vals) / n
        sd = math.sqrt(sum((v - avg) ** 2 for v in vals) / (n - 1))
        sv = sorted(vals)
        med = 0.5 * (sv[n // 2 - 1] + sv[n // 2]) if n % 2 == 0 else sv[n // 2]
        kept = [v for v in vals if med - sd <= v <= med + sd]
        trusted = sum(kept) / max(1, len(kept))

        r0, r1 = 12, 12 + n - 1
        vr = f"{cell_ref(r0, sc + 1)}:{cell_ref(r1, sc + 1)}"
        fr = f"{cell_ref(r0, sc + 4)}:{cell_ref(r1, sc + 4)}"
        rows_s = [
            ("Число замеров N", n, f"COUNT({vr})", 5),
            ("Исходное среднее (мс)", avg, f"AVERAGE({vr})", 4),
            ("Медиана med (мс)", med, f"MEDIAN({vr})", 4),
            ("Станд. отклонение SD (мс)", sd, f"STDEV({vr})", 4),
            ("Отброшено выбросов", n - len(kept), f"COUNTIF({fr},0)", 5),
            ("Итог AvgTrustedIntervalMed (мс)", trusted, f"AVERAGEIF({fr},1,{vr})", 8),
        ]
        for idx_s, (lb, cv, fm, st) in enumerate(rows_s):
            rr = 4 + idx_s
            sheet.set_str(rr, sc, lb, 0); sheet.set_str(rr, sc + 1, "", 0)
            sheet.merge(rr, sc, rr, sc + 1)
            sheet.set_formula(rr, sc + 2, fm, cv, st)
            sheet.set_str(rr, sc + 3, "", 0); sheet.set_str(rr, sc + 4, "", 0)

        mc = f"${col_letter(sc + 2)}$6"; sdc = f"${col_letter(sc + 2)}$7"
        for k, h in enumerate(["№ замера", "Время Xi (мс)", "med - SD", "med + SD", "В интервале (1/0)"]):
            sheet.set_str(11, sc + k, h, 2)
        for idx_v, v in enumerate(vals):
            rr = r0 + idx_v
            in_i = 1 if (med - sd <= v <= med + sd) else 0
            sheet.set_int(rr, sc, idx_v + 1, 5)
            sheet.set_num(rr, sc + 1, v, 4)
            sheet.set_formula(rr, sc + 2, f"{mc}-{sdc}", med - sd, 4)
            sheet.set_formula(rr, sc + 3, f"{mc}+{sdc}", med + sd, 4)
            sheet.set_formula(rr, sc + 4, f"IF(AND({cell_ref(rr,sc+1)}>={cell_ref(rr,sc+2)},{cell_ref(rr,sc+1)}<={cell_ref(rr,sc+3)}),1,0)", in_i, 5)

    write_ci(s4, 1, "Зад. 4.1: Сложение oneTBB parallel_for (8000×8000, p=4, N=100)", ser1)
    write_ci(s4, 7, "Зад. 4.1: Val_range oneTBB структуры (8000×8000, p=4, N=100)", ser2)
    for c in [1, 7]:
        s4.set_col_width(c, 12); s4.set_col_width(c+1, 15); s4.set_col_width(c+2, 14); s4.set_col_width(c+3, 14); s4.set_col_width(c+4, 17)

    # =========================================================================
    # Лист 5: Законы Амдала и Густавсона–Барсиса
    # =========================================================================
    s5 = SimpleSheet("Амдал и Густавсон")
    s5.set_str(1, 1, "Оценка доли последовательных вычислений f и сравнение с законами Амдала и Густавсона–Барсиса", 1)

    models = [
        ("Зад. 4.1: Сложение oneTBB parallel_for (8000×8000)", [(p, float(idx41[(3, "Add_Seq" if p==1 else "Add_TBB_ParFor_Lambda", p)]["speedup"])) for p in [1,2,3,4]]),
        ("Зад. 4.1: Сумма oneTBB редукция класс (8000×8000)", [(p, float(idx41[(3, "Sum_Seq" if p==1 else "Sum_TBB_Red_Class", p)]["speedup"])) for p in [1,2,3,4]]),
        ("Зад. 4.1: Val_range oneTBB структуры (8000×8000)",  [(p, float(idx41[(3, "Range_Seq" if p==1 else "Range_TBB_Red_Struct", p)]["speedup"])) for p in [1,2,3,4]]),
        ("Зад. 4.2: Медианный OpenMP (2560×1440, K=13)",      [(p, float(idx42[(3, "Median_Seq" if p==1 else "Median_OpenMP", 13, p)]["speedup"])) for p in [1,2,3,4]]),
        ("Зад. 4.2: Медианный oneTBB (2560×1440, K=13)",      [(p, float(idx42[(3, "Median_Seq" if p==1 else "Median_oneTBB", 13, p)]["speedup"])) for p in [1,2,3,4]]),
        ("Зад. 4.3: Текстуры OpenMP (2560×1440, 9×9)",        [(p, float(idx43[(2, "Texture_Seq" if p==1 else "Texture_OpenMP", 9, p)]["speedup"])) for p in [1,2,3,4]]),
        ("Зад. 4.3: Текстуры oneTBB 1D (2560×1440, 9×9)",     [(p, float(idx43[(2, "Texture_Seq" if p==1 else "Texture_oneTBB_1D", 9, p)]["speedup"])) for p in [1,2,3,4]]),
        ("Зад. 4.3: Текстуры oneTBB 2D (2560×1440, 9×9)",     [(p, float(idx43[(2, "Texture_Seq" if p==1 else "Texture_oneTBB_2D", 9, p)]["speedup"])) for p in [1,2,3,4]]),
    ]

    for ci, h in enumerate(["Алгоритм", "Оценка f", "S* = 1/f", "Sp(4) эксп.", "Амдал (4)", "Густавсон (4)"], 1):
        s5.set_str(3, ci, h, 2)
    s5.set_col_width(1, 46)
    for c in range(2, 7): s5.set_col_width(c, 16)

    r = 4
    for title, pts in models:
        f_est = estimate_f(pts)
        sp4 = dict(pts)[4]
        s_lim = (1.0 / f_est) if f_est > 1e-5 else 999.0
        amd4 = 1.0 / (f_est + (1.0 - f_est) / 4.0)
        gus4 = f_est + (1.0 - f_est) * 4.0
        s5.set_str(r, 1, title, 0)
        s5.set_num(r, 2, f_est, 4)
        s5.set_formula(r, 3, f"IF({cell_ref(r,2)}>0.0001,1/{cell_ref(r,2)},999)", s_lim, 4)
        s5.set_num(r, 4, sp4, 4)
        s5.set_formula(r, 5, f"1/({cell_ref(r,2)}+(1-{cell_ref(r,2)})/4)", amd4, 4)
        s5.set_formula(r, 6, f"{cell_ref(r,2)}+(1-{cell_ref(r,2)})*4", gus4, 4)
        r += 1

    out_path = "Lab4/Результаты_исследования_Лаб4_oneTBB.xlsx"
    write_xlsx(out_path, [s1, s2, s3, s4, s5])
    print(f"Successfully generated {out_path}")


if __name__ == "__main__":
    main()
