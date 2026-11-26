import csv
import os
import struct
import zipfile
from xml.sax.saxutils import escape


def fmt_ru(val: float, decimals: int = 4) -> str:
    s = f"{val:,.{decimals}f}"
    return s.replace(",", "\u00A0").replace(".", ",")


def fmt_sig4(val: float) -> str:
    av = abs(val)
    if av >= 1000.0:
        return fmt_ru(val, 2)
    elif av >= 100.0:
        return fmt_ru(val, 3)
    else:
        return fmt_ru(val, 4)


def png_dimensions(path: str):
    with open(path, "rb") as f:
        header = f.read(24)
        if header[:8] != b"\x89PNG\r\n\x1a\n":
            raise ValueError(f"Not a PNG file: {path}")
        return struct.unpack(">II", header[16:24])


def estimate_f_amdahl(pairs):
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


class GostDocxBuilder:
    """Генератор OOXML .docx строго по ГОСТ (A4, поля 3/2/2/2 см, TNR 14 пт, интервал 1.0, отступ 1.25 см)."""

    def __init__(self):
        self.body_parts = []
        self.images = []
        self.next_rid = 10
        self.next_docpr = 1

    def _run_xml(self, text: str, bold=False, italic=False, size_pt=14.0, font="Times New Roman", color="000000", underline=False):
        sz = int(round(size_pt * 2))
        rpr = [
            f'<w:rFonts w:ascii="{font}" w:hAnsi="{font}" w:cs="{font}" w:eastAsia="{font}"/>',
            f'<w:color w:val="{color}"/>',
            f'<w:sz w:val="{sz}"/>',
            f'<w:szCs w:val="{sz}"/>',
            '<w:lang w:val="ru-RU"/>',
        ]
        if bold:
            rpr.append("<w:b/><w:bCs/>")
        if italic:
            rpr.append("<w:i/><w:iCs/>")
        if underline:
            rpr.append('<w:u w:val="single"/>')
        return f'<w:r><w:rPr>{"".join(rpr)}</w:rPr><w:t xml:space="preserve">{escape(text)}</w:t></w:r>'

    def add_paragraph(self, text: str = "", align="both", indent_cm=1.25, bold=False, italic=False,
                      size_pt=14.0, font="Times New Roman", space_before_pt=0, space_after_pt=0,
                      line_mult=1.0, keep_next=False, page_break_before=False, runs=None):
        ppr = []
        if keep_next:
            ppr.append("<w:keepNext/>")
        if page_break_before:
            ppr.append("<w:pageBreakBefore/>")
        ppr.append("<w:widowControl/>")
        sb = int(round(space_before_pt * 20))
        sa = int(round(space_after_pt * 20))
        line_val = int(round(line_mult * 240))
        ppr.append(f'<w:spacing w:before="{sb}" w:after="{sa}" w:line="{line_val}" w:lineRule="auto"/>')
        if indent_cm > 0:
            ind = int(round(indent_cm * 567))
            ppr.append(f'<w:ind w:firstLine="{ind}"/>')
        ppr.append(f'<w:jc w:val="{align}"/>')

        if runs is not None:
            content = "".join(self._run_xml(**r) for r in runs)
        else:
            content = self._run_xml(text, bold=bold, italic=italic, size_pt=size_pt, font=font) if text else ""
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
            self.add_paragraph(
                align="both", indent_cm=1.25,
                runs=[
                    {"text": "– ", "size_pt": 14.0},
                    {"text": bold_prefix, "bold": True, "size_pt": 14.0},
                    {"text": text, "size_pt": 14.0},
                ]
            )
        else:
            self.add_paragraph(f"– {text}", align="both", indent_cm=1.25)

    def add_table(self, caption: str, headers: list, rows: list, col_widths_twips: list,
                  font_pt: float = 10.0, aligns: list = None):
        if caption:
            self.add_paragraph(
                caption, align="left", indent_cm=0, size_pt=14.0,
                space_before_pt=6, space_after_pt=3, keep_next=True
            )
        total_w = sum(col_widths_twips)
        ncols = len(col_widths_twips)
        if aligns is None:
            aligns = ["left"] + ["right"] * (ncols - 1)

        parts = [
            '<w:tbl><w:tblPr>',
            f'<w:tblW w:w="{total_w}" w:type="dxa"/>',
            '<w:tblLayout w:type="fixed"/><w:jc w:val="center"/>',
            '<w:tblBorders>'
            '<w:top w:val="single" w:sz="6" w:space="0" w:color="000000"/>'
            '<w:left w:val="single" w:sz="6" w:space="0" w:color="000000"/>'
            '<w:bottom w:val="single" w:sz="6" w:space="0" w:color="000000"/>'
            '<w:right w:val="single" w:sz="6" w:space="0" w:color="000000"/>'
            '<w:insideH w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
            '<w:insideV w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
            '</w:tblBorders>',
            '<w:tblCellMar><w:top w:w="35" w:type="dxa"/><w:bottom w:w="35" w:type="dxa"/>'
            '<w:left w:w="60" w:type="dxa"/><w:right w:w="60" w:type="dxa"/></w:tblCellMar>',
            '</w:tblPr><w:tblGrid>' + "".join(f'<w:gridCol w:w="{w}"/>' for w in col_widths_twips) + '</w:tblGrid>',
            '<w:tr><w:trPr><w:tblHeader/><w:cantSplit/></w:trPr>',
        ]
        for ci, h in enumerate(headers):
            parts.append(
                f'<w:tc><w:tcPr><w:tcW w:w="{col_widths_twips[ci]}" w:type="dxa"/>'
                '<w:shd w:val="clear" w:color="auto" w:fill="E8ECF2"/><w:vAlign w:val="center"/></w:tcPr>'
                '<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="220" w:lineRule="auto"/><w:jc w:val="center"/></w:pPr>'
                + self._run_xml(h, bold=True, size_pt=font_pt) + '</w:p></w:tc>'
            )
        parts.append('</w:tr>')

        for row in rows:
            parts.append('<w:tr><w:trPr><w:cantSplit/></w:trPr>')
            for ci, val in enumerate(row):
                al = aligns[ci] if ci < len(aligns) else "right"
                parts.append(
                    f'<w:tc><w:tcPr><w:tcW w:w="{col_widths_twips[ci]}" w:type="dxa"/><w:vAlign w:val="center"/></w:tcPr>'
                    f'<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="220" w:lineRule="auto"/><w:jc w:val="{al}"/></w:pPr>'
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

        rid = f"rId{self.next_rid}"
        self.next_rid += 1
        media_name = f"image{len(self.images) + 1}.png"
        self.images.append((rid, media_name, os.path.abspath(img_path)))

        docpr_id = self.next_docpr
        self.next_docpr += 1

        drawing = (
            '<w:p><w:pPr><w:keepNext/>'
            '<w:spacing w:before="80" w:after="40" w:line="240" w:lineRule="auto"/>'
            '<w:jc w:val="center"/></w:pPr>'
            '<w:r><w:drawing>'
            '<wp:inline distT="0" distB="0" distL="0" distR="0" '
            'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing">'
            f'<wp:extent cx="{cx}" cy="{cy}"/>'
            '<wp:effectExtent l="0" t="0" r="0" b="0"/>'
            f'<wp:docPr id="{docpr_id}" name="Picture {docpr_id}"/>'
            '<wp:cNvGraphicFramePr><a:graphicFrameLocks xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" noChangeAspect="1"/></wp:cNvGraphicFramePr>'
            '<a:graphic xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main">'
            '<a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/picture">'
            '<pic:pic xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture">'
            '<pic:nvPicPr>'
            f'<pic:cNvPr id="{docpr_id}" name="{media_name}"/>'
            '<pic:cNvPicPr/>'
            '</pic:nvPicPr>'
            '<pic:blipFill>'
            f'<a:blip r:embed="{rid}" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"/>'
            '<a:stretch><a:fillRect/></a:stretch>'
            '</pic:blipFill>'
            '<pic:spPr>'
            f'<a:xfrm><a:off x="0" y="0"/><a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
            '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom>'
            '</pic:spPr>'
            '</pic:pic></a:graphicData></a:graphic>'
            '</wp:inline></w:drawing></w:r></w:p>'
        )
        self.body_parts.append(drawing)
        self.add_paragraph(
            f"Рисунок {fig_num} – {caption}", align="center", indent_cm=0,
            size_pt=12.0, space_before_pt=0, space_after_pt=6
        )

    def add_title_page(self):
        """Титульный лист в точном стиле отчёта ЛР №2 и шаблона СибГУ им. М.Ф. Решетнева."""
        self.add_paragraph("МИНИСТЕРСТВО НАУКИ И ВЫСШЕГО ОБРАЗОВАНИЯ РОССИЙСКОЙ ФЕДЕРАЦИИ", align="center", indent_cm=0, size_pt=11.0)
        self.add_paragraph("Федеральное государственное бюджетное образовательное учреждение высшего образования", align="center", indent_cm=0, size_pt=11.0)
        self.add_paragraph("«Сибирский государственный университет науки и технологий имени академика М.Ф. Решетнева»", align="center", indent_cm=0, bold=True, size_pt=11.5, space_after_pt=4)
        self.add_paragraph("Институт информатики и телекоммуникаций", align="center", indent_cm=0, size_pt=12.0)
        self.add_paragraph("Кафедра информатики и вычислительной техники", align="center", indent_cm=0, size_pt=12.0, space_after_pt=56)

        self.add_paragraph("ОТЧЕТ ПО ЛАБОРАТОРНОЙ РАБОТЕ", align="center", indent_cm=0, bold=True, size_pt=16.0, space_after_pt=8)
        self.add_paragraph("Параллельные вычисления", align="center", indent_cm=0, size_pt=14.0, space_after_pt=18)

        # Рамка с номером и названием лабораторной работы (как в шаблоне и ЛР №2)
        box = (
            '<w:tbl><w:tblPr><w:tblW w:w="9071" w:type="dxa"/><w:jc w:val="center"/>'
            '<w:tblBorders>'
            '<w:top w:val="single" w:sz="6" w:space="0" w:color="000000"/>'
            '<w:left w:val="single" w:sz="6" w:space="0" w:color="000000"/>'
            '<w:bottom w:val="single" w:sz="6" w:space="0" w:color="000000"/>'
            '<w:right w:val="single" w:sz="6" w:space="0" w:color="000000"/>'
            '<w:insideH w:val="single" w:sz="4" w:space="0" w:color="000000"/>'
            '<w:insideV w:val="none"/>'
            '</w:tblBorders>'
            '<w:tblCellMar><w:top w:w="90" w:type="dxa"/><w:bottom w:w="90" w:type="dxa"/>'
            '<w:left w:w="120" w:type="dxa"/><w:right w:w="120" w:type="dxa"/></w:tblCellMar>'
            '</w:tblPr><w:tblGrid><w:gridCol w:w="9071"/></w:tblGrid>'
            '<w:tr><w:tc><w:tcPr><w:tcW w:w="9071" w:type="dxa"/></w:tcPr>'
            '<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/><w:jc w:val="center"/></w:pPr>'
            + self._run_xml("Лабораторная работа № 3", bold=True, size_pt=14.0) + '</w:p></w:tc></w:tr>'
            '<w:tr><w:tc><w:tcPr><w:tcW w:w="9071" w:type="dxa"/></w:tcPr>'
            '<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/><w:jc w:val="center"/></w:pPr>'
            + self._run_xml("Параллельные алгоритмы сортировки в OpenMP и их практическое применение", bold=False, size_pt=14.0)
            + '</w:p></w:tc></w:tr></w:tbl>'
        )
        self.body_parts.append(box)
        self.add_paragraph("", indent_cm=0, space_after_pt=76)

        # Таблица подписей в точном формате шаблона (Руководитель / Обучающийся)
        def sig_cell(w, text, align="center", size=14.0, bottom_border=False, italic=False):
            bdr = '<w:tcBorders><w:bottom w:val="single" w:sz="4" w:space="0" w:color="000000"/></w:tcBorders>' if bottom_border else ''
            return (
                f'<w:tc><w:tcPr><w:tcW w:w="{w}" w:type="dxa"/>{bdr}<w:vAlign w:val="bottom"/></w:tcPr>'
                f'<w:p><w:pPr><w:spacing w:before="0" w:after="0" w:line="220" w:lineRule="auto"/><w:jc w:val="{align}"/></w:pPr>'
                + (self._run_xml(text, italic=italic, size_pt=size) if text else "") + '</w:p></w:tc>'
            )

        ws = [2100, 2600, 200, 1971, 200, 2000]  # sum = 9071
        sig = [
            '<w:tbl><w:tblPr><w:tblW w:w="9071" w:type="dxa"/><w:tblLayout w:type="fixed"/><w:jc w:val="center"/>'
            '<w:tblBorders><w:top w:val="none"/><w:left w:val="none"/><w:bottom w:val="none"/>'
            '<w:right w:val="none"/><w:insideH w:val="none"/><w:insideV w:val="none"/></w:tblBorders>'
            '<w:tblCellMar><w:top w:w="20" w:type="dxa"/><w:bottom w:w="20" w:type="dxa"/><w:left w:w="20" w:type="dxa"/><w:right w:w="20" w:type="dxa"/></w:tblCellMar>'
            '</w:tblPr><w:tblGrid>' + "".join(f'<w:gridCol w:w="{w}"/>' for w in ws) + '</w:tblGrid>',
            # Строка 1: Руководитель
            '<w:tr>'
            + sig_cell(ws[0], "Руководитель", "left", 14.0)
            + sig_cell(ws[1], "", "center", 14.0)
            + sig_cell(ws[2], "", "center", 14.0)
            + sig_cell(ws[3], "", "center", 14.0, bottom_border=True)
            + sig_cell(ws[4], "", "center", 14.0)
            + sig_cell(ws[5], "А.Г. Зотин", "center", 14.0, bottom_border=True)
            + '</w:tr>',
            # Подстрочные пояснения 1
            '<w:tr>'
            + sig_cell(ws[0], "", "left", 9.0)
            + sig_cell(ws[1], "", "center", 9.0)
            + sig_cell(ws[2], "", "center", 9.0)
            + sig_cell(ws[3], "подпись, дата", "center", 9.5, italic=True)
            + sig_cell(ws[4], "", "center", 9.0)
            + sig_cell(ws[5], "инициалы, фамилия", "center", 9.5, italic=True)
            + '</w:tr>',
            # Отступ между строками
            '<w:tr>' + "".join(sig_cell(w, " ", "center", 10.0) for w in ws) + '</w:tr>',
            # Строка 2: Обучающийся
            '<w:tr>'
            + sig_cell(ws[0], "Обучающийся", "left", 14.0)
            + sig_cell(ws[1], "БПИ24-02, 24151438", "center", 13.0, bottom_border=True)
            + sig_cell(ws[2], "", "center", 14.0)
            + sig_cell(ws[3], "", "center", 14.0, bottom_border=True)
            + sig_cell(ws[4], "", "center", 14.0)
            + sig_cell(ws[5], "Д.А. Пузынин", "center", 14.0, bottom_border=True)
            + '</w:tr>',
            # Подстрочные пояснения 2
            '<w:tr>'
            + sig_cell(ws[0], "", "left", 9.0)
            + sig_cell(ws[1], "номер группы, зачетной книжки", "center", 9.5, italic=True)
            + sig_cell(ws[2], "", "center", 9.0)
            + sig_cell(ws[3], "подпись, дата", "center", 9.5, italic=True)
            + sig_cell(ws[4], "", "center", 9.0)
            + sig_cell(ws[5], "инициалы, фамилия", "center", 9.5, italic=True)
            + '</w:tr>',
            '</w:tbl>'
        ]
        self.body_parts.append("".join(sig))
        self.add_paragraph("", indent_cm=0, space_before_pt=135)
        self.add_paragraph("Красноярск 2026 г.", align="center", indent_cm=0, size_pt=14.0)

    def save(self, out_path: str):
        sect_pr = (
            '<w:sectPr>'
            '<w:footerReference w:type="default" r:id="rId2"/>'
            '<w:footerReference w:type="first" r:id="rId3"/>'
            '<w:pgSz w:w="11906" w:h="16838"/>'
            '<w:pgMar w:top="1134" w:right="1134" w:bottom="1134" w:left="1701" w:header="708" w:footer="567" w:gutter="0"/>'
            '<w:titlePg/>'
            '</w:sectPr>'
        )

        document_xml = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" '
            'xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships" '
            'xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing">\n'
            '<w:body>\n' + "\n".join(self.body_parts) + "\n" + sect_pr + "\n</w:body>\n</w:document>"
        )

        styles_xml = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">\n'
            '  <w:docDefaults>\n'
            '    <w:rPrDefault><w:rPr><w:rFonts w:ascii="Times New Roman" w:hAnsi="Times New Roman" w:cs="Times New Roman"/>'
            '<w:sz w:val="28"/><w:szCs w:val="28"/><w:lang w:val="ru-RU"/></w:rPr></w:rPrDefault>\n'
            '    <w:pPrDefault><w:pPr><w:spacing w:before="0" w:after="0" w:line="240" w:lineRule="auto"/></w:pPr></w:pPrDefault>\n'
            '  </w:docDefaults>\n'
            '  <w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>\n'
            '</w:styles>'
        )

        footer_default_xml = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<w:ftr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">\n'
            '  <w:p><w:pPr><w:jc w:val="center"/></w:pPr>\n'
            '    <w:fldSimple w:instr=" PAGE "><w:r><w:rPr><w:rFonts w:ascii="Times New Roman" w:hAnsi="Times New Roman"/>'
            '<w:sz w:val="24"/><w:szCs w:val="24"/></w:rPr><w:t>2</w:t></w:r></w:fldSimple>\n'
            '  </w:p>\n'
            '</w:ftr>'
        )

        footer_first_xml = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<w:ftr xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"><w:p/></w:ftr>'
        )

        settings_xml = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<w:settings xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">\n'
            '  <w:compat><w:compatSetting w:name="compatibilityMode" w:uri="http://schemas.microsoft.com/office/word" w:val="15"/></w:compat>\n'
            '</w:settings>'
        )

        rels = [
            '<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>',
            '<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer" Target="footer1.xml"/>',
            '<Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer" Target="footer2.xml"/>',
            '<Relationship Id="rId4" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" Target="settings.xml"/>',
        ]
        for rid, media_name, _ in self.images:
            rels.append(
                f'<Relationship Id="{rid}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" Target="media/{media_name}"/>'
            )

        doc_rels_xml = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">\n'
            + "\n".join("  " + x for x in rels) + "\n</Relationships>"
        )

        content_types_xml = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">\n'
            '  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>\n'
            '  <Default Extension="xml" ContentType="application/xml"/>\n'
            '  <Default Extension="png" ContentType="image/png"/>\n'
            '  <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>\n'
            '  <Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>\n'
            '  <Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>\n'
            '  <Override PartName="/word/footer1.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"/>\n'
            '  <Override PartName="/word/footer2.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml"/>\n'
            '</Types>'
        )

        root_rels_xml = (
            '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n'
            '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">\n'
            '  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>\n'
            '</Relationships>'
        )

        with zipfile.ZipFile(out_path, "w", zipfile.ZIP_DEFLATED) as zf:
            zf.writestr("[Content_Types].xml", content_types_xml)
            zf.writestr("_rels/.rels", root_rels_xml)
            zf.writestr("word/document.xml", document_xml)
            zf.writestr("word/styles.xml", styles_xml)
            zf.writestr("word/settings.xml", settings_xml)
            zf.writestr("word/footer1.xml", footer_default_xml)
            zf.writestr("word/footer2.xml", footer_first_xml)
            zf.writestr("word/_rels/document.xml.rels", doc_rels_xml)
            for _, media_name, abs_path in self.images:
                zf.write(abs_path, f"word/media/{media_name}")


