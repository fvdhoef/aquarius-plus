' open "test.bin" FOR OUTPUT AS a%
' q$ = "Hello world"
' write #a%, q$
' close #a%

' open "test.bin" FOR INPUT AS a%
' input #a%, q$, a
' print q$

line input "Hello"; a$
print "|";a$;"|"

' ? "filesize ="; lof(a%)
' q$ = "12345"
' seek a%, 7
' read #a%, q$
' ? seek(a%)

' print q$

' open "test.bin" FOR OUTPUT AS a%
' print #a%, "Hello world"
' print #a%, "Test"
' print #a%, "123"
' close #a%

' open "test.bin" FOR INPUT AS a%
' print input$(5, a%)
' close

' open "test.bin" FOR INPUT AS a%
' while not eof(a%)
'     line input #a%, q$
'     print q$
' wend
' close #a%

' kill "test.bin"



' for i = 1 to 10
'     print #a%, "Hello world"; tab(40); i
' next


' print inkey$

' print "bla"+"dinges"
' print "a" = "a"
' print "ab" >= "ab"

' print inkey$
' print timer

' print "0123456789012345678901234567890"
' print 1,2,3,4,5,6,7
' print ,, "bla",;
' print "ja"

' print ucase$("Hi there")
' print lcase$("Hi there")
' print ltrim$("          Hi there")
' print rtrim$("          Hi there    "); "_"
' print "blaat"
' print tab(20); "Hi"

' defint a-z

' dim a$(10)

' for i = 0 to 9
'     a$(i) = chr$(65+i)
' next

' erase a$,b$

' for i = 0 to 9
'     print i, a$(i)
' next
