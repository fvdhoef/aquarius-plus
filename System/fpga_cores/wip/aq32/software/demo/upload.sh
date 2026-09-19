#!/bin/sh
set -e
ninja -C build
curl -X DELETE http://aqplus-minivz/cores/aq32/demo.aq32
curl -X PUT -T build/demo.aq32 http://aqplus-minivz/cores/aq32/demo.aq32
printf '\x1Edemo\n' | curl --data-binary @- http://aqplus-minivz/keyboard
