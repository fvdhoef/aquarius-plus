#!/bin/sh
set -e
ninja -C build
curl -X DELETE http://aqplus-minivz/cores/aq32/edit.aq32
curl -X PUT -T build/edit.aq32 http://aqplus-minivz/cores/aq32/edit.aq32
printf '\x1Eedit\n' | curl --data-binary @- http://aqplus-minivz/keyboard
