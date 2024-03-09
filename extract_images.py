import cv2
from numpy import array
from pdf2image import convert_from_path

# convert PDF to image then to array ready for opencv
pages = convert_from_path('input/2310.16497.pdf', dpi=100)
img = array(pages[11])

#height, width = img.shape[:2]
#desired_width = 2**8
#ratio = desired_width / width
#desired_height = int(height * ratio)
#img = cv2.resize(img, (desired_width, desired_height))

#blur = cv2.pyrMeanShiftFiltering(img, 11, 21)
gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
thresh = cv2.threshold(gray, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)[1]

cnts = cv2.findContours(thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
cnts = cnts[0] if len(cnts) == 2 else cnts[1]
ctr = 0
for c in cnts:
  peri = cv2.arcLength(c, True)
  approx = cv2.approxPolyDP(c, 0.015 * peri, True)
  if len(approx) == 4:
    x,y,w,h = cv2.boundingRect(approx)
    if w * h < 1000: 
      continue
    print(w * h)
    cv2.rectangle(img,(x,y),(x+w,y+h),(36,255,12),2)
    roi = img[y:y+h, x:x+w]
    cv2.imwrite('output/' + str(ctr) + '.png', roi, [cv2.IMWRITE_PNG_COMPRESSION, 0])
    ctr += 1

cv2.imwrite('output/img.png', img, [cv2.IMWRITE_PNG_COMPRESSION, 0])
cv2.imwrite('output/thresh.png', img, [cv2.IMWRITE_PNG_COMPRESSION, 0])
#cv2.imshow('thresh', thresh)
#cv2.imshow('image', img)
#cv2.waitKey(0)
#cv2.destroyAllWindows()
