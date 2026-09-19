#!/bin/sh
set -e
ninja -C build
curl -X DELETE http://aqplus-minivz/cores/aq32/tiledemo.aq32
curl -X PUT -T build/tiledemo.aq32 http://aqplus-minivz/cores/aq32/tiledemo.aq32
printf '\x1Etiledemo\n' | curl --data-binary @- http://aqplus-minivz/keyboard
