#!/bin/bash
set -e
ninja -C build
curl -X DELETE http://aqplus-minivz.local/cores/aqua-8/aqua-8.bin
curl -X PUT -T build/aqua-8.bin http://aqplus-minivz.local/cores/aqua-8/aqua-8.bin
printf '\x1E' | curl --data-binary @- http://aqplus-minivz.local/keyboard
