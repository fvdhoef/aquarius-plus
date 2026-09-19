#!/bin/sh
set -e
./compile_all.sh

PROJECTS=$(cat projects)
for i in $PROJECTS; do
    echo Uploading $i
    curl -X DELETE http://aqplus-minivz/cores/aq32/$i.aq32 || true
    curl -X PUT -T $i/build/$i.aq32 http://aqplus-minivz/cores/aq32/$i.aq32
done

echo Uploading basic.hlp
curl -X DELETE http://aqplus-minivz/cores/aq32/basic.hlp || true
curl -X PUT -T basic/help/content/basic.hlp http://aqplus-minivz/cores/aq32/basic.hlp

echo Done.
