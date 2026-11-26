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
                    self.col_widths[c] = min(max(est, 9), 42)
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
                    if isinstance(val, str):
                        out.append(f'<c r="{ref}" s="{style}" t="str"><f>{escape(formula)}</f><v>{escape(val)}</v></c>')
                    else:
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


def write_xlsx(path: str, sheets):
    styles_xml = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
  <numFmts count="3">
    <numFmt numFmtId="164" formatCode="0.0000"/>
    <numFmt numFmtId="165" formatCode="0.000"/>
    <numFmt numFmtId="166" formatCode="0"/>
  </numFmts>
  <fonts count="4">
    <font><sz val="11"/><name val="Times New Roman"/></font>
    <font><b/><sz val="11"/><name val="Times New Roman"/></font>
    <font><b/><sz val="13"/><name val="Times New Roman"/></font>
    <font><b/><sz val="11"/><color rgb="FF1F4E78"/><name val="Times New Roman"/></font>
  </fonts>
  <fills count="4">
    <fill><patternFill patternType="none"/></fill>
    <fill><patternFill patternType="gray125"/></fill>
    <fill><patternFill patternType="solid"><fgColor rgb="FFD9E1F2"/><bgColor indexed="64"/></patternFill></fill>
    <fill><patternFill patternType="solid"><fgColor rgb="FFF2F2F2"/><bgColor indexed="64"/></patternFill></fill>
  </fills>
  <borders count="2">
    <border><left/><right/><top/><bottom/><diagonal/></border>
    <border>
      <left style="thin"><color auto="1"/></left>
      <right style="thin"><color auto="1"/></right>
      <top style="thin"><color auto="1"/></top>
      <bottom style="thin"><color auto="1"/></bottom>
      <diagonal/>
    </border>
  </borders>
  <cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
  <cellXfs count="9">
    <xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/>
    <xf numFmtId="0" fontId="2" fillId="0" borderId="0" xfId="0"><alignment horizontal="left" vertical="center"/></xf>
    <xf numFmtId="0" fontId="1" fillId="2" borderId="1" xfId="0"><alignment horizontal="center" vertical="center" wrapText="1"/></xf>
    <xf numFmtId="0" fontId="0" fillId="0" borderId="1" xfId="0"><alignment horizontal="left" vertical="center"/></xf>
    <xf numFmtId="164" fontId="0" fillId="0" borderId="1" xfId="0"><alignment horizontal="right" vertical="center"/></xf>
    <xf numFmtId="166" fontId="0" fillId="0" borderId="1" xfId="0"><alignment horizontal="center" vertical="center"/></xf>
    <xf numFmtId="0" fontId="3" fillId="3" borderId="1" xfId="0"><alignment horizontal="left" vertical="center"/></xf>
    <xf numFmtId="165" fontId="1" fillId="3" borderId="1" xfId="0"><alignment horizontal="right" vertical="center"/></xf>
    <xf numFmtId="0" fontId="1" fillId="0" borderId="1" xfId="0"><alignment horizontal="left" vertical="center"/></xf>
  </cellXfs>
