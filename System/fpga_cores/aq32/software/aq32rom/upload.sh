#!/bin/sh
curl -X DELETE http://aqplus-minivz/aq32.rom
curl -X PUT -T build/aq32rom.bin http://aqplus-minivz/aq32.rom
