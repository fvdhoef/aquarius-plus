#!/bin/sh
set -e
ninja -C build
curl -X DELETE http://aqplus-minivz/cores/aq32/midiplay.aq32
curl -X PUT -T build/midiplay.aq32 http://aqplus-minivz/cores/aq32/midiplay.aq32
printf '\x1Emidiplay\n' | curl --data-binary @- http://aqplus-minivz/keyboard
