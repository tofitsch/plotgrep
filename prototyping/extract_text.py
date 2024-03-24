import sys, pathlib, fitz
fname = sys.argv[1]

with fitz.open(fname) as doc:
   text = chr(12).join([page.get_textpage().extractTEXT() for page in doc])
pathlib.Path(fname + ".txt").write_bytes(text.encode())
