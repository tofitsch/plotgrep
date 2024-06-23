# https://pymupdf.readthedocs.io/en/latest/recipes-text.html
# TODO: do this in C instead of python
import os
import sys
import fitz

_, file_name, p, term = sys.argv

doc = fitz.open(file_name)

p = int(p)

if p < len(doc): 
   
  page = doc[p]

  rl = page.search_for(term, quads=True)

  if len(rl) > 0:

    page.add_squiggly_annot(rl)

    doc.save('tmp.pdf')

    os.system(f'zathura --page {p + 1} tmp.pdf')
