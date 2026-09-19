#!/bin/sh
set -e
ninja -C build
curl -X DELETE http://aqplus-minivz/cores/aq32/boot.aq32
curl -X PUT -T build/boot.bin http://aqplus-minivz/cores/aq32/boot.aq32
printf '\x1E' | curl --data-binary @- http://aqplus-minivz/keyboard
