import os
import fitz

# the document to annotate
doc = fitz.open("test1.pdf")

# the text to be marked
term = "factor obtained"

# work with one page only
page = doc[5]

# get list of text locations
# we use "quads", not rectangles because text may be tilted!
rl = page.search_for(term, quads=True)

# mark all found quads with one annotation
page.add_squiggly_annot(rl)

# save to a new PDF
doc.save("tmp.pdf")
os.system("zathura --page 6 tmp.pdf")
