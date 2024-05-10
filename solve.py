import cv2

import numpy as np
import PIL
from PIL import Image

# Split images into two folders: split_ordered and split_jumbled
# Loop over split_jumbled and find its best matches in split_ordered: store: (corresponding index (in `split_ordered`), index in split_jumbled)
# as a dictionary k,v pair in a new dict mydict
# sort by branch and bound

# combine images horizontally in groups of 4 then finally vertically

jumpled_img_path = '/content/drive/MyDrive/jumbled.png'
jumpled_split_dir = '/content/drive/MyDrive/split_jumbled'
ordered_img_path = '/content/drive/MyDrive/ordered.png'
ordered_split_dir = '/content/drive/MyDrive/split_ordered'
final_img_path = '/content/drive/MyDrive/cube_combined.png'

# split images to tiles
def split_img(loc_img, loc_out, ht, vt):
  img = cv2.imread(loc_img)
  height, width = img.shape[:2]

  qh = int(height / ht)
  qw = int(width / vt)

  i = 1
  for r in range(0,img.shape[0],qh):
      for c in range(0,img.shape[1],qw):
          h, w = img[r:r+qh, c:c+qw,:].shape[:2]
          # print(h,w)
          if(h < qh / 2 or w < qw / 2): # ignore edges
            continue

          cv2.imwrite(loc_out + f"/{i}.png",img[r:r+qh, c:c+qw,:])
          i += 1

# make dictionary of matching images
def make_dict():
  mydict = dict()
  empty_tile = 0

  for i in range(1,17):
    template = cv2.imread(f"{jumpled_split_dir}/{i}.png", 0)
    min_match = 0.25
    conf_found = -1
    index_found = 0

    for j in range(1,17):
        img = cv2.imread(ordered_split_dir + str(j) +".png", 0)
        if img.shape != template.shape:
          img = cv2.resize(img, template.shape[::-1], interpolation=cv2.INTER_AREA)

        # print(f"Confidence for {j}:")
        # print(cv2.matchTemplate(img, template, cv2.TM_CCOEFF_NORMED).max())

        conf = cv2.matchTemplate(img, template, cv2.TM_CCOEFF_NORMED).max()

        # ignore empty tile and select best match so far
        if(conf > min_match and conf > conf_found):
            conf_found = conf
            index_found = j
    
    if(conf_found == -1): # empty tile case
      # print("empty: ", i)
      empty_tile = i
    else:
      mydict[index_found] = i

  # add empty tile
  empty_tile_final = list(set(range(1,17)) - set(mydict.keys()))[0] # final position of empty tile in ordered.png
  mydict[empty_tile_final] = empty_tile
  
  # sort keys in dictionary
  myKeys = list(mydict.keys())
  myKeys.sort()
  sorted_dict = {i: mydict[i] for i in myKeys}

  return sorted_dict

# split images

# split_img(jumbled_img_path, jumbled_split_dir, 4, 4)
# split_img(ordered_img_path, ordered_split_dir, 4, 4)

# find matches

matches = make_dict()
print(matches)

# combine images

imgs_v = []
vals = list(matches.values())
for i in range(4):
  imgs2 = [ Image.open(jumpled_split_dir + str(x) + '.png') for x in vals[i*4:i*4+4] ]
  imgs_comb = np.hstack([ x for x in imgs2 ])
  imgs_v.append(imgs_comb)

imgs_comb = np.vstack([i for i in imgs_v])
imgs_comb = Image.fromarray(imgs_comb)
imgs_comb.save(final_img_path)