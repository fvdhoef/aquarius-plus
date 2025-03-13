#!/bin/sh
curl -X DELETE http://aqplus-minivz/aq32.core
curl -X PUT -T aq32_top.bit http://aqplus-minivz/aq32.core
