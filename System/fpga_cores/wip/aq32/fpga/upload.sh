#!/bin/sh
curl -X DELETE http://aqplus-minivz/cores/aq32/aq32.core
curl -X PUT -T aq32_top.bit http://aqplus-minivz/cores/aq32/aq32.core
