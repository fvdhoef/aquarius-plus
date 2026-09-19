100 'load screen "horoscope.scr"
101 load screen "horoscope_blk.scr"
110 dim c1(11)
120 get screen (9,20)-(30,20),*c1
130 save "tClick1.arry",*c1
200 'load screen "horoscope_2.scr"
201 load screen "horoscope_blk_2.scr"
210 get screen (9,20)-(30,20),*c1
220 save "tClick2.arry",*c1
300 cls
