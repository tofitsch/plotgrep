#https://medium.com/analytics-vidhya/image-search-engine-using-image-hashing-technique-in-python-e6749dacc8f7

import glob
import cv2
from scipy.spatial.distance import hamming
import numpy as np

def hash_array_to_hash_hex(hash_array):
  # convert hash array of 0 or 1 to hash string in hex
  hash_array = np.array(hash_array, dtype = np.uint8)
  hash_str = ''.join(str(i) for i in 1 * hash_array.flatten())
  return (hex(int(hash_str, 2)))

def hash_hex_to_hash_array(hash_hex):
  # convert hash string in hex to hash values of 0 or 1
  hash_str = int(hash_hex, 16)
  array_str = bin(hash_str)[2:]
  return np.array([i for i in array_str], dtype = np.uint8)

# hash dictionary to store hash values on images
image_hash_dict = {}

image_names = sorted(glob.glob('page*plot*.ppm')) + ['test.ppm']

for name in image_names:
  img = cv2.imread(name)
  # resize image and convert to gray scale
  img = cv2.resize(img, (64, 64))
  img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
  img = np.array(img, dtype = np.float32)
  # calculate dct of image 
  dct = cv2.dct(img)
  # to reduce hash length take only 8*8 top-left block 
  # as this block has more information than the rest
  dct_block = dct[: 8, : 8]
  # caclulate mean of dct block excluding first term i.e, dct(0, 0)
  dct_average = (dct_block.mean() * dct_block.size - dct_block[0, 0]) / (dct_block.size - 1)
  # convert dct block to binary values based on dct_average
  dct_block[dct_block < dct_average] = 0.0
  dct_block[dct_block != 0] = 1.0
  # store hash value
  image_hash_dict[name] = hash_array_to_hash_hex(dct_block.flatten())

print(image_hash_dict)

for image_name in image_hash_dict.keys():
  distance = hamming(
#    hash_hex_to_hash_array(image_hash_dict[image_name]),
#    hash_hex_to_hash_array(image_hash_dict['test/test.png'])
    hash_hex_to_hash_array(image_hash_dict[image_name]),
    hash_hex_to_hash_array(image_hash_dict['test.ppm'])
  )
  print("{0:<30} {1}".format(image_name, distance))
