# https://pymupdf.readthedocs.io/en/latest/recipes-text.html
import os
import glob
import fitz

term = 'order to record'

files = glob.glob('test1.pdf')

for d, file_name in enumerate(files):
  
  print(d, '/', len(files))

  doc = fitz.open(file_name)
  
  for p, page in enumerate(doc):
  
    rl = page.search_for(term, quads=True)
  
    if len(rl) > 0:
  
      page.add_squiggly_annot(rl)

      doc.save('tmp.pdf')
      os.system(f'zathura --page {p + 1} tmp.pdf')
      
      exit()
  
