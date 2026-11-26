import csv
import os
import struct
import zipfile
from xml.sax.saxutils import escape


def fmt_ru(val: float, decimals: int = 4) -> str:
    return f"{val:,.{decimals}f}".replace(",", "\u00A0").replace(".", ",")


def fmt_sig4(val: float) -> str:
    av = abs(val)
    if av >= 1000.0:
        return fmt_ru(val, 2)
    elif av >= 100.0:
        return fmt_ru(val, 3)
    return fmt_ru(val, 4)


def png_dimensions(path: str):
    with open(path, "rb") as f:
        hdr = f.read(24)
        if hdr[:8] != b"\x89PNG\r\n\x1a\n":
            raise ValueError(f"Not PNG: {path}")
        return struct.unpack(">II", hdr[16:24])


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


class GostDocxBuilder:
    def __init__(self):
        self.body_parts = []
        self.images = []
        self.next_rid = 10
        self.next_docpr = 1

    def _run_xml(self, text: str, bold=False, italic=False, size_pt=14.0, font="Times New Roman", color="000000"):
        sz = int(round(size_pt * 2))
        rpr = [
            f'<w:rFonts w:ascii="{font}" w:hAnsi="{font}" w:cs="{font}" w:eastAsia="{font}"/>',
            f'<w:color w:val="{color}"/>',
            f'<w:sz w:val="{sz}"/><w:szCs w:val="{sz}"/><w:lang w:val="ru-RU"/>',
        ]
        if bold:
            rpr.append("<w:b/><w:bCs/>")
        if italic:
            rpr.append("<w:i/><w:iCs/>")
        return f'<w:r><w:rPr>{"".join(rpr)}</w:rPr><w:t xml:space="preserve">{escape(text)}</w:t></w:r>'

    def add_paragraph(self, text: str = "", align="both", indent_cm=1.25, bold=False, italic=False,
                      size_pt=14.0, space_before_pt=0, space_after_pt=0, line_mult=1.0,
                      keep_next=False, page_break_before=False, runs=None):
        ppr = []
        if keep_next:
            ppr.append("<w:keepNext/>")
        if page_break_before:
            ppr.append("<w:pageBreakBefore/>")
        ppr.append("<w:widowControl/>")
        ppr.append(f'<w:spacing w:before="{int(round(space_before_pt*20))}" w:after="{int(round(space_after_pt*20))}" w:line="{int(round(line_mult*240))}" w:lineRule="auto"/>')
        if indent_cm > 0:
            ppr.append(f'<w:ind w:firstLine="{int(round(indent_cm*567))}"/>')
        ppr.append(f'<w:jc w:val="{align}"/>')
        content = "".join(self._run_xml(**r) for r in runs) if runs is not None else (
            self._run_xml(text, bold=bold, italic=italic, size_pt=size_pt) if text else ""
        )
        self.body_parts.append(f'<w:p><w:pPr>{"".join(ppr)}</w:pPr>{content}</w:p>')

    def add_section_heading(self, num: str, title: str, page_break=False):
        self.add_paragraph(
            f"{num}. {title}", align="left", indent_cm=1.25, bold=True, size_pt=14.0,
            space_before_pt=0 if page_break else 10, space_after_pt=4,
            keep_next=True, page_break_before=page_break
        )

    def add_subsection_heading(self, num: str, title: str):
        self.add_paragraph(
            f"{num}. {title}", align="left", indent_cm=1.25, bold=True, size_pt=14.0,
            space_before_pt=8, space_after_pt=4, keep_next=True
        )

    def add_bullet(self, text: str, bold_prefix: str = ""):
        if bold_prefix:
            self.add_paragraph(align="both", indent_cm=1.25, runs=[
                {"text": "– ", "size_pt": 14.0},
                {"text": bold_prefix, "bold": True, "size_pt": 14.0},
                {"text": text, "size_pt": 14.0},
            ])
        else:
            self.add_paragraph(f"– {text}", align="both", indent_cm=1.25)

    def add_table(self, caption: str, headers: list, rows: list, col_widths: list, font_pt: float = 9.0, aligns=None):
        if caption:
            self.add_paragraph(caption, align="left", indent_cm=0, size_pt=14.0, space_before_pt=6, space_after_pt=3, keep_next=True)
        total_w = sum(col_widths)
        if aligns is None:
            aligns = ["left"] + ["right"] * (len(col_widths) - 1)
        parts = [
            f'<w:tbl><w:tblPr><w:tblW w:w="{total_w}" w:type="dxa"/><w:tblLayout w:type="fixed"/><w:jc w:val="center"/>'
            '<w:tblBorders>'
            '<w:top w:val="single" w:sz="6" w:space="0" w:color="000000"/>'
            '<w:left w:val="single" w:sz="6" w:space="0" w:color="000000"/>'
            '<w:bottom w:val="single" w:sz="6" w:space="0" w:color="000000"/>'
            '<w:right w:val="single" w:sz="6" w:space="0" w:color="000000"/>'
            '<w:insideH w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
            '<w:insideV w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
            '</w:tblBorders>'
            '<w:tblCellMar><w:top w:w="35" w:type="dxa"/><w:bottom w:w="35" w:type="dxa"/>'
            '<w:left w:w="55" w:type="dxa"/><w:right w:w="55" w:type="dxa"/></w:tblCellMar></w:tblPr>'
            '<w:tblGrid>' + "".join(f'<w:gridCol w:w="{w}"/>' for w in col_widths) + '</w:tblGrid>'
            '<w:tr><w:trPr><w:tblHeader/><w:cantSplit/></w:trPr>'
        ]
        for ci, h in enumerate(headers):
            parts.append(
                f'<w:tc><w:tcPr><w:tcW w:w="{col_widths[ci]}" w:type="dxa"/>'
                '<w:shd w:val="clear" w:color="auto" w:fill="E8ECF2"/><w:vAlign w:val="center"/></w:tcPr>'
                '<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="220" w:lineRule="auto"/><w:jc w:val="center"/></w:pPr>'
                + self._run_xml(h, bold=True, size_pt=font_pt) + '</w:p></w:tc>'
            )
        parts.append('</w:tr>')
        for row in rows:
            parts.append('<w:tr><w:trPr><w:cantSplit/></w:trPr>')
            for ci, val in enumerate(row):
                parts.append(
                    f'<w:tc><w:tcPr><w:tcW w:w="{col_widths[ci]}" w:type="dxa"/><w:vAlign w:val="center"/></w:tcPr>'
                    f'<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="220" w:lineRule="auto"/><w:jc w:val="{aligns[ci]}"/></w:pPr>'
                    + self._run_xml(str(val), size_pt=font_pt) + '</w:p></w:tc>'
                )
            parts.append('</w:tr>')
        parts.append('</w:tbl>')
        self.body_parts.append("".join(parts))

    def add_figure(self, img_path: str, fig_num: int, caption: str, width_cm: float = 15.8, max_height_cm: float = 11.2):
        w_px, h_px = png_dimensions(img_path)
        cx = int(round(width_cm * 360000))
        cy = int(round(cx * (h_px / float(w_px))))
        max_cy = int(round(max_height_cm * 360000))
        if cy > max_cy:
            cy = max_cy
            cx = int(round(cy * (w_px / float(h_px))))

        rid = f"rId{self.next_rid}"; self.next_rid += 1
        media_name = f"image{len(self.images) + 1}.png"
        self.images.append((rid, media_name, os.path.abspath(img_path)))
        docpr_id = self.next_docpr; self.next_docpr += 1

        self.body_parts.append(
            '<w:p><w:pPr><w:keepNext/>'
            '<w:spacing w:before="70" w:after="30" w:line="240" w:lineRule="auto"/><w:jc w:val="center"/></w:pPr>'
            '<w:r><w:drawing><wp:inline distT="0" distB="0" distL="0" distR="0" '
            'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing">'
            f'<wp:extent cx="{cx}" cy="{cy}"/><wp:effectExtent l="0" t="0" r="0" b="0"/>'
            f'<wp:docPr id="{docpr_id}" name="Picture {docpr_id}"/>'
            '<wp:cNvGraphicFramePr><a:graphicFrameLocks xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" noChangeAspect="1"/></wp:cNvGraphicFramePr>'
            '<a:graphic xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">'
            '<a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/picture">'
            '<pic:pic xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture">'
            f'<pic:nvPicPr><pic:cNvPr id="{docpr_id}" name="{media_name}"/><pic:cNvPicPr/></pic:nvPicPr>'
            f'<pic:blipFill><a:blip r:embed="{rid}" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill>'
            f'<pic:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm><a:prstGeom prst="rect"><a:avLst/></a:prstGeom></pic:spPr>'
            '</pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing></w:r></w:p>'
        )
        self.add_paragraph(f"Рисунок {fig_num} – {caption}", align="center", indent_cm=0, size_pt=12.0, space_after_pt=6)

    def add_title_page(self):
        self.add_paragraph("МИНИСТЕРСТВО НАУКИ И ВЫСШЕГО ОБРАЗОВАНИЯ РОССИЙСКОЙ ФЕДЕРАЦИИ", align="center", indent_cm=0, size_pt=11.0)
        self.add_paragraph("Федеральное государственное бюджетное образовательное учреждение высшего образования", align="center", indent_cm=0, size_pt=11.0)
        self.add_paragraph("«Сибирский государственный университет науки и технологий имени академика М.Ф. Решетнева»", align="center", indent_cm=0, bold=True, size_pt=11.5, space_after_pt=4)
        self.add_paragraph("Институт информатики и телекоммуникаций", align="center", indent_cm=0, size_pt=12.0)
        self.add_paragraph("Кафедра информатики и вычислительной техники", align="center", indent_cm=0, size_pt=12.0, space_after_pt=56)

        self.add_paragraph("ОТЧЕТ ПО ЛАБОРАТОРНОЙ РАБОТЕ", align="center", indent_cm=0, bold=True, size_pt=16.0, space_after_pt=8)
        self.add_paragraph("Параллельные вычисления", align="center", indent_cm=0, size_pt=14.0, space_after_pt=18)

        box = (
            '<w:tbl><w:tblPr><w:tblW w:w="9071" w:type="dxa"/><w:jc w:val="center"/>'
            '<w:tblBorders><w:top w:val="single" w:sz="6" w:space="0" w:color="000000"/>'
            '<w:left w:val="single" w:sz="6" w:space="0" w:color="000000"/>'
            '<w:bottom w:val="single" w:sz="6" w:space="0" w:color="000000"/>'
            '<w:right w:val="single" w:sz="6" w:space="0" w:color="000000"/>'
            '<w:insideH w:val="single" w:sz="4" w:space="0" w:color="000000"/><w:insideV w:val="none"/></w:tblBorders>'
            '<w:tblCellMar><w:top w:w="90" w:type="dxa"/><w:bottom w:w="90" w:type="dxa"/><w:left w:w="120" w:type="dxa"/><w:right w:w="120" w:type="dxa"/></w:tblCellMar>'
            '</w:tblPr><w:tblGrid><w:gridCol w:w="9071"/></w:tblGrid>'
            '<w:tr><w:tc><w:tcPr><w:tcW w:w="9071" w:type="dxa"/></w:tcPr><w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/><w:jc w:val="center"/></w:pPr>'
            + self._run_xml("Лабораторная работа № 4", bold=True, size_pt=14.0) + '</w:p></w:tc></w:tr>'
            '<w:tr><w:tc><w:tcPr><w:tcW w:w="9071" w:type="dxa"/></w:tcPr><w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/><w:jc w:val="center"/></w:pPr>'
            + self._run_xml("Алгоритмы обработки массивов и их реализация с использованием oneTBB", size_pt=14.0)
            + '</w:p></w:tc></w:tr></w:tbl>'
        )
        self.body_parts.append(box)
        self.add_paragraph("", indent_cm=0, space_after_pt=76)

        def sig_cell(w, text, align="center", size=14.0, bottom_border=False, italic=False):
            bdr = '<w:tcBorders><w:bottom w:val="single" w:sz="4" w:space="0" w:color="000000"/></w:tcBorders>' if bottom_border else ''
            return (
                f'<w:tc><w:tcPr><w:tcW w:w="{w}" w:type="dxa"/>{bdr}<w:vAlign w:val="bottom"/></w:tcPr>'
                f'<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="220" w:lineRule="auto"/><w:jc w:val="{align}"/></w:pPr>'
                + (self._run_xml(text, italic=italic, size_pt=size) if text else "") + '</w:p></w:tc>'
            )

        ws = [2100, 2600, 200, 1971, 200, 2000]
        self.body_parts.append(
            '<w:tbl><w:tblPr><w:tblW w:w="9071" w:type="dxa"/><w:tblLayout w:type="fixed"/><w:jc w:val="center"/>'
            '<w:tblBorders><w:top w:val="none"/><w:left w:val="none"/><w:bottom w:val="none"/>'
            '<w:right w:val="none"/><w:insideH w:val="none"/><w:insideV w:val="none"/></w:tblBorders>'
            '<w:tblCellMar><w:top w:w="20" w:type="dxa"/><w:bottom w:w="20" w:type="dxa"/><w:left w:w="20" w:type="dxa"/><w:right w:w="20" w:type="dxa"/></w:tblCellMar>'
            '</w:tblPr><w:tblGrid>' + "".join(f'<w:gridCol w:w="{w}"/>' for w in ws) + '</w:tblGrid>'
            '<w:tr>' + sig_cell(ws[0], "Руководитель", "left") + sig_cell(ws[1], "") + sig_cell(ws[2], "")
            + sig_cell(ws[3], "", "center", 14.0, True) + sig_cell(ws[4], "") + sig_cell(ws[5], "А.Г. Зотин", "center", 14.0, True) + '</w:tr>'
            '<w:tr>' + sig_cell(ws[0], "", "left", 9.0) + sig_cell(ws[1], "", "center", 9.0) + sig_cell(ws[2], "", "center", 9.0)
            + sig_cell(ws[3], "подпись, дата", "center", 9.5, False, True) + sig_cell(ws[4], "", "center", 9.0)
            + sig_cell(ws[5], "инициалы, фамилия", "center", 9.5, False, True) + '</w:tr>'
            '<w:tr>' + "".join(sig_cell(w, " ", "center", 10.0) for w in ws) + '</w:tr>'
            '<w:tr>' + sig_cell(ws[0], "Обучающийся", "left") + sig_cell(ws[1], "БПИ24-02, 24151438", "center", 13.0, True)
            + sig_cell(ws[2], "") + sig_cell(ws[3], "", "center", 14.0, True) + sig_cell(ws[4], "")
            + sig_cell(ws[5], "Д.А. Пузынин", "center", 14.0, True) + '</w:tr>'
            '<w:tr>' + sig_cell(ws[0], "", "left", 9.0) + sig_cell(ws[1], "номер группы, зачетной книжки", "center", 9.5, False, True)
            + sig_cell(ws[2], "", "center", 9.0) + sig_cell(ws[3], "подпись, дата", "center", 9.5, False, True)
            + sig_cell(ws[4], "", "center", 9.0) + sig_cell(ws[5], "инициалы, фамилия", "center", 9.5, False, True) + '</w:tr></w:tbl>'
        )
        self.add_paragraph("", indent_cm=0, space_before_pt=135)
        self.add_paragraph("Красноярск 2026 г.", align="center", indent_cm=0, size_pt=14.0)

    def save(self, out_path: str):
        sect_pr = (
            '<w:sectPr><w:footerReference w:type="default" r:id="rId2"/><w:footerReference w:type="first" r:id="rId3"/>'
            '<w:pgSz w:w="11906" w:h="16838"/>'
            '<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1701" w:header="708" w:footer="567" w:gutter="0"/>'
            '<w:titlePg/></w:sectPr>'
        )
        doc_xml = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
            'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
            'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing">\n<w:body>\n'
            + "\n".join(self.body_parts) + "\n" + sect_pr + "\n</w:body>\n</w:document>"
        )
        styles_xml = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">\n'
            '  <w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii="Times New Roman" w:hAnsi="Times New Roman" w:cs="Times New Roman"/>'
            '<w:sz w:val="28"/><w:szCs w:val="28"/><w:lang w:val="ru-RU"/></w:rPr></w:rPrDefault>'
            '<w:pPrDefault><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/></w:pPr></w:pPrDefault></w:docDefaults>'
            '<w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style></w:styles>'
        )
        ftr1 = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<w:ftr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:p><w:pPr><w:jc w:val="center"/></w:pPr>'
            '<w:fldSimple w:instr=" PAGE "><w:r><w:rPr><w:rFonts w:ascii="Times New Roman" w:hAnsi="Times New Roman"/><w:sz w:val="24"/><w:szCs w:val="24"/></w:rPr><w:t>2</w:t></w:r></w:fldSimple>'
            '</w:p></w:ftr>'
        )
        ftr2 = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<w:ftr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:p/></w:ftr>'
        settings = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:compat><w:compatSetting w:name="compatibilityMode" w:uri="http://schemas.microsoft.com/office/word" w:val="15"/></w:compat></w:settings>'
        rels = [
            '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>',
            '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer" Target="footer1.xml"/>',
            '<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer" Target="footer2.xml"/>',
            '<Relationship Id="rId4" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>',
        ] + [
            f'<Relationship Id="{rid}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" Target="media/{mn}"/>'
            for rid, mn, _ in self.images
        ]
        doc_rels = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">\n' + "\n".join("  " + r for r in rels) + "\n</Relationships>"
        ct = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
            '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>'
            '<Default Extension="xml" ContentType="application/xml"/><Default Extension="png" ContentType="image/png"/>'
            '<Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>'
            '<Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>'
            '<Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>'
            '<Override PartName="/word/footer1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"/>'
            '<Override PartName="/word/footer2.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"/></Types>'
        )
        root_rels = '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships"><Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/></Relationships>'

        with zipfile.ZipFile(out_path, "w", zipfile.ZIP_DEFLATED) as zf:
            zf.writestr("[Content_Types].xml", ct)
            zf.writestr("_rels/.rels", root_rels)
            zf.writestr("word/document.xml", doc_xml)
            zf.writestr("word/styles.xml", styles_xml)
            zf.writestr("word/settings.xml", settings)
            zf.writestr("word/footer1.xml", ftr1)
            zf.writestr("word/footer2.xml", ftr2)
            zf.writestr("word/_rels/document.xml.rels", doc_rels)
            for _, mn, ap in self.images:
                zf.write(ap, f"word/media/{mn}")


