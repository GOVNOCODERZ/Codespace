import os
import subprocess
import tempfile
import zipfile
import xml.etree.ElementTree as ET

docx_name = "Lab5/БПИ24-02. Пузынин. Лаб. № 5.docx"
xlsx_name = "Lab5/Результаты_исследования_Лаб5_OpenMP_oneTBB.xlsx"
zip_name  = "Lab5/БПИ24-02. Пузынин. Лаб. № 5.zip"

src_files = [
    "Lab5/src/common_stats.h",
    "Lab5/src/BMPFileRW.h",
    "Lab5/src/lab5_convolution.cpp",
    "Lab5/src/Makefile",
    "Lab5/src/README.txt",
]

with zipfile.ZipFile(zip_name, "w", zipfile.ZIP_DEFLATED) as zf:
    for f in src_files:
        zf.write(f, os.path.basename(f))
print(f"[OK] Created {zip_name} ({os.path.getsize(zip_name):,} bytes, {len(src_files)} source files)")

for pkg in [docx_name, xlsx_name]:
    assert os.path.exists(pkg), f"Missing {pkg}"
    with zipfile.ZipFile(pkg, "r") as zf:
        assert zf.testzip() is None, f"Corrupt zip in {pkg}"
        xml_cnt = 0
        for name in zf.namelist():
            if name.endswith(".xml") or name.endswith(".rels"):
                ET.fromstring(zf.read(name))
                xml_cnt += 1
        print(f"[OK] {pkg}: {os.path.getsize(pkg):,} bytes, {len(zf.namelist())} files, {xml_cnt} valid XML parts")

with zipfile.ZipFile(docx_name, "r") as zf:
    doc_xml = zf.read("word/document.xml").decode("utf-8")
    rels_xml = zf.read("word/_rels/document.xml.rels").decode("utf-8")
    assert 'w:left="1701"' in doc_xml
    assert 'w:right="1134"' in doc_xml
    assert 'w:top="1134"' in doc_xml
    assert 'w:bottom="1134"' in doc_xml
    assert 'w:line="240"' in doc_xml
    assert 'w:firstLine="709"' in doc_xml
    assert "<w:titlePg/>" in doc_xml
    assert "Consolas" not in doc_xml
    assert "Д.А. Пузынин" in doc_xml and "БПИ24-02, 24151438" in doc_xml
    assert "Лабораторная работа № 5" in doc_xml
    rels_root = ET.fromstring(rels_xml)
    media = [el.attrib["Target"] for el in rels_root if el.attrib["Target"].startswith("media/")]
    for m in media:
        assert f"word/{m}" in zf.namelist(), f"Missing {m}"
    print(f"[OK] GOST margins (3/2/2/2 cm), line=1.0, indent=1.25 cm, NO listings, {len(media)} PNG figures verified")

# Independent standalone compile test from the zip archive
with tempfile.TemporaryDirectory() as td:
    with zipfile.ZipFile(zip_name, "r") as zf:
        zf.extractall(td)
    cmd = [
        r"C:\msys64\ucrt64\bin\g++.exe",
        "-std=c++17", "-fopenmp", "-O3", "-Wall", "-Wextra", "-Werror",
        os.path.join(td, "lab5_convolution.cpp"), "-ltbb12", "-o", os.path.join(td, "lab5_test.exe")
    ]
    res = subprocess.run(cmd, capture_output=True, text=True)
    assert res.returncode == 0, f"Compile failed: {res.stderr}"
    print("[OK] Clean standalone compile (-Wall -Wextra -Werror -fopenmp -ltbb12): lab5_convolution.cpp")
