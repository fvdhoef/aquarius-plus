1000 DIM D$(10)
2000 WS$ = "http://api.open-notify.org/iss-now.json"
3000 LOAD WS$, *D$
4000 SAVE "iss-now.json",*D$
