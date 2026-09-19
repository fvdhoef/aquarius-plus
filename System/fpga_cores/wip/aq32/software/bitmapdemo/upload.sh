#!/bin/sh
set -e
ninja -C build
curl -X DELETE http://aqplus-minivz/cores/aq32/bitmapdemo.aq32
curl -X PUT -T build/bitmapdemo.aq32 http://aqplus-minivz/cores/aq32/bitmapdemo.aq32
printf '\x1Ebitmapdemo\n' | curl --data-binary @- http://aqplus-minivz/keyboard
