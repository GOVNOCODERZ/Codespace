import os
import subprocess
import tempfile
import zipfile
import xml.etree.ElementTree as ET

docx_name = "БПИ24-02. Пузынин. Лаб. № 3.docx"
xlsx_name = "Результаты_исследования_Лаб3_OpenMP.xlsx"
zip_name  = "БПИ24-02. Пузынин. Лаб. № 3.zip"

# 0. Remove old duplicate filenames if present
for old_f in [
    "Лабораторная_работа_3_Отчет.docx",
    "Лабораторная_работа_3_Результаты.xlsx",
    "Лабораторная_работа_3_Исходный_код.zip",
]:
    if os.path.exists(old_f):
        try:
            os.remove(old_f)
        except PermissionError:
            print(f"[INFO] Skipped deleting locked old file: {old_f}")

# 1. Create clean source code zip (NO binaries/build outputs)
src_files = [
    "src/common_stats.h",
    "src/BMPFileRW.h",
    "src/task3_1_sort.cpp",
    "src/task3_2_median.cpp",
    "src/Makefile",
    "src/README.txt",
]
with zipfile.ZipFile(zip_name, "w", zipfile.ZIP_DEFLATED) as zf:
    for f in src_files:
        zf.write(f, os.path.basename(f))
print(f"[OK] Created {zip_name} ({os.path.getsize(zip_name):,} bytes, {len(src_files)} source files)")

# 2. Validate .docx and .xlsx XML & zip integrity
for pkg in [docx_name, xlsx_name]:
    assert os.path.exists(pkg), f"Missing {pkg}"
    with zipfile.ZipFile(pkg, "r") as zf:
        bad = zf.testzip()
        assert bad is None, f"Corrupt zip entry {bad} in {pkg}"
        xml_cnt = 0
        for name in zf.namelist():
            if name.endswith(".xml") or name.endswith(".rels"):
                ET.fromstring(zf.read(name))
                xml_cnt += 1
        print(f"[OK] {pkg}: {os.path.getsize(pkg):,} bytes, {len(zf.namelist())} files, {xml_cnt} valid XML parts")

# 3. Deep verify GOST margins, line spacing, no code listings, embedded media & relationships in .docx
with zipfile.ZipFile(docx_name, "r") as zf:
    doc_xml = zf.read("word/document.xml").decode("utf-8")
    rels_xml = zf.read("word/_rels/document.xml.rels").decode("utf-8")
    assert 'w:left="1701"' in doc_xml, "Left margin != 3 cm"
    assert 'w:right="1134"' in doc_xml, "Right margin != 2 cm"
    assert 'w:top="1134"' in doc_xml, "Top margin != 2 cm"
    assert 'w:bottom="1134"' in doc_xml, "Bottom margin != 2 cm"
    assert 'w:line="240"' in doc_xml, "Line spacing != 1.0"
    assert 'w:firstLine="709"' in doc_xml, "First-line indent != 1.25 cm"
    assert "<w:titlePg/>" in doc_xml, "Missing titlePg"
    assert "Consolas" not in doc_xml, "Code listing should NOT be in report!"
    assert "Д.А. Пузынин" in doc_xml and "БПИ24-02, 24151438" in doc_xml, "Missing student title page info"
    rels_root = ET.fromstring(rels_xml)
    media_targets = [el.attrib["Target"] for el in rels_root if el.attrib["Target"].startswith("media/")]
    for t in media_targets:
        assert f"word/{t}" in zf.namelist(), f"Missing embedded media {t}"
    print(f"[OK] GOST margins (3/2/2/2 cm), line=1.0, indent=1.25 cm, titlePg, NO listings, {len(media_targets)} PNG figures verified")

# 4. Extract source zip to temp dir and compile both targets with -Wall -Wextra -Werror
with tempfile.TemporaryDirectory() as td:
    with zipfile.ZipFile(zip_name, "r") as zf:
        zf.extractall(td)
    for cpp, exe in [("task3_1_sort.cpp", "t1.exe"), ("task3_2_median.cpp", "t2.exe")]:
        cmd = [
            r"C:\msys64\ucrt64\bin\g++.exe",
            "-std=c++17", "-fopenmp", "-O3", "-Wall", "-Wextra", "-Werror",
            os.path.join(td, cpp), "-o", os.path.join(td, exe)
        ]
        res = subprocess.run(cmd, capture_output=True, text=True)
        assert res.returncode == 0, f"Compile failed for {cpp}: {res.stderr}"
        print(f"[OK] Clean standalone compile (-Wall -Wextra -Werror): {cpp}")