def load_csv(path: str):
    with open(path, "r", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def main():
    t31 = load_csv("results/task3_1_summary.csv")
    t32 = load_csv("results/task3_2_summary.csv")
    idx31 = {(r["dtype"], int(r["n"]), r["algo"], int(r["threads"])): r for r in t31}
    idx32 = {(int(r["img_id"]), r["algo"], int(r["ksize"]), int(r["threads"])): r for r in t32}

    doc = GostDocxBuilder()

    # =========================================================================
    # ТИТУЛЬНЫЙ ЛИСТ
    # =========================================================================
    doc.add_title_page()

    # =========================================================================
    # 1. ЦЕЛЬ РАБОТЫ, 2. ЗАДАНИЯ, 3. ФОРМУЛЫ И МЕТОДИКА, 4. КОНФИГУРАЦИЯ
    # =========================================================================
    doc.add_section_heading("1", "Цель работы", page_break=True)
    doc.add_paragraph(
        "Изучение различных алгоритмов сортировки и их параллельных вариантов реализации. "
        "Знакомство с медианным фильтром для обработки изображений."
    )

    doc.add_section_heading("2", "Задания")
    doc.add_paragraph(
        align="both", indent_cm=1.25,
        runs=[
            {"text": "Задание 3.1. ", "bold": True, "size_pt": 14.0},
            {"text": (
                "Разработать консольное приложение, реализующее сортировку одномерного массива разными "
                "алгоритмами (в последовательной и параллельной реализации): классический алгоритм пузырька "
                "(только последовательно), пузырька (алгоритм чёт-нечётной перестановки), Шелла и быстрой "
                "сортировки (qsort). Выполнить экспериментальное исследование на трёх наборах (20 000–50 000 эл.) "
                "для типов данных int и double (не менее 30 запусков для каждой реализации) на одинаковом исходном "
                "массиве, заполненном по закономерности от индекса."
            ), "size_pt": 14.0}
        ]
    )
    doc.add_paragraph(
        align="both", indent_cm=1.25,
        runs=[
            {"text": "Задание 3.2. ", "bold": True, "size_pt": 14.0},
            {"text": (
                "Разработать консольное приложение, реализующее медианную фильтрацию четырёх цветных "
                "BMP-изображений (input_1.bmp – input_4.bmp) с сохранением результатов output_X_alg.bmp "
                "и использованием не менее двух алгоритмов сортировки из задания 3.1 (сортировка Шелла и qsort). "
                "Провести экспериментальное исследование (не менее 15 запусков для каждой реализации) для размеров "
                "окрестности Ksize×Ksize (Ksize = 7, 11, 15) и четырёх изображений разного разрешения."
            ), "size_pt": 14.0}
        ]
    )

    doc.add_section_heading("3", "Формулы заполнения массива и методика измерений")
    doc.add_paragraph(
        "Для детерминированного заполнения исходного массива в задании 3.1 (НД1 = 20 000, НД2 = 35 000, "
        "НД3 = 50 000 эл.) использованы аналитические функции от индекса i (перед каждым запуском массив "
        "копируется в рабочий буфер вне замера времени):"
    )
    doc.add_paragraph(
        "a_int[i] = floor(10000·sin(0,37·i + 0,5) + 5000·cos(1,13·i)) + ((7919·i + 104729) mod 10000) − 5000",
        align="center", indent_cm=0, italic=True, size_pt=12.5, space_before_pt=3, space_after_pt=2
    )
    doc.add_paragraph(
        "a_double[i] = 1000·sin(0,37·i + 0,5) + 500·cos(1,13·i) + 0,01·((7919·i + 104729) mod 10000)",
        align="center", indent_cm=0, italic=True, size_pt=12.5, space_before_pt=0, space_after_pt=4
    )
    doc.add_paragraph(
        "Замеры времени выполнены функцией omp_get_wtime() (для быстрых сортировок Шелла и qsort "
        "с пакетной компенсацией тика omp_get_wtick() = 1 мс и 110 замерами; для пузырька — 30 замеров, "
        "чёт-нечётной — 35 замеров, медианного фильтра — 15 замеров и 110 замеров для контрольной серии). "
        "Фильтрация выбросов выполнена по медиане функцией AvgTrustedIntervalMed (интервал med ± SD). "
        "Показатели эффективности: Sp(n) = T1/Tp, Ep(n) = Sp/p, Cp(n) = p·Tp; теоретические пределы — "
        "закон Амдала Sp ≤ 1/(f + (1−f)/p) и закон Густавсона–Барсиса Sp ≤ f + (1−f)·p."
    )

    doc.add_section_heading("4", "Конфигурация компьютера и параметры ОС")
    doc.add_bullet("AMD Ryzen 5 5600 (6 физических ядер / 12 логических потоков), тактовая частота до 4,65 ГГц.", "Процессор: ")
    doc.add_bullet("32 ГБ (2 модуля), частота 3000 МГц.", "Оперативная память: ")
    doc.add_bullet("Windows 11 26H2 (сборка 26300, 64-разрядная).", "Операционная система: ")
    doc.add_bullet("G++ 15.2.0 (MSYS2 MinGW UCRT64), поддержка OpenMP 4.5 (_OPENMP = 201511).", "Компилятор: ")
    doc.add_bullet("Release (-std=c++17 -fopenmp -O3 -march=native -Wall -Wextra, без предупреждений).", "Сборка: ")

    # =========================================================================
    # 5. ЭКРАННЫЕ СНИМКИ РАБОТЫ ПРОГРАММ (ПОЛНЫЙ ВЫВОД КОНСОЛИ)
    # =========================================================================
    doc.add_section_heading("5", "Экранные снимки работы программ (полный вывод консоли)", page_break=True)

    doc.add_subsection_heading("5.1", "Задание 3.1 — сортировка одномерного массива (int и double)")
    doc.add_paragraph(
        "Ниже приведён полный, без сокращений, консольный вывод приложения task3_1.exe по всем шести "
        "экспериментальным сериям (типы int и double, наборы 20 000, 35 000 и 50 000 элементов, "
        "потоки 1–32, статистика доверительного интервала, Sp, Ep и Cp)."
    )

    fig_num = 1
    c31_specs = [
        ("results/console_3_1_p1.png", "Полный консольный вывод задания 3.1 (ч. 1/6): параметры стенда и int, НД1 (n = 20 000)", 11.4),
        ("results/console_3_1_p2.png", "Полный консольный вывод задания 3.1 (ч. 2/6): int, НД2 (n = 35 000)", 10.2),
        ("results/console_3_1_p3.png", "Полный консольный вывод задания 3.1 (ч. 3/6): int, НД3 (n = 50 000)", 10.2),
        ("results/console_3_1_p4.png", "Полный консольный вывод задания 3.1 (ч. 4/6): double, НД1 (n = 20 000)", 10.8),
        ("results/console_3_1_p5.png", "Полный консольный вывод задания 3.1 (ч. 5/6): double, НД2 (n = 35 000)", 10.2),
        ("results/console_3_1_p6.png", "Полный консольный вывод задания 3.1 (ч. 6/6): double, НД3 (n = 50 000)", 10.4),
    ]
    for path, cap, max_h in c31_specs:
        doc.add_figure(path, fig_num, cap, width_cm=15.8, max_height_cm=max_h)
        fig_num += 1

    doc.add_subsection_heading("5.2", "Задание 3.2 — медианная фильтрация BMP-изображений")
    doc.add_paragraph(
        "В соответствии со сноской 3 методических указаний для проведения полных 15-кратных серий "
        "по всем комбинациям алгоритмов (Шелла и 3-сторонний qsort), окон (Ksize = 7, 11, 15) и числа "
        "потоков использованы 4 цветных 24-битных BMP-изображения с 8% импульсным шумом в пропорциональной "
        "шкале разрешений 16:9: НД 1 (480×270), НД 2 (640×360), НД 3 (960×540) и НД 4 (1280×720). "
        "Ниже представлен полный консольный вывод программы task3_2.exe."
    )

    c32_specs = [
        ("results/console_3_2_p1.png", "Полный консольный вывод задания 3.2 (ч. 1/5): загрузка input_1..4.bmp и НД 1 (480×270)", 11.2),
        ("results/console_3_2_p2.png", "Полный консольный вывод задания 3.2 (ч. 2/5): НД 2 (640×360)", 9.8),
        ("results/console_3_2_p3.png", "Полный консольный вывод задания 3.2 (ч. 3/5): НД 3 (960×540)", 9.8),
        ("results/console_3_2_p4.png", "Полный консольный вывод задания 3.2 (ч. 4/5): НД 4 (1280×720, Ksize = 7 и 11, потоки 1–16)", 11.0),
        ("results/console_3_2_p5.png", "Полный консольный вывод задания 3.2 (ч. 5/5): НД 4 (1280×720, Ksize = 15, потоки 1–16)", 7.5),
    ]
    for path, cap, max_h in c32_specs:
        doc.add_figure(path, fig_num, cap, width_cm=15.8, max_height_cm=max_h)
        fig_num += 1

    doc.add_paragraph(
        "Примеры исходных зашумлённых и обработанных медианным фильтром изображений (в том числе "
        "увеличенный в 4 раза фрагмент для оценки сохранения контуров) показаны на рисунках "
        f"{fig_num} и {fig_num + 1}."
    )
    doc.add_figure("results/visual_median_compare.png", fig_num,
                   "Примеры исходных (зашумлённых) и отфильтрованных изображений (НД 2, НД 3, НД 4)", width_cm=15.8, max_height_cm=9.5)
    fig_num += 1
    doc.add_figure("results/visual_median_zoom.png", fig_num,
                   "Увеличенный фрагмент (4×) НД 4 (1280×720): подавление импульсного шума и влияние Ksize", width_cm=15.8, max_height_cm=6.5)
    fig_num += 1

    # =========================================================================
    # 6. РЕЗУЛЬТАТЫ ЭКСПЕРИМЕНТАЛЬНОГО ИССЛЕДОВАНИЯ И АНАЛИЗ
    # =========================================================================
    doc.add_section_heading("6", "Результаты экспериментального исследования", page_break=True)
    doc.add_paragraph(
        "Полные сводные таблицы 3.1 (сортировка для int и double, 20 000 / 35 000 / 50 000 эл., "
        "время, Sp, Ep, Cp), 3.2 (медианная фильтрация НД 1 – НД 4, Ksize = 7, 11, 15), расширенные "
        "таблицы масштабируемости до 16–32 потоков, 110 сырых замеров доверительного интервала с живыми "
        "формулами и расчёты по законам Амдала и Густавсона–Барсиса представлены в приложенной книге Excel "
        "«Результаты_исследования_Лаб3_OpenMP.xlsx». В таблице 1 приведена сводная оценка доли "
        "последовательных вычислений f и сравнение с теоретическими пределами."
    )

    shell_pts = [(p, float(idx31[("int", 50000, "Shell_Par" if p > 1 else "Shell_Seq", p)]["speedup"])) for p in [1,2,3,4,6,8,12,16]]
    qsort_pts = [(p, float(idx31[("int", 50000, "QuickSort_Par" if p > 1 else "QuickSort_Seq", p)]["speedup"])) for p in [1,2,3,4,6,8,12,16]]
    mshell_pts = [(p, float(idx32[(4, "Median_Shell", 15, p)]["speedup"])) for p in [1,2,3,4,6,8,12,16]]
    mqsort_pts = [(p, float(idx32[(4, "Median_QuickSort", 15, p)]["speedup"])) for p in [1,2,3,4,6,8,12,16]]

    f_shell  = estimate_f_amdahl([x for x in shell_pts  if 2 <= x[0] <= 6])
    f_qsort  = estimate_f_amdahl([x for x in qsort_pts  if 2 <= x[0] <= 6])
    f_mshell = estimate_f_amdahl([x for x in mshell_pts if 2 <= x[0] <= 12])
    f_mqsort = estimate_f_amdahl([x for x in mqsort_pts if 2 <= x[0] <= 12])

    def amd(f, p): return 1.0 / (f + (1.0 - f) / p)
    def gus(f, p): return f + (1.0 - f) * p

    t_rows = []
    for lbl, f_e, pts in [
        ("Зад. 3.1: Шелла (int, 50 000)", f_shell, shell_pts),
        ("Зад. 3.1: QuickSort (int, 50 000)", f_qsort, qsort_pts),
        ("Зад. 3.2: Медиан. Шелла (1280×720, K=15)", f_mshell, mshell_pts),
        ("Зад. 3.2: Медиан. qsort (1280×720, K=15)", f_mqsort, mqsort_pts),
    ]:
        sp_m = dict(pts)
        t_rows.append([
            lbl, fmt_ru(f_e, 4), fmt_ru(1.0 / f_e if f_e > 1e-5 else 999.0, 2),
            fmt_ru(sp_m[4], 3), fmt_ru(amd(f_e, 4), 3), fmt_ru(gus(f_e, 4), 3),
            fmt_ru(sp_m[6], 3), fmt_ru(sp_m[12], 3),
        ])
    doc.add_table(
        "Таблица 1 – Оценка доли последовательных вычислений f и сравнение с законами Амдала и Густавсона–Барсиса",
        ["Алгоритм", "f", "S* = 1/f", "Sp(4) эксп.", "Амдал (4)", "Густ. (4)", "Sp(6) эксп.", "Sp(12) эксп."],
        t_rows, [2951, 800, 820, 920, 900, 900, 890, 890], font_pt=9.0,
        aligns=["left", "right", "right", "right", "right", "right", "right", "right"]
    )

    # Компактные графики (по 2 на страницу, полностью читаемые)
    charts = [
        ("results/chart_ci_task31.png", "Доверительный интервал (med ± SD): Задание 3.1, QuickSort (int, n = 50 000, p = 4, N = 110)", 9.0),
        ("results/chart_ci_task32.png", "Доверительный интервал (med ± SD): Задание 3.2, медианный фильтр Шелла (480×270, K = 7, p = 4, N = 110)", 9.0),
        ("results/chart_speedup_amdahl_task31.png", "Ускорение Sp(n) алгоритмов сортировки (n = 50 000) в сравнении с кривыми закона Амдала", 9.4),
        ("results/chart_speedup_task32.png", "Ускорение Sp(n) медианного фильтра (1280×720) и сравнение с законами Амдала / Густавсона–Барсиса", 9.4),
        ("results/chart_eff_cost.png", "Эффективность Ep(p) и проверка стоимостной оптимальности Cp / T1", 8.2),
        ("results/chart_time_ksize_task32.png", "Время медианной фильтрации (p = 4) для разных размеров окна Ksize и разрешений BMP", 9.0),
    ]
    for path, cap, max_h in charts:
        doc.add_figure(path, fig_num, cap, width_cm=15.6, max_height_cm=max_h)
        fig_num += 1

    oe4_sp = float(idx31[("int", 50000, "OddEven_Par", 4)]["speedup"])
    sh4_sp = float(idx31[("int", 50000, "Shell_Par", 4)]["speedup"])
    qs4_sp = float(idx31[("int", 50000, "QuickSort_Par", 4)]["speedup"])
    ms6_sp = float(idx32[(4, "Median_Shell", 15, 6)]["speedup"])
    ms12_sp = float(idx32[(4, "Median_Shell", 15, 12)]["speedup"])
    mq4_ms = float(idx32[(4, "Median_QuickSort", 15, 4)]["trusted_avg_med_ms"])
    ms4_ms = float(idx32[(4, "Median_Shell", 15, 4)]["trusted_avg_med_ms"])

    doc.add_subsection_heading("6.1", "Наблюдения")
    doc.add_paragraph(
        f"В задании 3.1 алгоритмы сортировки Шелла и qsort на массиве из 50 000 элементов превосходят "
        f"классическую сортировку пузырьком по быстродействию более чем в 1000 раз. Сортировка Шелла "
        f"имеет заметную последовательную долю f ≈ {fmt_ru(f_shell, 4)} (S4 = {fmt_ru(sh4_sp, 3)}), так как "
        f"на финальных шагах h = 4 и h = 1 число независимых подмассивов меньше числа потоков. Быстрая "
        f"сортировка qsort на задачах OpenMP (#pragma omp task) масштабируется лучше (f ≈ {fmt_ru(f_qsort, 4)}, "
        f"S4 = {fmt_ru(qs4_sp, 3)}, пик до 3,67–3,87 на 8–16 потоках), однако на p = 2 для n = 50 000 "
        f"наблюдается просадка из-за дисбаланса первого разбиения и накладных расходов планировщика задач."
    )
    doc.add_paragraph(
        f"Параллельная блочная чёт-нечётная перестановка (схема Гергеля) показала алгоритмическое "
        f"сверхлинейное ускорение S4 = {fmt_ru(oe4_sp, 2)} (Cp < T1), поскольку разбиение задачи O(n²) "
        f"на p блоков снижает суммарную трудоёмкость локальной сортировки до p·O((n/p)²) = O(n²/p), "
        f"а последующие p фаз Compare-Split имеют линейную сложность O(p·n)."
    )
    doc.add_paragraph(
        f"В задании 3.2 медианная фильтрация показала почти идеальное масштабирование и стоимостную "
        f"оптимальность (Cp ≈ T1) на всех 6 физических ядрах Ryzen 5 5600: при 1280×720 и Ksize = 15 "
        f"оценка f ≈ {fmt_ru(f_mshell, 4)}, ускорение достигает S6 = {fmt_ru(ms6_sp, 3)} (Ep ≈ 0,95) и "
        f"продолжает расти на SMT-потоках до S12 = {fmt_ru(ms12_sp, 3)}, выходя на плато при p = 16. "
        f"Использование планарного формата каналов (SoA), приватных буферов и schedule(dynamic, 8) "
        f"устранило дисбаланс на неоднородных сценах. Вариант с 3-сторонним qsort быстрее сортировки Шелла "
        f"на всех наборах ({fmt_sig4(mq4_ms)} мс против {fmt_sig4(ms4_ms)} мс на НД 4, K = 15, p = 4) за счёт "
        f"эффективной группировки повторяющихся значений яркости в окне. Импульсный шум 8% полностью "
        f"устраняется уже при Ksize = 7."
    )

    # =========================================================================
    # 7. ВЫВОДЫ
    # =========================================================================
    doc.add_section_heading("7", "Выводы")
    doc.add_subsection_heading("7.1", "Выводы по результатам исследования")
    doc.add_paragraph(
        "Экспериментально подтверждено, что эффективность распараллеливания напрямую зависит от "
        "структуры алгоритма и доли последовательных вычислений f. Наибольшую масштабируемость и "
        "стоимостную оптимальность (Ep ≈ 0,94–0,99 на физических ядрах, Cp ≈ T1) обеспечила задача "
        "медианной фильтрации изображений благодаря высокому параллелизму по данным. В алгоритмах "
        "сортировки одномерных массивов параллельные Шелла и qsort ограничены законом Амдала "
        "(последовательные финальные проходы h = 1 и верхние разбиения дерева рекурсии), а при "
        "превышении числа аппаратных потоков (p = 16, 32) ускорение падает из-за переподписки. "
        "Фильтрация замеров через доверительный интервал по медиане AvgTrustedIntervalMed эффективно "
        "отсекает системные выбросы."
    )
    doc.add_subsection_heading("7.2", "Выводы по лабораторной работе в целом")
    doc.add_paragraph(
        "Получены практические навыки параллельной реализации алгоритмов сортировки в OpenMP "
        "(параллельные циклы, барьерная синхронизация, модель задач task) и их применения для "
        "обработки растровых BMP-изображений медианным фильтром. Подтверждена важность оптимизации "
        "доступа к памяти (планарное представление каналов, локальные буферы без false sharing) и "
        "динамической балансировки нагрузки."
    )

    # =========================================================================
    # 8. ОТВЕТЫ НА КОНТРОЛЬНЫЕ ВОПРОСЫ (КРАТКО — ТОЛЬКО ОСНОВНАЯ СУТЬ)
    # =========================================================================
    doc.add_section_heading("8", "Ответы на контрольные вопросы", page_break=True)

    qa_short = [
        ("1. Охарактеризуйте алгоритм сортировки «Пузырьком».",
         "Устойчивый обменный алгоритм, многократно проходящий массив и меняющий местами неупорядоченные "
         "соседние элементы, «выталкивая» максимум в конец. Сложность O(n²) в среднем/худшем и O(n) в лучшем "
         "случае, память O(1); плохо распараллеливается из-за последовательной зависимости соседних шагов."),

        ("2. Приведите алгоритм сортировки «Чет-нечетной перестановки».",
         "Модификация пузырька из n фаз: на чётных фазах сравниваются и обмениваются непересекающиеся пары "
         "(a[0], a[1]), (a[2], a[3]), ..., на нечётных — (a[1], a[2]), (a[3], a[4]), ... Внутри каждой фазы "
         "сравнения независимы и выполняются параллельно. Последовательная сложность O(n²), параллельная — O(n)."),

        ("3. Раскройте сущность сортировки алгоритмом «Шелла».",
         "Обобщение сортировки вставками по убывающей последовательности шагов h_k > ... > 1 (например, Кнута "
         "h = 3h + 1). На каждом шаге упорядочиваются h независимых подмассивов с шагом h, благодаря чему к "
         "шагу h = 1 массив почти отсортирован. Сложность O(n^(4/3))–O(n·log²n), память O(1)."),

        ("4. Охарактеризуйте сортировку алгоритмом qsort.",
         "Алгоритм «разделяй и властвуй»: выбирается опорный элемент (pivot), массив разбивается на части "
         "меньше и больше опорного (либо 3 группы <, =, >), после чего части сортируются рекурсивно. "
         "Средняя сложность O(n log n), худшая O(n²), стек O(log n) при устранении хвостовой рекурсии."),

        ("5. Перечислите виды параллелизма для алгоритмов сортировки.",
         "Параллелизм по данным (разбиение массива на независимые блоки/подмассивы), параллелизм задач "
         "(динамические ветви рекурсии в qsort/mergesort через omp task), конвейерный параллелизм "
         "(сортирующие сети) и SIMD-векторизация сравнений-обменов."),

        ("6. Поясните известные вам способы распараллеливания сортировки «Чет-нечетной перестановки».",
         "Мелкозернистый (#pragma omp for внутри каждой из n фаз — страдает от накладных расходов n барьеров) "
         "и крупнозернистый блочный (схема Гергеля: массив делится на p блоков, каждый сортируется локально, "
         "затем выполняется всего p фаз парного слияния-разделения Compare-Split смежных блоков)."),

        ("7. Приведите параллельные алгоритмы сортировки «Шелла».",
         "Параллельная сортировка h независимых подмассивов директивой #pragma omp for на каждом шаге h "
         "(эффективна при h ≥ p); блочная параллельная сортировка Шелла с операциями Compare-Split на "
         "убывающих расстояниях между блоками и завершающими чёт-нечётными фазами."),

        ("8. Приведите параллельные алгоритмы быстрой сортировки.",
         "Рекурсивный параллелизм задач (#pragma omp task с порогом отсечения cutoff); полностью параллельное "
         "разбиение на базе префиксных сумм (parallel partition); выборочная сортировка PSRS (разбиение "
         "данных по регулярным разделителям для равномерной балансировки потоков)."),

        ("9. Опишите подходы к параллельной обработке изображений.",
         "Геометрическая декомпозиция по данным (построчное/ленточное или 2D-тайловое разбиение матрицы "
         "пикселей), функциональная декомпозиция по цветовым каналам (R, G, B через sections), конвейерная "
         "обработка потока кадров и аппаратное ускорение (SIMD, GPU)."),

        ("10. Раскройте суть медианной фильтрации изображения.",
         "Нелинейная пространственная фильтрация скользящим нечётным окном Ksize×Ksize: значения пикселей "
         "окрестности (отдельно для R, G, B) сортируются, и в выходной пиксель записывается медиана "
         "(средний элемент). Эффективно подавляет импульсный шум, сохраняя резкость границ."),

        ("11. Опишите известные вам алгоритмы медианной фильтрации изображения.",
         "Классический с полной сортировкой окна O(K² log K); поиск медианы без полной сортировки QuickSelect "
         "O(K²); скользящая гистограмма Хуанга O(K); алгоритмы постоянного времени O(1) на пиксель Перро–Эбера "
         "и быстрый алгоритм Бардина на основе постолбцовых гистограмм."),

        ("12. Охарактеризуйте подходы к обработке изображений с применением параллельных вычислений.",
         "Декомпозиция кадра на непересекающиеся горизонтальные полосы строк или 2D-блоки, распределяемые "
         "между потоками через #pragma omp parallel for (static/dynamic/guided) с чтением из исходного "
         "буфера и записью в отдельный выходной буфер (double buffering) без блокировок."),

        ("13. Раскройте аспекты использования параллельных вычислений при реализации медианного фильтра.",
         "Независимость вычисления каждого выходного пикселя; выделение приватных стековых массивов окна "
         "MEDMAS для каждого потока (исключение гонок и false sharing); динамическое планирование строк "
         "schedule(dynamic) для выравнивания нагрузки на неоднородных участках; корректная обработка границ."),

        ("14. Назовите известные вам возможные проблемы при реализации параллельной обработки изображений.",
         "Гонки данных при записи в исходный буфер или общем временном массиве; ложное разделение кэш-линий "
         "(false sharing); дисбаланс нагрузки; ограничение пропускной способности ОЗУ при неоптимальном обходе "
         "по столбцам; переподписка потоков и ошибки обработки краёв/padding BMP."),

        ("15. Охарактеризуйте способы представления данных изображения для повышения быстродействия алгоритмов обработки.",
         "Выделение памяти единым непрерывным блоком; планарное хранение каналов SoA (отдельные массивы "
         "R, G, B) вместо RGBTRIPLE (AoS) для построчного memcpy и SIMD; выравнивание строк по 16–64 байтам; "
         "дополнение изображения рамкой шириной R (border padding) для исключения проверок границ в цикле."),
    ]

    for q, a in qa_short:
        doc.add_paragraph(q, align="left", indent_cm=1.25, bold=True, size_pt=14.0, space_before_pt=5, space_after_pt=1, keep_next=True)
        doc.add_paragraph(a, align="both", indent_cm=1.25, size_pt=14.0, space_after_pt=2)

    out_docx = "БПИ24-02. Пузынин. Лаб. № 3.docx"
    doc.save(out_docx)
    print(f"Successfully generated compact GOST report: {out_docx}")


if __name__ == "__main__":
    main()
