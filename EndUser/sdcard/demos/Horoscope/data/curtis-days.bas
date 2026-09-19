1000 REM Horoscope & Biorhythm
1100 SET FAST ON
1200 input y,m,d
1210 ja=(14-m)/12
1220 jy=y-ja
1230 jm=m+12*ja-3
1240 j=d+int((153*jm+2)/5)+jy*365+int(jy/4)-int(jy/100)+int(jy/400)
1250 j=int(j-693901)
2000 print j
