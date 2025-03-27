#!/bin/sh
set -e
ninja -C build
curl -X DELETE http://aqplus-minivz/aq32.rom
curl -X PUT -T build/aq32rom.bin http://aqplus-minivz/aq32.rom
printf '\x1E' | curl --data-binary @- http://aqplus-minivz/keyboard