</styleSheet>"""

    wb_sheets = []
    wb_rels = []
    ct_ov = [
        '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>',
        '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>',
    ]
    for i, sh in enumerate(sheets, 1):
        wb_sheets.append(f'<sheet name="{escape(sh.name)}" sheetId="{i}" r:id="rId{i}"/>')
        wb_rels.append(f'<Relationship Id="rId{i}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet{i}.xml"/>')
        ct_ov.append(f'<Override PartName="/xl/worksheets/sheet{i}.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>')
    wb_rels.append(f'<Relationship Id="rId{len(sheets)+1}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>')

    wb_xml = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"><sheets>' + "".join(wb_sheets) + '</sheets></workbook>'
    wb_rels_xml = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">' + "".join(wb_rels) + '</Relationships>'
    ct_xml = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types"><Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/><Default Extension="xml" ContentType="application/xml"/>' + "".join(ct_ov) + '</Types>'
    root_rels = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships"><Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/></Relationships>'

    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zf:
        zf.writestr("[Content_Types].xml", ct_xml)
        zf.writestr("_rels/.rels", root_rels)
        zf.writestr("xl/workbook.xml", wb_xml)
        zf.writestr("xl/_rels/workbook.xml.rels", wb_rels_xml)
        zf.writestr("xl/styles.xml", styles_xml)
        for i, sh in enumerate(sheets, 1):
            zf.writestr(f"xl/worksheets/sheet{i}.xml", sh.to_xml())


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
    t61 = list(csv.DictReader(open("Lab6/results/lab6_table61.csv", encoding="utf-8")))
    t62 = list(csv.DictReader(open("Lab6/results/lab6_table62.csv", encoding="utf-8")))
    raw = list(csv.DictReader(open("Lab6/results/lab6_raw_samples.csv", encoding="utf-8")))

    idx61 = {(int(r["ds_id"]), r["step_id"], r["backend"], r["container"], int(r["threads"])): r for r in t61}
    idx62 = {(int(r["ds_id"]), r["impl_id"], int(r["threads"])): r for r in t62}

    # ---------------- ЛИСТ 1: Таблица 6.1 (По функциональным блокам) ----------------
    sh1 = SimpleSheet("Табл. 6.1 (По блокам)")
    sh1.set_str(1, 1, "Таблица 6.1 – Время выполнения (мс) и ускорение Sp(n) по функциональным блокам детектора Харриса (OpenMP и oneTBB)", 1)
    sh1.merge(1, 1, 1, 14)

    sh1.set_str(3, 1, "Функциональный блок / вид реализации", 2); sh1.merge(3, 1, 4, 1)
    sh1.set_str(3, 2, "Поток", 2); sh1.merge(3, 2, 4, 2)
    ds_labels = [(1, "НД 1 (1280×720)"), (2, "НД 2 (1920×1080)"), (3, "НД 3 (2560×1440)")]
    for di, (_, dname) in enumerate(ds_labels):
        c0 = 3 + di * 4
        sh1.set_str(3, c0, dname, 2); sh1.merge(3, c0, 3, c0 + 3)
        for oi, hdr in enumerate(["Время, мс", "Sp(n)", "Ep(n)", "Cp, мс"]):
            sh1.set_str(4, c0 + oi, hdr, 2)

    sh1.set_col_width(1, 42)
    sh1.set_col_width(2, 8)

    step_defs = [
        ("Step1_GrayGauss", "Шаг 1: Яркость + сглаживание Гаусса (2×1D)", False),
        ("Step2_Gradients", "Шаг 2: Вычисление градиентов DiffX, DiffY, DiffXY", False),
        ("Step3_ResponseR", "Шаг 3: Вычисление отклика угла R (массив H)", False),
        ("Step4_1_Collect", "Шаг 4.1: Поиск min/max, threshold и сбор HarrisPoints", True),
        ("Step4_2_Sort",    "Шаг 4.2: Сортировка HarrisPoints по убыванию R", True),
        ("Step4_3_NMS",     "Шаг 4.3: Отсечение точек в радиусе rang = 20", True),
        ("Step5_Visualize", "Шаг 5: Визуализация особых точек (rang = 1)", False),
    ]

    # Сохраняем номера строк в Табл. 6.1, чтобы Табл. 6.2 ссылалась на них формулами!
    row_map_61 = {}
    r = 5
    for sid, stitle, has_cont in step_defs:
        sh1.set_str(r, 1, stitle, 6); sh1.merge(r, 1, r, 14)
        r += 1

        if not has_cont:
            variants = [
                ("Seq",    "None", 1, f"{sid} — последовательная (посл.)"),
                ("OpenMP", "None", 2, f"{sid} — OpenMP"),
                ("OpenMP", "None", 3, f"{sid} — OpenMP"),
                ("OpenMP", "None", 4, f"{sid} — OpenMP"),
                ("oneTBB", "None", 2, f"{sid} — Intel oneTBB"),
                ("oneTBB", "None", 3, f"{sid} — Intel oneTBB"),
                ("oneTBB", "None", 4, f"{sid} — Intel oneTBB"),
            ]
        else:
            variants = [
                ("Seq",    "STL",    1, f"{sid} — последовательная (STL vector)"),
                ("OpenMP", "STL",    2, f"{sid} — OpenMP (STL vector)"),
                ("OpenMP", "STL",    3, f"{sid} — OpenMP (STL vector)"),
                ("OpenMP", "STL",    4, f"{sid} — OpenMP (STL vector)"),
                ("oneTBB", "STL",    2, f"{sid} — Intel oneTBB (STL vector)"),
                ("oneTBB", "STL",    3, f"{sid} — Intel oneTBB (STL vector)"),
                ("oneTBB", "STL",    4, f"{sid} — Intel oneTBB (STL vector)"),
                ("OpenMP", "TBBVec", 2, f"{sid} — OpenMP (oneTBB concurrent_vector)"),
                ("OpenMP", "TBBVec", 3, f"{sid} — OpenMP (oneTBB concurrent_vector)"),
                ("OpenMP", "TBBVec", 4, f"{sid} — OpenMP (oneTBB concurrent_vector)"),
                ("oneTBB", "TBBVec", 2, f"{sid} — Intel oneTBB (oneTBB concurrent_vector)"),
                ("oneTBB", "TBBVec", 3, f"{sid} — Intel oneTBB (oneTBB concurrent_vector)"),
                ("oneTBB", "TBBVec", 4, f"{sid} — Intel oneTBB (oneTBB concurrent_vector)"),
            ]

        seq_row = r
        for be, cont, p, lbl in variants:
            row_map_61[(sid, be, cont, p)] = r
            sh1.set_str(r, 1, lbl, 3)
            sh1.set_int(r, 2, p, 5)
            for di, (ds_id, _) in enumerate(ds_labels):
                c0 = 3 + di * 4
                rec = idx61[(ds_id, sid, be, cont, p)]
                t_val = float(rec["trusted_avg_med_ms"])
                sp_val = float(rec["speedup"])
                ep_val = float(rec["efficiency"])
                cp_val = float(rec["cost_ms"])
                sh1.set_num(r, c0, t_val, 4)
                t_ref = cell_ref(r, c0)
                t1_ref = cell_ref(seq_row, c0)
                sp_ref = cell_ref(r, c0 + 1)
                p_ref = cell_ref(r, 2)
                sh1.set_formula(r, c0 + 1, f"{t1_ref}/{t_ref}", sp_val, 4)
                sh1.set_formula(r, c0 + 2, f"{sp_ref}/{p_ref}", ep_val, 4)
                sh1.set_formula(r, c0 + 3, f"{p_ref}*{t_ref}", cp_val, 4)
            r += 1

    # ---------------- ЛИСТ 2: Таблица 6.2 (Суммарное время формирования карты) ----------------
    sh2 = SimpleSheet("Табл. 6.2 (Итого)")
    sh2.set_str(1, 1, "Таблица 6.2 – Общее время формирования карты особых точек (сумма по данным Таблицы 6.1) и ускорение Sp(n)", 1)
    sh2.merge(1, 1, 1, 14)

    sh2.set_str(3, 1, "Вид полной реализации конвейера Харриса", 2); sh2.merge(3, 1, 4, 1)
    sh2.set_str(3, 2, "Поток", 2); sh2.merge(3, 2, 4, 2)
    for di, (_, dname) in enumerate(ds_labels):
        c0 = 3 + di * 4
        sh2.set_str(3, c0, dname, 2); sh2.merge(3, c0, 3, c0 + 3)
        for oi, hdr in enumerate(["Время, мс", "Sp(n)", "Ep(n)", "Cp, мс"]):
            sh2.set_str(4, c0 + oi, hdr, 2)
    sh2.set_col_width(1, 44)
    sh2.set_col_width(2, 8)

    t62_specs = [
        ("Seq",           "Реализация последовательная (посл.)",                      [1],       ("Seq", "Seq", "STL")),
        ("OpenMP_STL",    "OpenMP (парал.), Шаг 4: STL vector",                       [2, 3, 4], ("OpenMP", "OpenMP", "STL")),
        ("OpenMP_TBBVec", "OpenMP (парал.), Шаг 4: потокобезопасный oneTBB",          [2, 3, 4], ("OpenMP", "OpenMP", "TBBVec")),
        ("oneTBB_STL",    "Intel oneTBB (парал.), Шаг 4: STL vector",                 [2, 3, 4], ("oneTBB", "oneTBB", "STL")),
        ("oneTBB_TBBVec", "Intel oneTBB (парал.), Шаг 4: потокобезопасный oneTBB",    [2, 3, 4], ("oneTBB", "oneTBB", "TBBVec")),
        ("Hybrid_STL",    "Intel oneTBB + OpenMP (гибрид), Шаг 4: STL vector",        [2, 3, 4], ("OpenMP", "oneTBB", "STL")),
        ("Hybrid_TBBVec", "Intel oneTBB + OpenMP (гибрид), Шаг 4: поток. oneTBB",     [2, 3, 4], ("OpenMP", "oneTBB", "TBBVec")),
    ]

    r = 5
    seq_row_62 = 5
    sh1_q = "'Табл. 6.1 (По блокам)'"
    for iid, iname, plist, (be_reg, be_s4, cont_s4) in t62_specs:
        for p in plist:
            sh2.set_str(r, 1, iname, 3)
            sh2.set_int(r, 2, p, 5)
            rows_to_sum = [
                row_map_61[("Step1_GrayGauss", be_reg, "None", p)],
                row_map_61[("Step2_Gradients", be_reg, "None", p)],
                row_map_61[("Step3_ResponseR", be_reg, "None", p)],
                row_map_61[("Step4_1_Collect", be_s4,  cont_s4, p)],
                row_map_61[("Step4_2_Sort",    be_s4,  cont_s4, p)],
                row_map_61[("Step4_3_NMS",     be_s4,  cont_s4, p)],
                row_map_61[("Step5_Visualize", be_reg, "None", p)],
            ]
            for di, (ds_id, _) in enumerate(ds_labels):
                c0 = 3 + di * 4
                rec = idx62[(ds_id, iid, p)]
                tot_val = float(rec["total_ms"])
                sp_val = float(rec["speedup"])
                ep_val = float(rec["efficiency"])
                cp_val = float(rec["cost_ms"])
                sum_expr = "+".join(f"{sh1_q}!{cell_ref(rr, c0)}" for rr in rows_to_sum)
                sh2.set_formula(r, c0, sum_expr, tot_val, 4)
                t_ref = cell_ref(r, c0)
                t1_ref = cell_ref(seq_row_62, c0)
                sp_ref = cell_ref(r, c0 + 1)
                p_ref = cell_ref(r, 2)
                sh2.set_formula(r, c0 + 1, f"{t1_ref}/{t_ref}", sp_val, 4)
                sh2.set_formula(r, c0 + 2, f"{sp_ref}/{p_ref}", ep_val, 4)
                sh2.set_formula(r, c0 + 3, f"{p_ref}*{t_ref}", cp_val, 4)
            r += 1

    # Сводка числа найденных особых точек
    r += 1
    sh2.set_str(r, 1, "Проверка корректности: число найденных особых точек (после порога / итоговых после rang=20)", 6)
    sh2.merge(r, 1, r, 14); r += 1
    for ds_id, dname in ds_labels:
        rec_seq = idx62[(ds_id, "Seq", 1)]
        rec_tbb = idx62[(ds_id, "oneTBB_TBBVec", 4)]
        sh2.set_str(r, 1, dname, 3)
        sh2.set_str(r, 2, "Точки", 5)
        sh2.set_str(r, 3, f"После порога: {rec_seq['pts_after_thr']}", 3); sh2.merge(r, 3, r, 6)
        sh2.set_str(r, 7, f"Итого STL: {rec_seq['pts_final']}", 3); sh2.merge(r, 7, r, 10)
        sh2.set_str(r, 11, f"Итого oneTBB conc_vec: {rec_tbb['pts_final']}", 3); sh2.merge(r, 11, r, 14)
        r += 1

    # ---------------- ЛИСТ 3: STL vs concurrent_vector (Шаг 4) ----------------
    sh3 = SimpleSheet("STL vs concurrent_vector")
    sh3.set_str(1, 1, "Сравнение классического контейнера STL (std::vector) и потокобезопасного oneapi::tbb::concurrent_vector на Шаге 4 (p = 4)", 1)
    sh3.merge(1, 1, 1, 8)
    hdrs3 = ["Набор данных", "Подшаг (p = 4)", "OpenMP STL, мс", "OpenMP conc_vec, мс", "Отн. OpenMP", "oneTBB STL, мс", "oneTBB conc_vec, мс", "Отн. oneTBB"]
    for ci, h in enumerate(hdrs3, 1):
        sh3.set_str(3, ci, h, 2)
    sh3.set_col_width(1, 20); sh3.set_col_width(2, 26)
    r = 4
    for ds_id, dname in ds_labels:
        for sid, sname in [("Step4_1_Collect", "Шаг 4.1 (порог и сбор)"),
                           ("Step4_2_Sort",    "Шаг 4.2 (сортировка)"),
                           ("Step4_3_NMS",     "Шаг 4.3 (отсечение rang=20)")]:
            os_v = float(idx61[(ds_id, sid, "OpenMP", "STL", 4)]["trusted_avg_med_ms"])
            ot_v = float(idx61[(ds_id, sid, "OpenMP", "TBBVec", 4)]["trusted_avg_med_ms"])
            ts_v = float(idx61[(ds_id, sid, "oneTBB", "STL", 4)]["trusted_avg_med_ms"])
            tt_v = float(idx61[(ds_id, sid, "oneTBB", "TBBVec", 4)]["trusted_avg_med_ms"])
            sh3.set_str(r, 1, dname, 3)
            sh3.set_str(r, 2, sname, 3)
            sh3.set_num(r, 3, os_v, 4)
            sh3.set_num(r, 4, ot_v, 4)
            sh3.set_formula(r, 5, f"C{r}/D{r}", os_v / ot_v if ot_v > 0 else 1.0, 4)
            sh3.set_num(r, 6, ts_v, 4)
            sh3.set_num(r, 7, tt_v, 4)
            sh3.set_formula(r, 8, f"F{r}/G{r}", ts_v / tt_v if tt_v > 0 else 1.0, 4)
            r += 1
        # Сумма Шага 4
        sh3.set_str(r, 1, dname, 8)
        sh3.set_str(r, 2, "Итого Шаг 4 (4.1+4.2+4.3)", 8)
        for ci in [3, 4, 6, 7]:
            col = col_letter(ci)
            val = sum(sh3.rows[rr][ci][1] for rr in range(r - 3, r))
            sh3.set_formula(r, ci, f"SUM({col}{r-3}:{col}{r-1})", val, 7)
        sh3.set_formula(r, 5, f"C{r}/D{r}", sh3.rows[r][3][1] / sh3.rows[r][4][1], 7)
        sh3.set_formula(r, 8, f"F{r}/G{r}", sh3.rows[r][6][1] / sh3.rows[r][7][1], 7)
        r += 1

    # ---------------- ЛИСТ 4: Доверительный интервал (100 сырых замеров) ----------------
    sh4 = SimpleSheet("Доверительный интервал")
    sh4.set_str(1, 1, "Проверка доверительного интервала AvgTrustedIntervalMed (Шаг 2: градиенты oneTBB, НД 3 2560×1440, p = 4, N = 100)", 1)
    sh4.merge(1, 1, 1, 6)
    for ci, h in enumerate(["№ замера", "Время, мс", "В интервале?", "Принятое, мс"], 1):
        sh4.set_str(3, ci, h, 2)

    vals = [float(x["time_ms"]) for x in raw]
    n_raw = len(vals)
    svals = sorted(vals)
    med = svals[n_raw // 2] if n_raw % 2 == 1 else 0.5 * (svals[n_raw // 2 - 1] + svals[n_raw // 2])
    raw_avg = sum(vals) / n_raw
    sd = math.sqrt(sum((v - raw_avg) ** 2 for v in vals) / (n_raw - 1))
    lo, hi = med - sd, med + sd
    kept = [v for v in vals if lo <= v <= hi]
    clean_avg = sum(kept) / len(kept)

    last_r = 3 + n_raw
    for i, v in enumerate(vals, 1):
        rr = 3 + i
        ok = 1 if (lo <= v <= hi) else 0
        sh4.set_int(rr, 1, i, 5)
        sh4.set_num(rr, 2, v, 4)
        sh4.set_formula(rr, 3, f"IF(AND(B{rr}>=$G$7,B{rr}<=$G$8),1,0)", ok, 5)
        sh4.set_formula(rr, 4, f"IF(C{rr}=1,B{rr},0)", v if ok else 0.0, 4)

    stats_rows = [
        (3, "Показатель", "Значение", True),
        (4, "Исходное среднее (raw_avg), мс", (f"AVERAGE(B4:B{last_r})", raw_avg), False),
        (5, "Медиана (med), мс",              (f"MEDIAN(B4:B{last_r})", med), False),
        (6, "Стандартное отклонение (SD), мс",(f"STDEV(B4:B{last_r})", sd), False),
        (7, "Нижняя граница (med - SD), мс",  ("G5-G6", lo), False),
        (8, "Верхняя граница (med + SD), мс", ("G5+G6", hi), False),
        (9, "Принято замеров (из 100)",       (f"SUM(C4:C{last_r})", len(kept)), False),
        (10,"Отброшено выбросов",             (f"COUNT(B4:B{last_r})-G9", n_raw - len(kept)), False),
        (11,"Итог AvgTrustedIntervalMed, мс", (f"SUM(D4:D{last_r})/G9", clean_avg), False),
    ]
    sh4.set_col_width(6, 36); sh4.set_col_width(7, 16)
    for rr, lbl, val, is_hdr in stats_rows:
        if is_hdr:
            sh4.set_str(rr, 6, lbl, 2); sh4.set_str(rr, 7, val, 2)
        else:
            sh4.set_str(rr, 6, lbl, 3)
            sh4.set_formula(rr, 7, val[0], val[1], 5 if rr in (9, 10) else 4)

    # ---------------- ЛИСТ 5: Законы Амдала и Густавсона-Барсиса ----------------
    sh5 = SimpleSheet("Амдал и Густавсон")
    sh5.set_str(1, 1, "Оценка доли последовательных вычислений f и сравнение с законами Амдала и Густавсона–Барсиса (НД 3: 2560×1440)", 1)
    sh5.merge(1, 1, 1, 9)
    hdrs5 = ["Алгоритм / этап (2560×1440)", "f (оценка)", "S* = 1/f", "Sp(2) эксп.", "Sp(3) эксп.", "Sp(4) эксп.", "Амдал(4)", "Густавсон(4)", "Cp(4)/T1"]
    for ci, h in enumerate(hdrs5, 1):
        sh5.set_str(3, ci, h, 2)
    sh5.set_col_width(1, 44)

    amd_models = [
        ("Шаг 1: Яркость + Гаусс (OpenMP)",       [(p, float(idx61[(3, "Step1_GrayGauss", "Seq" if p==1 else "OpenMP", "None", p)]["speedup"])) for p in [1,2,3,4]]),
        ("Шаг 1: Яркость + Гаусс (oneTBB)",       [(p, float(idx61[(3, "Step1_GrayGauss", "Seq" if p==1 else "oneTBB", "None", p)]["speedup"])) for p in [1,2,3,4]]),
        ("Шаг 2: Градиенты 1 и 2 пор. (OpenMP)",  [(p, float(idx61[(3, "Step2_Gradients", "Seq" if p==1 else "OpenMP", "None", p)]["speedup"])) for p in [1,2,3,4]]),
        ("Шаг 2: Градиенты 1 и 2 пор. (oneTBB)",  [(p, float(idx61[(3, "Step2_Gradients", "Seq" if p==1 else "oneTBB", "None", p)]["speedup"])) for p in [1,2,3,4]]),
        ("Полный Харрис: OpenMP (STL)",           [(p, float(idx62[(3, "Seq" if p==1 else "OpenMP_STL", p)]["speedup"])) for p in [1,2,3,4]]),
        ("Полный Харрис: OpenMP (conc_vec)",      [(p, float(idx62[(3, "Seq" if p==1 else "OpenMP_TBBVec", p)]["speedup"])) for p in [1,2,3,4]]),
        ("Полный Харрис: Intel oneTBB (STL)",     [(p, float(idx62[(3, "Seq" if p==1 else "oneTBB_STL", p)]["speedup"])) for p in [1,2,3,4]]),
        ("Полный Харрис: Intel oneTBB (conc_vec)",[(p, float(idx62[(3, "Seq" if p==1 else "oneTBB_TBBVec", p)]["speedup"])) for p in [1,2,3,4]]),
        ("Полный Харрис: Гибрид OpenMP+TBB (STL)",[(p, float(idx62[(3, "Seq" if p==1 else "Hybrid_STL", p)]["speedup"])) for p in [1,2,3,4]]),
        ("Полный Харрис: Гибрид OpenMP+TBB (c_v)",[(p, float(idx62[(3, "Seq" if p==1 else "Hybrid_TBBVec", p)]["speedup"])) for p in [1,2,3,4]]),
    ]

    r = 4
    for lbl, pts in amd_models:
        fe = estimate_f(pts)
        d = dict(pts)
        s_star = 1.0 / fe if fe > 1e-4 else 999.0
        amd4 = 1.0 / (fe + (1.0 - fe) / 4.0)
        gus4 = fe + (1.0 - fe) * 4.0
        cp_ratio = 4.0 / d[4]
        sh5.set_str(r, 1, lbl, 3)
        sh5.set_num(r, 2, fe, 4)
        sh5.set_formula(r, 3, f"IF(B{r}>0.0001,1/B{r},999)", s_star, 4)
        sh5.set_num(r, 4, d[2], 4)
        sh5.set_num(r, 5, d[3], 4)
        sh5.set_num(r, 6, d[4], 4)
        sh5.set_formula(r, 7, f"1/(B{r}+(1-B{r})/4)", amd4, 4)
        sh5.set_formula(r, 8, f"B{r}+(1-B{r})*4", gus4, 4)
        sh5.set_formula(r, 9, f"4/F{r}", cp_ratio, 4)
        r += 1

    out_path = "Lab6/Результаты_исследования_Лаб6_OpenMP_oneTBB.xlsx"
    write_xlsx(out_path, [sh1, sh2, sh3, sh4, sh5])
    print(f"Successfully generated {out_path}")


if __name__ == "__main__":
    main()
