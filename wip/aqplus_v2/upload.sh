#!/bin/sh
curl -X DELETE http://aqplus-minivz/cores/aqplus_v2/aqplus_v2.core
curl -X PUT -T aqp_top.bit http://aqplus-minivz/cores/aqplus_v2/aqplus_v2.core
