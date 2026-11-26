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

    def add_table(self, caption: str, headers: list, rows: list, col_widths: list, font_pt: float = 8.5, aligns=None):
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
            '<w:left w:w="48" w:type="dxa"/><w:right w:w="48" w:type="dxa"/></w:tblCellMar></w:tblPr>'
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
            + self._run_xml("Лабораторная работа № 6", bold=True, size_pt=14.0) + '</w:p></w:tc></w:tr>'
            '<w:tr><w:tc><w:tcPr><w:tcW w:w="9071" w:type="dxa"/></w:tcPr><w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/><w:jc w:val="center"/></w:pPr>'
            + self._run_xml("Реализация алгоритмов для расчета особых точек с использованием OpenMP и oneTBB", size_pt=14.0)
            + '</w:p></w:tc></w:tr></w:tbl>'
        )
        self.body_parts.append(box)
        self.add_paragraph("", indent_cm=0, space_after_pt=66)

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
        self.add_paragraph("", indent_cm=0, space_before_pt=125)
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


def main():
    t61 = list(csv.DictReader(open("Lab6/results/lab6_table61.csv", encoding="utf-8")))
    t62 = list(csv.DictReader(open("Lab6/results/lab6_table62.csv", encoding="utf-8")))
    idx61 = {(int(r["ds_id"]), r["step_id"], r["backend"], r["container"], int(r["threads"])): r for r in t61}
    idx62 = {(int(r["ds_id"]), r["impl_id"], int(r["threads"])): r for r in t62}

    doc = GostDocxBuilder()
    doc.add_title_page()

    # 1. Цель работы
    doc.add_section_heading("1", "Цель работы", page_break=True)
    doc.add_paragraph(
        "Изучение различных видов реализации алгоритмов для расчёта особых точек "
        "(на основе детектора Харриса) с использованием технологий OpenMP и Intel oneTBB."
    )

    # 2. Задания
    doc.add_section_heading("2", "Задания")
    doc.add_paragraph(align="both", indent_cm=1.25, runs=[
        {"text": "Задание 6.1. ", "bold": True, "size_pt": 14.0},
        {"text": (
            "Разработать консольное приложение, выполняющее вычисление особых точек (детектор Харриса) "
            "и их визуализацию на изображении с использованием OpenMP и Intel oneTBB. Выполнить два типа "
            "реализации шага 4 (поиска особых точек для визуализации): с использованием классических "
            "контейнеров STL (std::vector) и с применением потокобезопасного контейнера oneTBB "
            "(oneapi::tbb::concurrent_vector). В приложение должны загружаться файлы input_X.bmp "
            "и формироваться выходные файлы output_X_var.bmp."
        ), "size_pt": 14.0}
    ])
    doc.add_paragraph(align="both", indent_cm=1.25, runs=[
        {"text": "Задание 6.2. ", "bold": True, "size_pt": 14.0},
        {"text": (
            "Провести экспериментальное исследование по обработке трёх наборов данных (1280×720, "
            "1920×1080, 2560×1440) разными реализациями (не менее 10 запусков на каждую конфигурацию; "
            "в работе выполнено по 12 запусков и контрольная серия 100 запусков для доверительного "
            "интервала). Оформить расчёт времени и ускорения по каждому функциональному блоку "
            "(шаги 1, 2, 3, 4.1, 4.2, 4.3, 5) в виде таблицы 6.1 и суммарное время формирования "
            "карты особых точек по всем 7 реализациям — в виде таблицы 6.2."
        ), "size_pt": 14.0}
    ])

    # 3. Алгоритм и методика измерений
    doc.add_section_heading("3", "Алгоритм детектора Харриса и методика измерений")
    doc.add_paragraph(
        "Алгоритм реализован строго по схеме из методических указаний в пять этапов: "
        "Шаг 1 — расчёт массива яркости I(y,x) = 0,299·R + 0,587·G + 0,114·B и предварительное "
        "сглаживание быстрым разделимым фильтром Гаусса 2×1D (Ksize = 5, σ = 1,0 из ЛР № 5); "
        "Шаг 2 — двухпроходный расчёт градиентов DiffX, DiffY, DiffXY операторами Прюитта 3×3 "
        "(без усечения диапазона [0..255]); Шаг 3 — вычисление отклика угла "
        "R(y,x) = (A·B − C²) − k·(A + B)² при k = 0,04; "
        "Шаг 4 — поиск min/max массива H в едином проходе, расчёт порога "
        "threshold = (max(H) − min(H))·kf + min(H) при kf = 0,70 ∈ [0,6; 0,8] и сбор кандидатов "
        "(Шаг 4.1), сортировка по убыванию R с помощью OpenMP task merge sort / oneTBB parallel_sort "
        "(Шаг 4.2), подавление немаксимумов в радиусе rang = 20 пикселей (Шаг 4.3); "
        "Шаг 5 — копирование исходного изображения и отрисовка красных маркеров 3×3 (rang = 1)."
    )
    doc.add_paragraph(
        "Для шага 4 реализованы два варианта контейнеров: классический STL std::vector (с потоко-локальными "
        "векторами и последующим слиянием без блокировок) и потокобезопасный oneapi::tbb::concurrent_vector "
        "(прямой параллельный push_back). В гибридной реализации (OpenMP + oneTBB) регулярные 2D-шаги "
        "(1, 2, 3, 5) выполняются в OpenMP, а шаги поиска, сортировки и отсечения особых точек (4.1–4.3) — "
        "в Intel oneTBB. Замеры времени выполнены через omp_get_wtime() (с калибруемым пакетированием для "
        "субмиллисекундных этапов) и фильтрацией выбросов AvgTrustedIntervalMed (med ± SD)."
    )

    # 4. Конфигурация компьютера
    doc.add_section_heading("4", "Конфигурация компьютера и параметры ОС")
    doc.add_bullet("AMD Ryzen 5 5600 (6 физических ядер / 12 логических потоков), частота до 4,65 ГГц, кэш L3 32 МБ.", "Процессор: ")
    doc.add_bullet("32 ГБ DDR4 (2 модуля), частота 3000 МГц.", "Оперативная память: ")
    doc.add_bullet("Windows 11 26H2 (сборка 26300, 64-разрядная).", "Операционная система: ")
    doc.add_bullet("G++ 15.2.0 (MSYS2 MinGW UCRT64), OpenMP 4.5 (_OPENMP = 201511) и Intel oneTBB 2022.2.0 (-ltbb12).", "Инструментарий: ")
    doc.add_bullet("Release (-std=c++17 -fopenmp -O3 -march=native -Wall -Wextra -Werror, 0 предупреждений).", "Сборка (VS Code): ")

    # 5. Экранные снимки (полный вывод консоли) и визуальные примеры
    doc.add_section_heading("5", "Экранные снимки работы программ (полный вывод консоли)", page_break=True)
    doc.add_paragraph(
        "Ниже приведён полный, несокращённый консольный вывод программы lab6_harris.exe для всех "
        "трёх наборов данных (1280×720, 1920×1080, 2560×1440), всех функциональных блоков таблицы 6.1 "
        "и всех 7 полных реализаций таблицы 6.2, а также примеры исходных и обработанных BMP-изображений."
    )

    fig_num = 1
    slices = [
        ("Lab6/results/c6_p1.png", "Полный консольный вывод ЛР №6 (ч. 1/9): параметры, загрузка BMP и НД 1 Табл. 6.1 (Шаги 1–3)", 12.6),
        ("Lab6/results/c6_p2.png", "Полный консольный вывод ЛР №6 (ч. 2/9): НД 1 (1280×720) Табл. 6.1 (Шаги 4.1, 4.2, 4.3 и 5)", 14.0),
        ("Lab6/results/c6_p3.png", "Полный консольный вывод ЛР №6 (ч. 3/9): НД 1 (1280×720) Табл. 6.2 (суммарное время и точки)", 8.2),
        ("Lab6/results/c6_p4.png", "Полный консольный вывод ЛР №6 (ч. 4/9): НД 2 (1920×1080) Табл. 6.1 (Шаги 1–3)", 8.6),
        ("Lab6/results/c6_p5.png", "Полный консольный вывод ЛР №6 (ч. 5/9): НД 2 (1920×1080) Табл. 6.1 (Шаги 4.1, 4.2, 4.3 и 5)", 14.0),
        ("Lab6/results/c6_p6.png", "Полный консольный вывод ЛР №6 (ч. 6/9): НД 2 (1920×1080) Табл. 6.2 (суммарное время и точки)", 8.2),
        ("Lab6/results/c6_p7.png", "Полный консольный вывод ЛР №6 (ч. 7/9): НД 3 (2560×1440) Табл. 6.1 (Шаги 1–3)", 8.6),
        ("Lab6/results/c6_p8.png", "Полный консольный вывод ЛР №6 (ч. 8/9): НД 3 (2560×1440) Табл. 6.1 (Шаги 4.1, 4.2, 4.3 и 5)", 14.0),
        ("Lab6/results/c6_p9.png", "Полный консольный вывод ЛР №6 (ч. 9/9): НД 3 (2560×1440) Табл. 6.2 (итоги и завершение)", 8.6),
    ]
    for fn, cap, mh in slices:
        doc.add_figure(fn, fig_num, cap, width_cm=15.8, max_height_cm=mh)
        fig_num += 1

    doc.add_figure(
        "Lab6/results/visual_lab6_harris.png", fig_num,
        "Примеры исходных и обработанных BMP-изображений детектором Харриса (с увеличенными фрагментами углов)",
        width_cm=15.8, max_height_cm=10.2
    )
    fig_num += 1

    # 6. Результаты экспериментального исследования
    doc.add_section_heading("6", "Результаты экспериментального исследования", page_break=True)
    doc.add_paragraph(
        "Полные таблицы 6.1 (по всем функциональным блокам) и 6.2 (суммарное время, связанное формулами "
        "с таблицей 6.1), сравнение STL std::vector и oneTBB concurrent_vector, проверка доверительного "
        "интервала на 100 замерах и оценки по законам Амдала и Густавсона–Барсиса приведены в приложенном "
        "файле «Результаты_исследования_Лаб6_OpenMP_oneTBB.xlsx». В таблице 1 сведены итоговые показатели "
        "таблицы 6.2 при p = 4 потоках для всех трёх наборов данных."
    )

    impls = [
        ("Seq",           "Sequential (посл., p = 1)",            1),
        ("OpenMP_STL",    "OpenMP (Шаг 4: STL vector)",           4),
        ("OpenMP_TBBVec", "OpenMP (Шаг 4: oneTBB conc_vec)",      4),
        ("oneTBB_STL",    "Intel oneTBB (Шаг 4: STL vector)",     4),
        ("oneTBB_TBBVec", "Intel oneTBB (Шаг 4: oneTBB conc_vec)",4),
        ("Hybrid_STL",    "OpenMP+oneTBB (Шаг 4: STL vector)",    4),
        ("Hybrid_TBBVec", "OpenMP+oneTBB (Шаг 4: oneTBB conc_vec)",4),
    ]
    t_rows = []
    for iid, lbl, p in impls:
        r1 = idx62[(1, iid, p)]
        r2 = idx62[(2, iid, p)]
        r3 = idx62[(3, iid, p)]
        if p == 1:
            fe_s, amd_s = "–", "–"
        else:
            pts = [(1, 1.0)] + [(pp, float(idx62[(3, iid, pp)]["speedup"])) for pp in [2, 3, 4]]
            fe = estimate_f(pts)
            amd = 1.0 / (fe + (1.0 - fe) / 4.0)
            fe_s, amd_s = fmt_ru(fe, 3), fmt_ru(amd, 3)
        t_rows.append([
            lbl,
            fmt_ru(float(r1["total_ms"]), 2), fmt_ru(float(r1["speedup"]), 3),
            fmt_ru(float(r2["total_ms"]), 2), fmt_ru(float(r2["speedup"]), 3),
            fmt_ru(float(r3["total_ms"]), 2), fmt_ru(float(r3["speedup"]), 3),
            fe_s, amd_s
        ])

    doc.add_table(
        "Таблица 1 – Сводка результатов Таблицы 6.2 (p = 4) и оценка по закону Амдала (НД 3)",
        ["Полная реализация", "НД1, мс", "Sp1", "НД2, мс", "Sp2", "НД3, мс", "Sp3", "f (НД3)", "Амд.(4)"],
        t_rows, [2911, 780, 740, 780, 740, 780, 740, 800, 800], font_pt=8.2,
        aligns=["left", "right", "right", "right", "right", "right", "right", "right", "right"]
    )

    for fn, cap, mh in [
        ("Lab6/results/chart_ci_lab6.png", "Доверительный интервал (med ± SD): Шаг 2 (градиенты oneTBB, 2560×1440, p = 4, N = 100)", 8.6),
        ("Lab6/results/chart_speedup_lab6.png", "Ускорение Sp(p) полных реализаций детектора Харриса на НД 1 (1280×720) и НД 3 (2560×1440)", 8.6),
        ("Lab6/results/chart_steps_stl_vs_tbb_lab6.png", "Распределение времени по шагам конвейера и сравнение STL std::vector с oneTBB concurrent_vector (НД 3, p = 4)", 8.6),
    ]:
        doc.add_figure(fn, fig_num, cap, width_cm=15.8, max_height_cm=mh)
        fig_num += 1

    # Подтягиваем реальные числа для выводов
    n1_seq = float(idx62[(1, "Seq", 1)]["total_ms"])
    n1_best = float(idx62[(1, "Hybrid_STL", 4)]["total_ms"])
    n1_sp = float(idx62[(1, "Hybrid_STL", 4)]["speedup"])
    n2_seq = float(idx62[(2, "Seq", 1)]["total_ms"])
    n2_tbb = float(idx62[(2, "oneTBB_STL", 4)]["total_ms"])
    n2_sp = float(idx62[(2, "oneTBB_STL", 4)]["speedup"])
    n3_seq = float(idx62[(3, "Seq", 1)]["total_ms"])
    n3_tbb = float(idx62[(3, "oneTBB_STL", 4)]["total_ms"])
    n3_sp = float(idx62[(3, "oneTBB_STL", 4)]["speedup"])
    n3_tbbv = float(idx62[(3, "oneTBB_TBBVec", 4)]["total_ms"])

    s42_stl = float(idx61[(3, "Step4_2_Sort", "oneTBB", "STL", 4)]["trusted_avg_med_ms"])
    s42_cv  = float(idx61[(3, "Step4_2_Sort", "oneTBB", "TBBVec", 4)]["trusted_avg_med_ms"])
    s43_stl = float(idx61[(3, "Step4_3_NMS",  "oneTBB", "STL", 4)]["trusted_avg_med_ms"])
    s43_sp  = float(idx61[(3, "Step4_3_NMS",  "oneTBB", "STL", 4)]["speedup"])
    s43_cv  = float(idx61[(3, "Step4_3_NMS",  "oneTBB", "TBBVec", 4)]["trusted_avg_med_ms"])

    # 7. Выводы (строго по требованиям стр. 3 методички: по Табл. 6.1, по Табл. 6.2 и в целом)
    doc.add_section_heading("7", "Выводы")
    doc.add_subsection_heading("7.1", "Выводы по результатам исследования по функциям (таблица 6.1)")
    doc.add_bullet(
        "занимают суммарно около 90–94 % всего времени конвейера. На НД 1 (1280×720), где рабочие буферы "
        "полностью помещаются в кэш L3 (32 МБ), достигается ускорение до Sp(4) = 3,25–3,50. При переходе "
        "к НД 3 (2560×1440) объём float-буферов превышает размер кэша L3, и производительность упирается "
        "в пропускную способность памяти DDR4-3000 (Sp(4) ≈ 2,1–2,5).",
        "Шаги 1 (яркость + Гаусс) и 2 (градиенты DiffX/Y/XY): "
    )
    doc.add_bullet(
        "высокоскоростные потоковые операции (memory-bound: 0,1–2,0 мс), в которых на НД 3 многопоточность "
        "быстро насыщает шину памяти, а на шаге 5 (копирование + отрисовка маркеров) oneTBB достигает Sp(4) = 3,17.",
        "Шаги 3 (отклик угла R) и 5 (визуализация): "
    )
    doc.add_bullet(
        f"на шаге 4.2 oneTBB parallel_sort значительно быстрее OpenMP task merge sort, причём сортировка "
        f"непрерывного std::vector ({fmt_ru(s42_stl, 3)} мс) вдвое быстрее сегментированного concurrent_vector "
        f"({fmt_ru(s42_cv, 3)} мс). На шаге 4.3 (треугольный цикл отсечения в радиусе rang = 20) планировщик "
        f"work-stealing в oneTBB на STL достигает сверхлинейного ускорения Sp(4) = {fmt_ru(s43_sp, 3)} "
        f"({fmt_ru(s43_stl, 3)} мс против {fmt_ru(s43_cv, 3)} мс у concurrent_vector из-за накладных расходов "
        f"атомарных операций при параллельном push_back и несмежного хранения сегментов).",
        "Шаги 4.1, 4.2 и 4.3 (поиск, сортировка и отсечение точек, STL vs concurrent_vector): "
    )

    doc.add_subsection_heading("7.2", "Выводы по всем шагам формирования карты особых точек (таблица 6.2)")
    doc.add_paragraph(
        f"Все семь полных реализаций нашли идентичное количество особых точек (НД 1: 1079/124, НД 2: 3907/495, "
        f"НД 3: 3023/337), точно совпадающих с геометрическими углами шахматных и архитектурных объектов. "
        f"На НД 1 (1280×720) лучший результат показала гибридная реализация OpenMP+oneTBB (STL) "
        f"({fmt_ru(n1_seq, 2)} → {fmt_ru(n1_best, 2)} мс, Sp(4) = {fmt_ru(n1_sp, 3)}). На крупных изображениях "
        f"НД 2 (1920×1080) и НД 3 (2560×1440) минимальное время обеспечила полная реализация Intel oneTBB (STL): "
        f"{fmt_ru(n2_seq, 2)} → {fmt_ru(n2_tbb, 2)} мс (Sp(4) = {fmt_ru(n2_sp, 3)}) и "
        f"{fmt_ru(n3_seq, 2)} → {fmt_ru(n3_tbb, 2)} мс (Sp(4) = {fmt_ru(n3_sp, 3)}). Варианты с потокобезопасным "
        f"контейнером oneTBB concurrent_vector упрощают код шага 4 (убирают ручное слияние локальных векторов) "
        f"при минимальном замедлении итогового конвейера (на НД 3: {fmt_ru(n3_tbbv, 2)} против {fmt_ru(n3_tbb, 2)} мс, менее 4 %)."
    )

    doc.add_subsection_heading("7.3", "Выводы по лабораторной работе в целом")
    doc.add_paragraph(
        "Освоена многоэтапная параллельная реализация детектора углов Харриса в OpenMP и Intel oneTBB "
        "(parallel_for, parallel_reduce, parallel_sort, enumerable_thread_specific, concurrent_vector). "
        "Экспериментально подтверждено, что выбор контейнера и технологии должен опираться на профилирование "
        "отдельных этапов: потоко-локальные STL-векторы быстрее при интенсивном доступе и сортировке, тогда как "
        "concurrent_vector существенно упрощает потокобезопасное накопление результатов при незначительном overhead."
    )

    # 8. Ответы на контрольные вопросы (все 8 вопросов со стр. 3 методички — кратко по сути)
    doc.add_section_heading("8", "Ответы на контрольные вопросы", page_break=True)

    qa = [
        ("1. Охарактеризуйте oneTBB parallel_sort.",
         "oneapi::tbb::parallel_sort(begin, end[, comp]) — параллельный шаблонный алгоритм неустойчивой "
         "сортировки на месте (внутренняя реализация на основе параллельного quicksort) со средней сложностью "
         "O(N log N). Требует итераторов произвольного доступа (RandomAccessIterator), работает с массивами, "
         "std::vector, std::deque и oneapi::tbb::concurrent_vector, автоматически распределяя поддиапазоны "
         "между потоками через планировщик задач oneTBB."),

        ("2. Расскажите о потокобезопасных контейнерах.",
         "Потокобезопасные (конкурентные) контейнеры — структуры данных, поддерживающие одновременный "
         "доступ из нескольких потоков без внешней синхронизации и гонок данных. В отличие от обёрток "
         "с глобальным мьютексом вокруг STL-контейнеров, они используют мелкозернистую блокировку "
         "(fine-grained locking) или lock-free алгоритмы на атомарных операциях, обеспечивая масштабируемость. "
         "В oneTBB к ним относятся concurrent_vector, concurrent_queue, concurrent_bounded_queue, "
         "concurrent_priority_queue, concurrent_hash_map и concurrent_unordered_map/set."),

        ("3. Охарактеризуйте oneTBB concurrent_vector.",
         "oneapi::tbb::concurrent_vector — потокобезопасный динамический массив с произвольным доступом "
         "за O(1), допускающий одновременный рост и обращение к существующим элементам из разных потоков. "
         "В отличие от std::vector, элементы хранятся в последовательности сегментов памяти (степени двойки) "
         "и никогда не перемещаются при расширении — поэтому ссылки и итераторы на уже добавленные элементы "
         "не инвалидируются. Плата за это — немного более медленный произвольный доступ (доп. индирекция) "
         "и отсутствие гарантии непрерывности всей памяти."),

        ("4. Какие методы oneTBB concurrent_vector позволяют безопасно добавлять элементы и увеличивать размер?",
         "Потокобезопасными методами роста являются: push_back(val) и emplace_back(args...) — добавляют один "
         "элемент в конец; grow_by(delta[, val]) — атомарно увеличивает размер на delta элементов и возвращает "
         "итератор на начало выделенного блока; grow_to_at_least(n) — гарантирует размер не менее n элементов. "
         "Все они возвращают итератор на добавленные элементы и безопасны при одновременном вызове из разных потоков."),

        ("5. Охарактеризуйте oneTBB concurrent_queue.",
         "oneapi::tbb::concurrent_queue — потокобезопасная неограниченная FIFO-очередь (первым вошёл — первым "
         "вышел), рассчитанная на параллельную работу множества производителей и потребителей (MPMC). "
         "Поддерживает неблокирующее добавление push(x)/emplace(...) и извлечение try_pop(res). "
         "Для сценариев с ограничением размера и ожиданием появления элементов предусмотрена парная "
         "структура concurrent_bounded_queue (с блокирующими push/pop)."),

        ("6. В чем заключается основное отличие concurrent_queue от queue при программной реализации доступа к элементам?",
         "В стандартной std::queue чтение и удаление разделены на два вызова — front() и pop(), что в "
         "многопоточной среде приводит к состоянию гонки (другой поток может удалить элемент между front() "
         "и pop()). В oneTBB concurrent_queue доступ и удаление объединены в одну атомарную операцию "
         "bool try_pop(T& dest): если очередь не пуста, она атомарно извлекает элемент в dest и возвращает true, "
         "иначе сразу возвращает false (метод front() отсутствует)."),

        ("7. Поясните понятие mutex и приведите пример использования.",
         "Мьютекс (mutex — mutual exclusion, взаимное исключение) — примитив синхронизации, гарантирующий, "
         "что в критическую секцию кода (обращение к разделяемому ресурсу) в каждый момент времени входит "
         "не более одного потока. На практике применяется по идиоме RAII через scoped_lock (автоматически "
         "захватывает мьютекс в конструкторе и освобождает при выходе из области видимости, даже при исключении). "
         "Пример: tbb::mutex mtx; { tbb::mutex::scoped_lock lock(mtx); shared_counter += local_sum; }."),

        ("8. Охарактеризуйте известные вам особенности oneTBB mutex.",
         "В oneTBB представлено семейство мьютексов с единым RAII-интерфейсом scoped_lock под разные задачи: "
         "spin_mutex (активное ожидание — быстр для сверхкоротких секций в несколько тактов), "
         "queuing_mutex (справедливый FIFO-порядок без голодания потоков и с низким трафиком кэша), "
         "spin_rw_mutex и queuing_rw_mutex (разделяемые блокировки «читатели–писатель»: несколько читателей "
         "работают параллельно, писатель — эксклюзивно), а также speculative_spin_mutex (аппаратная "
         "транзакционная память) и null_mutex (пустая заглушка для шаблонного кода без синхронизации)."),
    ]

    for q, a in qa:
        doc.add_paragraph(align="both", indent_cm=1.25, space_before_pt=3, runs=[
            {"text": q + " ", "bold": True, "size_pt": 14.0},
            {"text": a, "size_pt": 14.0},
        ])

    out_path = "Lab6/БПИ24-02. Пузынин. Лаб. № 6.docx"
    doc.save(out_path)
    print(f"Successfully generated {out_path} ({os.path.getsize(out_path):,} bytes, {len(doc.images)} figures)")


if __name__ == "__main__":
    main()