def load_csv(path: str):
    with open(path, "r", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def main():
    t41 = load_csv("Lab4/results/task4_1_summary.csv")
    t42 = load_csv("Lab4/results/task4_2_summary.csv")
    t43 = load_csv("Lab4/results/task4_3_summary.csv")
    idx41 = {(int(r["ds_id"]), r["impl_name"], int(r["threads"])): r for r in t41}
    idx42 = {(int(r["img_id"]), r["impl_name"], int(r["ksize"]), int(r["threads"])): r for r in t42}
    idx43 = {(int(r["img_id"]), r["impl_name"], int(r["ksize"]), int(r["threads"])): r for r in t43}

    doc = GostDocxBuilder()
    doc.add_title_page()

    # 1. Цель работы
    doc.add_section_heading("1", "Цель работы", page_break=True)
    doc.add_paragraph(
        "Изучение различных видов реализации алгоритмов обработки массивов с использованием возможностей "
        "Intel oneTBB. Получение практических навыков использования oneTBB для параллельной реализации "
        "алгоритма медианного фильтра. Реализация алгоритмов вычисления текстурных признаков с применением "
        "OpenMP и oneTBB."
    )

    # 2. Задания
    doc.add_section_heading("2", "Задания")
    doc.add_paragraph(align="both", indent_cm=1.25, runs=[
        {"text": "Задание 4.1. ", "bold": True, "size_pt": 14.0},
        {"text": (
            "Разработать консольное приложение, выполняющее над квадратными вещественными матрицами "
            "(3000×3000 – 8000×8000, 3 набора данных, не менее 100 запусков) сложение, поэлементное "
            "перемножение, расчёт суммы всех элементов двух матриц и разницы максимума и минимума "
            "(Val_range = maxEl − minEl). Использовать OpenMP (FOR, Sections, редукция) и Intel oneTBB "
            "(task_group, parallel_for на основе лямбда-выражения и класса; для редукций — комбинации "
            "класса, лямбда-выражения и структурных объектов)."
        ), "size_pt": 14.0}
    ])
    doc.add_paragraph(align="both", indent_cm=1.25, runs=[
        {"text": "Задание 4.2. ", "bold": True, "size_pt": 14.0},
        {"text": (
            "Разработать консольное приложение медианной фильтрации трёх BMP-изображений (1280×720 – 2560×1440) "
            "для размеров окрестности Ksize = 5, 9, 13 (не менее 15 запусков) в последовательном варианте, "
            "а также с распараллеливанием OpenMP и Intel oneTBB с сохранением выходных файлов output_X_alg.bmp."
        ), "size_pt": 14.0}
    ])
    doc.add_paragraph(align="both", indent_cm=1.25, runs=[
        {"text": "Задание 4.3. ", "bold": True, "size_pt": 14.0},
        {"text": (
            "Разработать консольное приложение расчёта четырёх текстурных признаков (момент второго порядка μ2, "
            "относительная гладкость R, однородность U, средняя энтропия e) в локальной окрестности 5×5, 7×7, 9×9 "
            "для двух BMP-изображений из диапазона [1200×720 .. 2560×1440] (не менее 10 запусков) с формированием "
            "цветных карт по двум пользовательским порогам T1 и T2 в реализациях: последовательной, OpenMP и Intel oneTBB "
            "(включая дополнительную 2D-реализацию на blocked_range2d)."
        ), "size_pt": 14.0}
    ])

    # 3. Формулы заполнения и методика замеров
    doc.add_section_heading("3", "Формулы заполнения матриц и методика измерений")
    doc.add_paragraph(
        "В задании 4.1 вещественные матрицы double (НД 1: 3000×3000, НД 2: 5500×5500, НД 3: 8000×8000) "
        "заполнялись по детерминированным формулам от индексов i, j:"
    )
    doc.add_paragraph("mas1[i][j] = 100·sin(0,001·i + 0,002·j) + 0,01·((31·i + 17·j) mod 100)",
                      align="center", indent_cm=0, italic=True, size_pt=12.5, space_before_pt=2, space_after_pt=1)
    doc.add_paragraph("mas2[i][j] = 50·cos(0,002·i − 0,001·j) + 0,01·((13·i + 29·j) mod 100) + 1,5",
                      align="center", indent_cm=0, italic=True, size_pt=12.5, space_before_pt=0, space_after_pt=3)
    doc.add_paragraph(
        "Замеры времени выполнены функцией omp_get_wtime(), очистка выбросов — функцией AvgTrustedIntervalMed "
        "(доверительный интервал med ± SD), ограничение числа потоков oneTBB — через oneapi::tbb::task_arena(p). "
        "В задании 4.3 использованы коэффициенты порогов k1 = 0,30 ∈ [0,25; 0,40] и k2 = 0,70 ∈ [0,60; 0,75]."
    )

    # 4. Конфигурация компьютера
    doc.add_section_heading("4", "Конфигурация компьютера и параметры ОС")
    doc.add_bullet("AMD Ryzen 5 5600 (6 физических ядер / 12 логических потоков), частота до 4,65 ГГц.", "Процессор: ")
    doc.add_bullet("32 ГБ DDR4 (2 модуля), частота 3000 МГц.", "Оперативная память: ")
    doc.add_bullet("Windows 11 26H2 (сборка 26300, 64-разрядная).", "Операционная система: ")
    doc.add_bullet("G++ 15.2.0 (MSYS2 MinGW UCRT64), OpenMP 4.5 (_OPENMP = 201511) и Intel oneTBB 2022.2.0 (-ltbb12).", "Инструментарий: ")
    doc.add_bullet("Release (-std=c++17 -fopenmp -O3 -march=native -Wall -Wextra -Werror, 0 предупреждений).", "Сборка (VS Code): ")

    # 5. Экранные снимки полного консольного вывода
    doc.add_section_heading("5", "Экранные снимки работы программ (полный вывод консоли)", page_break=True)

    fig_num = 1
    doc.add_subsection_heading("5.1", "Задание 4.1 — обработка вещественных матриц (OpenMP и oneTBB)")
    doc.add_paragraph("Полный консольный вывод программы task4_1.exe по всем трём наборам данных (3000×3000, 5500×5500, 8000×8000, по 100 запусков):")
    for fn, cap, mh in [
        ("Lab4/results/c41_p1.png", "Полный консольный вывод задания 4.1 (ч. 1/6): параметры и НД 1 (3000×3000) — сложение и умножение", 11.2),
        ("Lab4/results/c41_p2.png", "Полный консольный вывод задания 4.1 (ч. 2/6): НД 1 (3000×3000) — редукции sumEl и Val_range", 9.6),
        ("Lab4/results/c41_p3.png", "Полный консольный вывод задания 4.1 (ч. 3/6): НД 2 (5500×5500) — сложение и умножение", 10.6),
        ("Lab4/results/c41_p4.png", "Полный консольный вывод задания 4.1 (ч. 4/6): НД 2 (5500×5500) — редукции sumEl и Val_range", 9.6),
        ("Lab4/results/c41_p5.png", "Полный консольный вывод задания 4.1 (ч. 5/6): НД 3 (8000×8000) — сложение и умножение", 10.6),
        ("Lab4/results/c41_p6.png", "Полный консольный вывод задания 4.1 (ч. 6/6): НД 3 (8000×8000) — редукции sumEl и Val_range", 9.8),
    ]:
        doc.add_figure(fn, fig_num, cap, width_cm=15.8, max_height_cm=mh); fig_num += 1

    doc.add_subsection_heading("5.2", "Задание 4.2 — медианный фильтр BMP (OpenMP и oneTBB)")
    doc.add_paragraph("Полный консольный вывод программы task4_2.exe для трёх изображений (1280×720, 1920×1080, 2560×1440, Ksize = 5, 9, 13, 15 запусков) и визуальные примеры фильтрации:")
    for fn, cap, mh in [
        ("Lab4/results/c42_p1.png", "Полный консольный вывод задания 4.2 (ч. 1/3): загрузка BMP и НД 1 (1280×720)", 11.4),
        ("Lab4/results/c42_p2.png", "Полный консольный вывод задания 4.2 (ч. 2/3): НД 2 (1920×1080)", 10.4),
        ("Lab4/results/c42_p3.png", "Полный консольный вывод задания 4.2 (ч. 3/3): НД 3 (2560×1440)", 10.6),
    ]:
        doc.add_figure(fn, fig_num, cap, width_cm=15.8, max_height_cm=mh); fig_num += 1

    doc.add_figure("Lab4/results/visual_task42_median.png", fig_num,
                   "Примеры исходных зашумлённых и отфильтрованных BMP-изображений (Задание 4.2)", width_cm=15.8, max_height_cm=9.4)
    fig_num += 1

    doc.add_subsection_heading("5.3", "Задание 4.3 — расчёт текстурных признаков и построение карт")
    doc.add_paragraph("Полный консольный вывод программы task4_3.exe для двух изображений (1280×720 и 2560×1440, окна 5×5, 7×7, 9×9, 15 запусков, OpenMP, oneTBB 1D и доп. oneTBB 2D blocked_range2d) и сформированные цветные карты признаков:")
    for fn, cap, mh in [
        ("Lab4/results/c43_p1.png", "Полный консольный вывод задания 4.3 (ч. 1/4): параметры, загрузка BMP и НД 1 (1280×720, 5×5 и 7×7)", 11.4),
        ("Lab4/results/c43_p2.png", "Полный консольный вывод задания 4.3 (ч. 2/4): НД 1 (1280×720, 9×9) и начало НД 2 (2560×1440, 5×5)", 10.6),
        ("Lab4/results/c43_p3.png", "Полный консольный вывод задания 4.3 (ч. 3/4): НД 2 (2560×1440, 7×7 и 9×9)", 10.6),
    ]:
        doc.add_figure(fn, fig_num, cap, width_cm=15.8, max_height_cm=mh); fig_num += 1

    doc.add_figure("Lab4/results/visual_task43_textures.png", fig_num,
                   "Исходное изображение (1280×720) и карты текстурных признаков μ2, R, U и e (k1 = 0,30; k2 = 0,70)", width_cm=15.8, max_height_cm=11.0)
    fig_num += 1

    # 6. Результаты и анализ
    doc.add_section_heading("6", "Результаты экспериментального исследования", page_break=True)
    doc.add_paragraph(
        "Полные таблицы 4.1, 4.2 и 4.3 (время, Sp, Ep, Cp), проверка доверительного интервала по 100 "
        "сырым замерам и расчёты по законам Амдала и Густавсона–Барсиса приведены в приложенном файле "
        "«Результаты_исследования_Лаб4_oneTBB.xlsx». В таблице 1 сведена оценка доли последовательных "
        "вычислений f и сравнение с теоретическими пределами на максимальных наборах данных."
    )

    models = [
        ("Зад. 4.1: Сложение oneTBB parallel_for (8000×8000)", [(p, float(idx41[(3, "Add_Seq" if p==1 else "Add_TBB_ParFor_Lambda", p)]["speedup"])) for p in [1,2,3,4]]),
        ("Зад. 4.1: Сумма oneTBB класс (8000×8000)",          [(p, float(idx41[(3, "Sum_Seq" if p==1 else "Sum_TBB_Red_Class", p)]["speedup"])) for p in [1,2,3,4]]),
        ("Зад. 4.1: Val_range oneTBB структуры (8000×8000)",  [(p, float(idx41[(3, "Range_Seq" if p==1 else "Range_TBB_Red_Struct", p)]["speedup"])) for p in [1,2,3,4]]),
        ("Зад. 4.2: Медианный OpenMP (2560×1440, K=13)",      [(p, float(idx42[(3, "Median_Seq" if p==1 else "Median_OpenMP", 13, p)]["speedup"])) for p in [1,2,3,4]]),
        ("Зад. 4.2: Медианный oneTBB (2560×1440, K=13)",      [(p, float(idx42[(3, "Median_Seq" if p==1 else "Median_oneTBB", 13, p)]["speedup"])) for p in [1,2,3,4]]),
        ("Зад. 4.3: Текстуры OpenMP (2560×1440, 9×9)",        [(p, float(idx43[(2, "Texture_Seq" if p==1 else "Texture_OpenMP", 9, p)]["speedup"])) for p in [1,2,3,4]]),
        ("Зад. 4.3: Текстуры oneTBB 1D (2560×1440, 9×9)",     [(p, float(idx43[(2, "Texture_Seq" if p==1 else "Texture_oneTBB_1D", 9, p)]["speedup"])) for p in [1,2,3,4]]),
        ("Зад. 4.3: Текстуры oneTBB 2D (2560×1440, 9×9)",     [(p, float(idx43[(2, "Texture_Seq" if p==1 else "Texture_oneTBB_2D", 9, p)]["speedup"])) for p in [1,2,3,4]]),
    ]
    t_rows = []
    for lbl, pts in models:
        fe = estimate_f(pts)
        sp4 = dict(pts)[4]
        amd4 = 1.0 / (fe + (1.0 - fe) / 4.0)
        gus4 = fe + (1.0 - fe) * 4.0
        t_rows.append([lbl, fmt_ru(fe, 4), fmt_ru(1.0 / fe if fe > 1e-4 else 999.0, 2), fmt_ru(sp4, 3), fmt_ru(amd4, 3), fmt_ru(gus4, 3)])
    doc.add_table(
        "Таблица 1 – Оценка доли последовательных вычислений f и сравнение с законами Амдала / Густавсона–Барсиса",
        ["Алгоритм", "f", "S* = 1/f", "Sp(4) эксп.", "Амдал (4)", "Густавсон (4)"],
        t_rows, [3971, 950, 950, 1080, 1060, 1060], font_pt=9.0,
        aligns=["left", "right", "right", "right", "right", "right"]
    )

    for fn, cap, mh in [
        ("Lab4/results/chart_ci_task41.png", "Доверительный интервал (med ± SD): Задание 4.1, Val_range oneTBB структуры (8000×8000, p = 4, N = 100)", 8.8),
        ("Lab4/results/chart_task41_speedup.png", "Сравнение ускорения Sp(4) реализаций OpenMP и Intel oneTBB в Задании 4.1 (8000×8000)", 8.8),
        ("Lab4/results/chart_task42_43_speedup.png", "Ускорение Sp(p) медианного фильтра (Зад. 4.2) и расчёта текстурных признаков (Зад. 4.3)", 8.8),
    ]:
        doc.add_figure(fn, fig_num, cap, width_cm=15.8, max_height_cm=mh); fig_num += 1

    add_sp2 = float(idx41[(3, "Add_TBB_ParFor_Lambda", 2)]["speedup"])
    rng_sp4 = float(idx41[(3, "Range_TBB_Red_Struct", 4)]["speedup"])
    m42_omp4 = float(idx42[(3, "Median_OpenMP", 13, 4)]["speedup"])
    m42_tbb4 = float(idx42[(3, "Median_oneTBB", 13, 4)]["speedup"])
    t43_omp4 = float(idx43[(2, "Texture_OpenMP", 9, 4)]["speedup"])
    t43_t1d4 = float(idx43[(2, "Texture_oneTBB_1D", 9, 4)]["speedup"])
    t43_t2d4 = float(idx43[(2, "Texture_oneTBB_2D", 9, 4)]["speedup"])

    doc.add_subsection_heading("6.1", "Наблюдения")
    doc.add_paragraph(
        f"В задании 4.1 поэлементные операции сложения и умножения матриц (3000×3000 – 8000×8000) "
        f"ограничены пропускной способностью оперативной памяти DDR4-3000 (memory-bound: 1 FLOP на 24 байта "
        f"обмена с ОЗУ). Уже на p = 2 потоках достигается насыщение шины памяти (Sp ≈ {fmt_ru(add_sp2, 2)} на 8000×8000), "
        f"причём все пять параллельных вариантов (OpenMP FOR/Sections, oneTBB task_group, parallel_for лямбда "
        f"и класс) показывают практически идентичное время. Редукция суммы (чтение без записи) и особенно "
        f"редукция Val_range (минимум и максимум — выше вычислительная интенсивность) масштабируются заметно "
        f"лучше (Val_range до Sp(4) = {fmt_ru(rng_sp4, 2)}), при этом варианты oneTBB (класс, лямбда, структуры) "
        f"не уступают OpenMP reduction."
    )
    doc.add_paragraph(
        f"В вычислительно-интенсивных задачах обработки изображений (Задания 4.2 и 4.3) ограничения шины "
        f"памяти отсутствуют, и достигается почти линейное ускорение и стоимостная оптимальность (Cp ≈ T1): "
        f"в задании 4.2 на 2560×1440 (Ksize = 13) OpenMP даёт Sp(4) = {fmt_ru(m42_omp4, 3)}, а Intel oneTBB — "
        f"Sp(4) = {fmt_ru(m42_tbb4, 3)} благодаря work-stealing планировщику по единому пространству 3×Height строк. "
        f"В задании 4.3 на 2560×1440 (9×9) OpenMP достигает Sp(4) = {fmt_ru(t43_omp4, 3)}, Intel oneTBB 1D — "
        f"{fmt_ru(t43_t1d4, 3)}, а дополнительная 2D-реализация на blocked_range2d — {fmt_ru(t43_t2d4, 3)} за счёт "
        f"улучшенной 2D-локальности кэша L1/L2. Сформированные цветные карты чётко сегментируют гладкие, "
        f"умеренно текстурированные и высококонтрастные области изображения."
    )

    # 7. Выводы
    doc.add_section_heading("7", "Выводы")
    doc.add_subsection_heading("7.1", "Выводы по результатам исследования по заданиям")
    doc.add_bullet(
        "продемонстрировано влияние ограничения пропускной способности памяти (memory wall) на поэлементные "
        "операции с большими матрицами; редукции суммы и размаха Val_range показывают более высокую "
        "эффективность, а все три подхода oneTBB (класс, лямбда, структурные объекты) обеспечивают эквивалентную "
        "высокую производительность и побитовую корректность.",
        "По заданию 4.1: "
    )
    doc.add_bullet(
        "для медианного фильтра на разрешениях 1280×720 – 2560×1440 обе технологии (OpenMP и Intel oneTBB) "
        "обеспечивают близкое к идеальному ускорение Sp(4) ≈ 3,85–4,00 и стоимостную оптимальность (Cp ≈ T1); "
        "динамическая балансировка work-stealing в oneTBB на уровне задач сопоставима с schedule(dynamic) в OpenMP.",
        "По заданию 4.2: "
    )
    doc.add_bullet(
        "реализован полный цикл вычисления текстурных признаков (μ2, R, U, e), параллельной редукции min/max "
        "(в oneTBB — через специализированный класс редукции) и пороговой визуализации; 2D-декомпозиция "
        "blocked_range2d в oneTBB подтвердила высокую масштабируемость на окнах 5×5 – 9×9.",
        "По заданию 4.3: "
    )
    doc.add_subsection_heading("7.2", "Выводы по лабораторной работе в целом")
    doc.add_paragraph(
        "Освоены ключевые механизмы библиотеки Intel oneTBB (task_arena, task_group, parallel_for, "
        "parallel_reduce, blocked_range и blocked_range2d) и выполнено их практическое сравнение с OpenMP "
        "на задачах матричной обработки, медианной фильтрации и текстурного анализа BMP-изображений."
    )

    # 8. Ответы на контрольные вопросы (все 14 вопросов со стр. 5 методички — кратко по сути)
    doc.add_section_heading("8", "Ответы на контрольные вопросы", page_break=True)

    qa = [
        ("1. Охарактеризуйте Intel oneTBB.",
         "Intel oneAPI Threading Building Blocks (oneTBB) — кроссплатформенная шаблонная библиотека C++ "
         "для параллелизма задач на системах с общей памятью. Абстрагирует программиста от ручного управления "
         "потоками ОС, используя пул потоков и планировщик с кражей задач (work-stealing), а также предоставляет "
         "параллельные алгоритмы, потокобезопасные контейнеры и масштабируемый аллокатор памяти."),

        ("2. Как можно узнать максимальное количество потоков которые будут созданы по умолчанию в oneTBB?",
         "С помощью функции oneapi::tbb::info::default_concurrency() (в устаревшем API — "
         "tbb::task_scheduler_init::default_num_threads() или tbb::this_task_arena::max_concurrency()). "
         "Она возвращает число аппаратных потоков, доступных текущему процессу."),

        ("3. Каким образом в Intel oneTBB происходит управление потоками?",
         "Управление максимальным уровнем параллелизма выполняется через объект oneapi::tbb::global_control "
         "(параметр max_allowed_parallelism) — глобально для процесса, либо локально через изолированную "
         "область выполнения oneapi::tbb::task_arena arena(num_threads) с запуском кода внутри arena.execute(...)."),

        ("4. Дайте понятие диапазона (Range), какие типы моделей диапазона доступны в oneTBB?",
         "Диапазон (Range) — это рекурсивно делимое пространство итераций (с методами empty(), is_divisible() "
         "и разделяющим конструктором со splitting-тегом tbb::split). Основные встроенные модели: "
         "blocked_range<T> (1D), blocked_range2d<Row,Col> (2D), blocked_range3d<Page,Row,Col> (3D) и "
         "blocked_rangeNd<T,N> (N-мерный диапазон)."),

        ("5. Охарактеризуйте oneTBB parallel_for.",
         "Шаблонный алгоритм параллельного выполнения независимых итераций по заданному диапазону Range. "
         "Рекурсивно разбивает диапазон на поддиапазоны и распределяет их по потокам пула. Поддерживает "
         "передачу тела цикла в виде лямбда-выражения, функтора (класса/структуры с operator()) или простого индексного цикла."),

        ("6. Поясните аспекты управления распределением работы oneTBB parallel_for.",
         "Распределение регулируется размером зерна grainsize (минимальный неделимый блок в blocked_range) "
         "и стратегией разбиения (partitioner): auto_partitioner (по умолчанию, адаптивная балансировка), "
         "simple_partitioner (строго до grainsize), static_partitioner (равномерное статическое деление) и "
         "affinity_partitioner (сохранение привязки блоков к кэшам ядер между повторными вызовами)."),

        ("7. Охарактеризуйте oneTBB parallel_for_each.",
         "Алгоритм параллельного применения функции к каждому элементу последовательности, заданной парой "
         "итераторов [first, last) (в том числе к контейнерам без произвольного доступа, например std::list). "
         "Поддерживает динамическое добавление новых элементов в процессе обработки через объект feeder."),

        ("8. В чем заключается особенность использования parallel_invoke?",
         "oneapi::tbb::parallel_invoke принимает фиксированный на этапе компиляции набор (от 2 и более) "
         "независимых вызываемых объектов (функций, лямбд, функторов), запускает их параллельно и "
         "автоматически ожидает завершения всех переданных задач до выхода из вызова."),

        ("9. Что из себя представляет oneTBB task_group?",
         "Класс oneapi::tbb::task_group представляет динамическую группу асинхронных задач. Позволяет "
         "произвольное число раз добавлять задачи методом run(func) (в том числе из циклов или рекурсивно), "
         "поддерживает отмену группы cancel() и структурированное ожидание завершения."),

        ("10. Что необходимо сделать для ожидания завершения задач при работе с oneTBB task_group?",
         "Необходимо явно вызвать метод tg.wait() (или tg.run_and_wait(func) для последней задачи) до "
         "уничтожения объекта task_group; в противном случае деструктор прервёт выполнение с исключением "
         "missing_wait."),

        ("11. Приведите пример использования task_group и parallel_invoke.",
         "Для task_group: tbb::task_group tg; tg.run([&]{ f1(); }); tg.run([&]{ f2(); }); tg.wait(); "
         "Для parallel_invoke: tbb::parallel_invoke([&]{ f1(); }, [&]{ f2(); }, [&]{ f3(); }); "
         "(ожидание встроено в вызов)."),

        ("12. Охарактеризуйте oneTBB parallel_reduce.",
         "Шаблонный алгоритм параллельной редукции (свёртки) по диапазону Range для ассоциативной операции "
         "(сумма, минимум, максимум и т.п.). Имеет две формы: императивную (с передачей объекта-класса "
         "с методами operator() и join()) и функциональную (с начальным значением identity, лямбдой/функтором "
         "вычисления и функцией объединения reduction)."),

        ("13. Опишите класс для реализации редукционной операции oneTBB.",
         "Класс должен содержать поля для накопления частичного результата и ссылок на входные данные, "
         "основной конструктор, разделяющий конструктор ClassName(const ClassName& other, tbb::split) "
         "(копирующий указатели и инициализирующий аккумулятор нейтральным значением), метод "
         "void operator()(const Range& r) для обработки поддиапазона и метод void join(const ClassName& rhs) "
         "для объединения результатов двух подзадач."),

        ("14. Каким образом можно реализовать выполнение разнородных редукционных операций одновременно?",
         "Через единый класс редукции (или структуру состояния в функциональной форме parallel_reduce), "
         "содержащий сразу несколько полей-аккумуляторов (например, minM2, maxM2, minR, maxR, minU, maxU, "
         "minE, maxE — как в задании 4.3, либо minEl и maxEl в задании 4.1). В одном проходе operator() "
         "обновляются все поля, а в join() каждое поле объединяется по своему правилу (min, max, +), что "
         "избегает повторного чтения больших массивов из памяти."),
    ]
    for q, a in qa:
        doc.add_paragraph(q, align="left", indent_cm=1.25, bold=True, size_pt=14.0, space_before_pt=5, space_after_pt=1, keep_next=True)
        doc.add_paragraph(a, align="both", indent_cm=1.25, size_pt=14.0, space_after_pt=2)

    out_docx = "Lab4/БПИ24-02. Пузынин. Лаб. № 4.docx"
    doc.save(out_docx)
    print(f"Successfully generated compact GOST report: {out_docx}")


if __name__ == "__main__":
    main()
