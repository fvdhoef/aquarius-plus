#!/bin/sh
set -e
ninja -C build
(cd help/content; ./convert.py)
curl -X DELETE http://aqplus-minivz/cores/aq32/basic.aq32
curl -X PUT -T build/basic.aq32 http://aqplus-minivz/cores/aq32/basic.aq32
curl -X PUT -T help/content/basic.hlp http://aqplus-minivz/cores/aq32/basic.hlp
printf '\x1Ebasic\n' | curl --data-binary @- http://aqplus-minivz/keyboard
