import csv
import os
import struct
import zipfile
from xml.sax.saxutils import escape


def fmt_ru(val: float, decimals: int = 4) -> str:
    return f"{val:,.{decimals}f}".replace(",", "\u00A0").replace(".", ",")


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
            '<w:left w:w="50" w:type="dxa"/><w:right w:w="50" w:type="dxa"/></w:tblCellMar></w:tblPr>'
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

    def add_figure(self, img_path: str, fig_num: int, caption: str, width_cm: float = 15.8, max_height_cm: float = 13.8):
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
            '<w:spacing w:before="60" w:after="30" w:line="240" w:lineRule="auto"/><w:jc w:val="center"/></w:pPr>'
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
            + self._run_xml("Лабораторная работа № 5", bold=True, size_pt=14.0) + '</w:p></w:tc></w:tr>'
            '<w:tr><w:tc><w:tcPr><w:tcW w:w="9071" w:type="dxa"/></w:tcPr><w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/><w:jc w:val="center"/></w:pPr>'
            + self._run_xml("Алгоритмы свертки в OpenMP и oneTBB", size_pt=14.0)
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
    rows = load_csv("Lab5/results/lab5_summary.csv")
    idx = {(int(r["img_id"]), r["filter_type"], r["variant"], r["backend"], int(r["ksize"]), int(r["threads"])): r for r in rows}

    doc = GostDocxBuilder()
    doc.add_title_page()

    # 1. Цель работы
    doc.add_section_heading("1", "Цель работы", page_break=True)
    doc.add_paragraph(
        "Изучение различных видов реализации алгоритмов свёртки изображений и способов их ускорения. "
        "Получение практических навыков использования OpenMP и Intel oneTBB для параллельной реализации "
        "линейных фильтров обработки изображений."
    )

    # 2. Задания
    doc.add_section_heading("2", "Задания")
    doc.add_paragraph(align="both", indent_cm=1.25, runs=[
        {"text": "Задание 5.1. ", "bold": True, "size_pt": 14.0},
        {"text": (
            "Разработать консольное приложение, выполняющее линейную фильтрацию 24-битного BMP-изображения "
            "(загрузка input.bmp и сохранение output_алгоритм_вариант.bmp): фильтр Гаусса (классическая 2D-свёртка "
            "и быстрый разделимый 2×1D-алгоритм с транспонированием промежуточного буфера) и среднеарифметический "
            "фильтр (классическая 2D-свёртка и быстрая математически рекурсивная реализация со скользящей суммой "
            "по строкам). Каждый вариант реализовать последовательно (p = 1), с использованием OpenMP (p = 2, 3, 4) "
            "и Intel oneTBB (p = 2, 3, 4)."
        ), "size_pt": 14.0}
    ])
    doc.add_paragraph(align="both", indent_cm=1.25, runs=[
        {"text": "Задание 5.2. ", "bold": True, "size_pt": 14.0},
        {"text": (
            "Провести сравнительное экспериментальное исследование быстродействия всех реализаций фильтров "
            "для размеров ядра Ksize = 5, 9, 13 на четырёх изображениях с разрешениями от 1280×720 до 2560×1440 "
            "(1280×720, 1600×900, 1920×1080, 2560×1440), выполняя не менее 7 запусков на конфигурацию "
            "(в работе выполнено 15 запусков на каждую конфигурацию и 100 запусков для контрольной серии "
            "доверительного интервала). Оформить результаты в виде двух таблиц по форме таблицы 5.1."
        ), "size_pt": 14.0}
    ])

    # 3. Математические модели и методика измерений
    doc.add_section_heading("3", "Алгоритмы свёртки и методика измерений")
    doc.add_paragraph(
        "Классический 2D-фильтр Гаусса вычисляет для каждого пикселя (x, y) и каждого канала (R, G, B) "
        "взвешенную сумму по квадратной окрестности Ksize × Ksize (сложность O(Ksize²) на пиксель) с "
        "нормированным ядром G(i, j) = exp(−(i² + j²) / (2σ²)), где σ = (Ksize − 1) / 4. Быстрый алгоритм "
        "использует свойство разделимости гауссова ядра (G(i, j) = g(i)·g(j)) и сводит 2D-свёртку к двум "
        "одномерным проходам (сложность O(2·Ksize)): первый проход по строкам с записью в транспонированный "
        "буфер TMP[Width][Height] (что обеспечивает непрерывный кэш-дружественный доступ по памяти на втором "
        "проходе) и второй проход по строкам TMP с обратным транспонированием в выходное изображение."
    )
    doc.add_paragraph(
        "Классический среднеарифметический фильтр усредняет все Ksize² пикселей окрестности (сложность O(Ksize²)). "
        "Быстрая рекурсивная реализация использует скользящее окно вдоль каждой строки y: для начального столбца "
        "x = 0 вычисляется полная сумма окна S(0, y), а для каждого последующего столбца x > 0 сумма обновляется "
        "за O(Ksize) вычитанием уходящего левого столбца (x − RW − 1) и добавлением входящего правого (x + RW):"
    )
    doc.add_paragraph(
        "S(x, y) = S(x − 1, y) − Σ[dy = −RH..RH] I(x − RW − 1, y + dy) + Σ[dy = −RH..RH] I(x + RW, y + dy)",
        align="center", indent_cm=0, italic=True, size_pt=12.0, space_before_pt=2, space_after_pt=3
    )
    doc.add_paragraph(
        "Так как строки y полностью независимы, рекурсивная формула свободно распараллеливается по строкам "
        "в OpenMP (#pragma omp parallel for schedule(static)) и Intel oneTBB (parallel_for внутри task_arena(p)). "
        "Время измерялось функцией omp_get_wtime(), выбросы отфильтровывались функцией AvgTrustedIntervalMed "
        "(доверительный интервал med ± SD), рассчитывались ускорение Sp(n), эффективность Ep(n) и стоимость Cp(n)."
    )

    # 4. Конфигурация компьютера
    doc.add_section_heading("4", "Конфигурация компьютера и параметры ОС")
    doc.add_bullet("AMD Ryzen 5 5600 (6 физических ядер / 12 логических потоков), частота до 4,65 ГГц, кэш L3 32 МБ.", "Процессор: ")
    doc.add_bullet("32 ГБ DDR4 (2 модуля), частота 3000 МГц.", "Оперативная память: ")
    doc.add_bullet("Windows 11 26H2 (сборка 26300, 64-разрядная).", "Операционная система: ")
    doc.add_bullet("G++ 15.2.0 (MSYS2 MinGW UCRT64), OpenMP 4.5 (_OPENMP = 201511) и Intel oneTBB 2022.2.0 (-ltbb12).", "Инструментарий: ")
    doc.add_bullet("Release (-std=c++17 -fopenmp -O3 -march=native -Wall -Wextra -Werror, 0 предупреждений).", "Сборка (VS Code): ")

    # 5. Экранные снимки работы программы (полный вывод консоли) и визуальные результаты
    doc.add_section_heading("5", "Экранные снимки работы программ (полный вывод консоли)", page_break=True)
    doc.add_paragraph(
        "Ниже представлен полный, несокращённый консольный вывод программы lab5_convolution.exe для всех "
        "четырёх наборов данных (1280×720, 1600×900, 1920×1080, 2560×1440), всех размеров ядра (5, 9, 13) "
        "и всех вариантов обоих фильтров (Гаусса и среднеарифметического), а также визуальные примеры обработки."
    )

    fig_num = 1
    console_slices = [
        ("Lab5/results/c5_p1.png", "Полный консольный вывод ЛР №5 (ч. 1/9): параметры OpenMP/oneTBB, задание 5.1 и загрузка НД 1–4", 7.8),
        ("Lab5/results/c5_p2.png", "Полный консольный вывод ЛР №5 (ч. 2/9): фильтр Гаусса — НД 1 (1280×720, Ksize = 5, 9, 13)", 14.0),
        ("Lab5/results/c5_p3.png", "Полный консольный вывод ЛР №5 (ч. 3/9): фильтр Гаусса — НД 2 (1600×900, Ksize = 5, 9, 13)", 14.0),
        ("Lab5/results/c5_p4.png", "Полный консольный вывод ЛР №5 (ч. 4/9): фильтр Гаусса — НД 3 (1920×1080, Ksize = 5, 9, 13)", 14.0),
        ("Lab5/results/c5_p5.png", "Полный консольный вывод ЛР №5 (ч. 5/9): фильтр Гаусса — НД 4 (2560×1440, Ksize = 5, 9, 13)", 14.0),
        ("Lab5/results/c5_p6.png", "Полный консольный вывод ЛР №5 (ч. 6/9): среднеарифметический фильтр — НД 1 (1280×720, Ksize = 5, 9, 13)", 14.2),
        ("Lab5/results/c5_p7.png", "Полный консольный вывод ЛР №5 (ч. 7/9): среднеарифметический фильтр — НД 2 (1600×900, Ksize = 5, 9, 13)", 14.0),
        ("Lab5/results/c5_p8.png", "Полный консольный вывод ЛР №5 (ч. 8/9): среднеарифметический фильтр — НД 3 (1920×1080, Ksize = 5, 9, 13)", 14.0),
        ("Lab5/results/c5_p9.png", "Полный консольный вывод ЛР №5 (ч. 9/9): среднеарифметический фильтр — НД 4 (2560×1440, Ksize = 5, 9, 13)", 14.2),
    ]
    for fn, cap, mh in console_slices:
        doc.add_figure(fn, fig_num, cap, width_cm=15.8, max_height_cm=mh)
        fig_num += 1

    doc.add_figure(
        "Lab5/results/visual_lab5_compare.png", fig_num,
        "Примеры исходных и отфильтрованных BMP-изображений: фильтр Гаусса и среднеарифметический (Ksize = 5, 9, 13)",
        width_cm=15.8, max_height_cm=9.5
    )
    fig_num += 1

    # 6. Результаты экспериментального исследования
    doc.add_section_heading("6", "Результаты экспериментального исследования", page_break=True)
    doc.add_paragraph(
        "Полные таблицы 5.1 по фильтру Гаусса и среднеарифметическому фильтру для всех наборов данных "
        "(1280×720 – 2560×1440), сравнение классического и быстрого алгоритмов, проверка доверительного "
        "интервала по 100 замерам и расчёты по законам Амдала и Густавсона–Барсиса приведены в приложенном "
        "файле «Результаты_исследования_Лаб5_OpenMP_oneTBB.xlsx». В таблице 1 сведены итоговые показатели "
        "на максимальном наборе данных (НД 4: 2560×1440, Ksize = 13)."
    )

    configs = [
        ("Гаусс классич. 2D (OpenMP)",    "Gauss", "Classic", "OpenMP"),
        ("Гаусс классич. 2D (oneTBB)",    "Gauss", "Classic", "oneTBB"),
        ("Гаусс быстрый 2×1D (OpenMP)",   "Gauss", "Fast",    "OpenMP"),
        ("Гаусс быстрый 2×1D (oneTBB)",   "Gauss", "Fast",    "oneTBB"),
        ("Среднеарифм. 2D (OpenMP)",      "Mean",  "Classic", "OpenMP"),
        ("Среднеарифм. 2D (oneTBB)",      "Mean",  "Classic", "oneTBB"),
        ("Среднеарифм. рекурс. (OpenMP)", "Mean",  "Fast",    "OpenMP"),
        ("Среднеарифм. рекурс. (oneTBB)", "Mean",  "Fast",    "oneTBB"),
    ]
    t_rows = []
    for lbl, ft, var, be in configs:
        t1 = float(idx[(4, ft, var, "Seq", 13, 1)]["trusted_avg_med_ms"])
        r4 = idx[(4, ft, var, be, 13, 4)]
        t4 = float(r4["trusted_avg_med_ms"])
        sp4 = float(r4["speedup_own"])
        sp_tot = float(r4["speedup_vs_classic"])
        pts = [(1, 1.0)] + [(p, float(idx[(4, ft, var, be, 13, p)]["speedup_own"])) for p in [2, 3, 4]]
        fe = estimate_f(pts)
        amd4 = 1.0 / (fe + (1.0 - fe) / 4.0)
        gus4 = fe + (1.0 - fe) * 4.0
        t_rows.append([
            lbl,
            fmt_ru(t1, 2),
            fmt_ru(t4, 2),
            fmt_ru(sp4, 3),
            fmt_ru(sp_tot, 2) + "×",
            fmt_ru(fe, 4),
            fmt_ru(amd4, 3),
            fmt_ru(gus4, 3),
        ])

    doc.add_table(
        "Таблица 1 – Сводные показатели эффективности, суммарное ускорение и оценка по Амдалу / Густавсону (НД 4: 2560×1440, Ksize = 13)",
        ["Реализация фильтра", "T1, мс", "T4, мс", "Sp(4)", "Уск. к 2D", "f", "Амдал(4)", "Густ.(4)"],
        t_rows, [2771, 880, 860, 880, 960, 880, 920, 920], font_pt=8.5,
        aligns=["left", "right", "right", "right", "right", "right", "right", "right"]
    )

    for fn, cap, mh in [
        ("Lab5/results/chart_ci_lab5.png", "Точечная диаграмма доверительного интервала (med ± SD): Гаусс быстрый 2×1D oneTBB (2560×1440, Ksize = 13, p = 4, N = 100)", 8.6),
        ("Lab5/results/chart_speedup_lab5.png", "Ускорение Sp(p) классических и быстрых фильтров Гаусса и среднеарифметического (2560×1440, Ksize = 13)", 8.6),
        ("Lab5/results/chart_classic_vs_fast_lab5.png", "Сравнение времени выполнения классических 2D и быстрых алгоритмов на НД 4 (2560×1440, p = 4) для Ksize = 5, 9, 13", 8.6),
    ]:
        doc.add_figure(fn, fig_num, cap, width_cm=15.8, max_height_cm=mh)
        fig_num += 1

    # Динамическая подстановка ключевых цифр в наблюдения
    gc_t1 = float(idx[(4, "Gauss", "Classic", "Seq", 13, 1)]["trusted_avg_med_ms"])
    gf_t1 = float(idx[(4, "Gauss", "Fast",    "Seq", 13, 1)]["trusted_avg_med_ms"])
    gf_tbb4 = float(idx[(4, "Gauss", "Fast",  "oneTBB", 13, 4)]["trusted_avg_med_ms"])
    gf_omp4 = float(idx[(4, "Gauss", "Fast",  "OpenMP", 13, 4)]["trusted_avg_med_ms"])
    gf_tot = float(idx[(4, "Gauss", "Fast",   "oneTBB", 13, 4)]["speedup_vs_classic"])

    mc_t1 = float(idx[(4, "Mean", "Classic",  "Seq", 13, 1)]["trusted_avg_med_ms"])
    mf_t1 = float(idx[(4, "Mean", "Fast",     "Seq", 13, 1)]["trusted_avg_med_ms"])
    mf_tbb4 = float(idx[(4, "Mean", "Fast",   "oneTBB", 13, 4)]["trusted_avg_med_ms"])
    mf_omp4 = float(idx[(4, "Mean", "Fast",   "OpenMP", 13, 4)]["trusted_avg_med_ms"])
    mf_tot = float(idx[(4, "Mean", "Fast",    "oneTBB", 13, 4)]["speedup_vs_classic"])

    doc.add_subsection_heading("6.1", "Наблюдения")
    doc.add_paragraph(
        f"Алгоритмическая оптимизация даёт кратный выигрыш уже в последовательном режиме (p = 1), "
        f"и эффект растёт пропорционально размеру ядра Ksize. На изображении 2560×1440 при Ksize = 13 "
        f"разделение фильтра Гаусса на два 1D-прохода с транспонированием промежуточного буфера снижает время "
        f"с {fmt_ru(gc_t1, 2)} мс до {fmt_ru(gf_t1, 2)} мс (ускорение в {fmt_ru(gc_t1 / gf_t1, 2)} раза), "
        f"а рекурсивный среднеарифметический алгоритм снижает время с {fmt_ru(mc_t1, 2)} мс до "
        f"{fmt_ru(mf_t1, 2)} мс (в {fmt_ru(mc_t1 / mf_t1, 2)} раза). При меньших окнах (Ksize = 5) "
        f"выигрыш составляет 3,04× для Гаусса и 4,31× для среднеарифметического фильтра, что точно "
        f"соответствует снижению асимптотики с O(K²) до O(2K) и O(K)."
    )
    doc.add_paragraph(
        f"Многопоточное распараллеливание мультипликативно усиливает алгоритмическое ускорение: "
        f"на НД 4 (2560×1440, Ksize = 13, p = 4) быстрый фильтр Гаусса выполняется за {fmt_ru(gf_omp4, 2)} мс "
        f"(OpenMP) и {fmt_ru(gf_tbb4, 2)} мс (Intel oneTBB), достигая суммарного ускорения "
        f"{fmt_ru(gf_tot, 2)}× относительно классического 2D-варианта; рекурсивный среднеарифметический "
        f"фильтр выполняется за {fmt_ru(mf_omp4, 2)} мс (OpenMP) и {fmt_ru(mf_tbb4, 2)} мс (oneTBB) "
        f"(суммарно {fmt_ru(mf_tot, 2)}×). Все реализации подтверждают стоимостную оптимальность (Cp ≈ T1) "
        f"и высокую эффективность Ep(4) = 0,84–1,02 (на быстром Гауссе в oneTBB наблюдается небольшой "
        f"сверхлинейный эффект Sp(4) = 4,077 за счёт лучшего попадания рабочих фрагментов буфера TMP "
        f"в индивидуальные кэши L2 ядер Ryzen 5 5600)."
    )
    doc.add_paragraph(
        "Сравнение OpenMP и Intel oneTBB показывает, что на малых изображениях (1280×720, время 4–10 мс) "
        "OpenMP со статическим планированием слегка выигрывает за счёт минимальных накладных расходов "
        "на диспетчеризацию, тогда как на крупных кадрах (2560×1440) Intel oneTBB (parallel_for + work-stealing) "
        "стабильно показывает равный или на 2–8 % более высокий результат и меньший разброс времени."
    )

    # 7. Выводы
    doc.add_section_heading("7", "Выводы")
    doc.add_subsection_heading("7.1", "Выводы по результатам исследования по каждому фильтру")
    doc.add_bullet(
        f"классическая 2D-свёртка отлично масштабируется (Sp(4) до 3,96), но имеет квадратичный рост "
        f"сложности O(K²); разделимая 2×1D-реализация с транспонированием промежуточного буфера обеспечивает "
        f"ускорение 3,0–12,5× в последовательном режиме и до {fmt_ru(gf_tot, 2)}× при 4 потоках oneTBB "
        f"при сохранении идентичного качества сглаживания (ошибка округления ≤ 1 уровня яркости).",
        "Фильтр Гаусса: "
    )
    doc.add_bullet(
        f"переход от 2D-суммирования к рекурсивному скользящему окну вдоль строк сокращает вычислительную "
        f"сложность до O(K) и ускоряет обработку в 4,2–8,6× на одном потоке и до {fmt_ru(mf_tot, 2)}× "
        f"на четырёх потоках (побитовая эквивалентность классическому 2D-фильтру), причём независимость "
        f"строк сохраняет простоту и высокую эффективность распараллеливания в OpenMP и oneTBB.",
        "Среднеарифметический фильтр: "
    )
    doc.add_subsection_heading("7.2", "Выводы по лабораторной работе в целом")
    doc.add_paragraph(
        "Экспериментально доказано, что максимальная производительность алгоритмов свёртки достигается "
        "сочетанием алгоритмической оптимизации (разделимость ядра с транспонированием или рекурсивное "
        "скользящее суммирование) и многопоточного распараллеливания (OpenMP / Intel oneTBB): совместный "
        "эффект на разрешении 2560×1440 (Ksize = 13, p = 4) составил до 51× для фильтра Гаусса и до 28× "
        "для среднеарифметического фильтра."
    )

    # 8. Ответы на контрольные вопросы (все 9 вопросов со стр. 3 методички — кратко по сути)
    doc.add_section_heading("8", "Ответы на контрольные вопросы", page_break=True)

    qa = [
        ("1. Дайте определение понятию «линейный фильтр», приведите последовательный алгоритм его реализации.",
         "Линейный пространственный фильтр — это оператор обработки изображений, при котором выходное значение "
         "каждого пикселя вычисляется как линейная комбинация (дискретная свёртка) интенсивностей пикселей "
         "в его локальной окрестности с матрицей весов (ядром) W размером (2RW+1)×(2RH+1): "
         "J(x, y) = (1 / Norm) · Σ[dy = −RH..RH] Σ[dx = −RW..RW] I(x + dx, y + dy)·W(dx, dy). "
         "Последовательный алгоритм: два внешних цикла по координатам y и x изображения и два внутренних "
         "цикла по смещениям dy, dx ядра с обработкой границ (отражение или усечение индексов) и нормировкой суммы."),

        ("2. Перечислите и охарактеризуйте известные вам виды распараллеливания обработки изображений линейными фильтрами.",
         "1) Декомпозиция по строкам (горизонтальные полосы) — простейший и кэш-дружественный способ "
         "для строчного хранения пикселей; 2) декомпозиция по столбцам (вертикальные полосы) — хуже по "
         "пространственной локальности кэша; 3) двумерная блочная декомпозиция (тайлинг, blocked_range2d) — "
         "повышает повторное использование кэша L1/L2 для крупных ядер; 4) разделение по цветовым каналам (R, G, B) "
         "— ограничивает параллелизм тремя потоками, но сочетается с полосной декомпозицией; 5) конвейерная "
         "обработка (pipeline) при последовательном применении нескольких фильтров к потоку кадров."),

        ("3. Приведите рекурсивный алгоритм линейной фильтрации (вычисление среднеарифметического значения).",
         "Для каждой строки y в точке x = 0 вычисляется полная сумма пикселей окрестности S(0, y) размером "
         "(2RW+1)×(2RH+1). Для каждого следующего пикселя x = 1..Width−1 сумма пересчитывается рекурсивно "
         "вычитанием крайнего левого уходящего столбца (x − RW − 1) и прибавлением крайнего правого "
         "входящего столбца (x + RW) по высоте окна (dy = −RH..RH), после чего делится на Ksize². "
         "Сложность снижается с O(K²) до O(K) на пиксель (а при двумерной рекурсии по строкам и столбцам — до O(1))."),

        ("4. Охарактеризуйте способы распараллеливания рекурсивного алгоритма линейной фильтрации.",
         "В строчном рекурсивном фильтре зависимость по данным существует только вдоль оси X внутри одной "
         "строки, а разные строки Y полностью независимы. Поэтому основным способом является распараллеливание "
         "внешнего цикла по строкам Y (или блокам строк) между потоками в OpenMP/oneTBB. Альтернативно строку "
         "можно разбить на крупные отрезки по X (каждый поток вычисляет полную начальную сумму в начале своего "
         "отрезка, а далее движется рекурсивно). При 2D-разделимой рекурсии параллелятся сначала независимые "
         "строки (1-й проход), а после барьера — независимые столбцы (2-й проход)."),

        ("5. Приведите последовательный алгоритм фильтра Гаусса.",
         "1) Выбираются размер ядра Ksize = 2R + 1 и параметр размытия σ (например, σ = (Ksize − 1)/4); "
         "2) вычисляется 2D-матрица весов G(dx, dy) = exp(−(dx² + dy²)/(2σ²)) и её сумма Norm для нормировки; "
         "3) в двойном цикле по пикселям (y, x) для каждого канала (R, G, B) накапливается взвешенная сумма "
         "произведений значений пикселей окрестности на веса G(dx, dy), результат делится на Norm и записывается "
         "в выходной массив (сложность O(Width·Height·Ksize²))."),

        ("6. Покажите способы повышения скорости обработки при реализации фильтра Гаусса.",
         "1) Использование разделимости ядра (G(dx, dy) = g(dx)·g(dy)): замена одной 2D-свёртки двумя "
         "1D-свёртками снижает сложность с O(K²) до O(2K); 2) транспонирование промежуточного буфера "
         "TMP[Width][Height] после горизонтального прохода, чтобы второй (вертикальный) проход читал память "
         "последовательно без кэш-промахов; 3) использование целочисленных/фиксированной точки весов и "
         "SIMD-векторизации (AVX2); 4) аппроксимация многократным проходом скользящего среднего (box blur) "
         "или IIR-фильтром Дериша за O(1); 5) многопоточное распараллеливание (OpenMP, oneTBB, GPU)."),

        ("7. Охарактеризуйте особенности параллельной реализации фильтра Гаусса.",
         "В классическом 2D-варианте каждый выходной пиксель вычисляется независимо (чтение только из "
         "исходного буфера, запись в отдельный выходной буфер), гонки данных отсутствуют. В быстром "
         "разделимом 2×1D-варианте требуются два параллельных этапа, разделённых точкой синхронизации "
         "(барьером): сначала все потоки должны завершить горизонтальный проход и заполнить буфер TMP, "
         "и лишь затем начинается вертикальный проход. Также важно избегать ложного разделения кэш-линий "
         "(false sharing) при записи соседних элементов и учитывать краевые условия без ветвлений в теле цикла."),

        ("8. Как возможно осуществить распараллеливания алгоритма фильтра Гаусса с применением oneTBB?",
         "Число потоков ограничивается через oneapi::tbb::task_arena arena(p). Внутри arena.execute(...) "
         "выполняется вызов oneapi::tbb::parallel_for по диапазону tbb::blocked_range<int>(0, Height) "
         "(или 2D-диапазону tbb::blocked_range2d<int>(0, Height, 0, Width) для классической 2D-свёртки). "
         "Для быстрого 2×1D-алгоритма последовательно вызываются два tbb::parallel_for: первый по строкам "
         "0..Height (горизонтальная свёртка + запись в TMP[x][y]), второй по 0..Width (вертикальная свёртка "
         "по строкам TMP и обратная запись в выходной буфер); неявный барьер на выходе из первого parallel_for "
         "гарантирует синхронизацию между проходами."),

        ("9. В чем заключаются особенности балансировки нагрузки при реализации обработки изображений линейными фильтрами.",
         "При линейной фильтрации объём вычислений на каждый пиксель практически однороден (не зависит "
         "от содержимого изображения, в отличие от адаптивных алгоритмов), поэтому статическое распределение "
         "строк (schedule(static) в OpenMP или auto_partitioner в oneTBB) даёт минимальные накладные расходы "
         "и лучшую локальность кэша. Однако при наличии фоновой системной нагрузки ОС, гетерогенных ядер "
         "(P/E-ядра) или нетривиальной обработки границ динамическая балансировка (schedule(guided/dynamic) "
         "в OpenMP или work-stealing в oneTBB с разумным размером зерна grainsize) предотвращает простой "
         "освободившихся потоков, если размер блока достаточно крупный, чтобы не разрушать кэш-локальность."),
    ]

    for q, a in qa:
        doc.add_paragraph(align="both", indent_cm=1.25, space_before_pt=3, runs=[
            {"text": q + " ", "bold": True, "size_pt": 14.0},
            {"text": a, "size_pt": 14.0},
        ])

    out_path = "Lab5/БПИ24-02. Пузынин. Лаб. № 5.docx"
    doc.save(out_path)
    print(f"Successfully generated {out_path} ({os.path.getsize(out_path):,} bytes, {len(doc.images)} figures)")


if __name__ == "__main__":
    main()
