#!/bin/sh
set -e
ninja -C build
curl -X DELETE http://aqplus-minivz/cores/aq32/pcmtest.aq32
curl -X PUT -T build/pcmtest.aq32 http://aqplus-minivz/cores/aq32/pcmtest.aq32
printf '\x1Epcmtest\n' | curl --data-binary @- http://aqplus-minivz/keyboard
