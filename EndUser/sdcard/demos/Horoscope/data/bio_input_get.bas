100 load screen "bioInput.scr"
110 dim ta(51)
120 get screen (3,19)-(36,21),*ta
130 save "bioInp.arry",*ta
200 load screen "bioInputTxt.scr"
210 get screen (3,19)-(36,21),*ta
220 save "bioTxt.arry",*ta
300 cls
