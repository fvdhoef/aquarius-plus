#!/bin/sh
./png2bm.py sierra_001.png image1.bm4
./png2bm.py parrots.png image2.bm4
./png2bm.py monkeyisland.png image3.bm4
../txt2bas.py image.txt image.bas
