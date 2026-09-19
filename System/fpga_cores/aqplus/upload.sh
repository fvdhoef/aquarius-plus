#!/bin/sh
curl -X DELETE http://aqplus/cores/aqplus/aqplus.core
curl -X PUT -T aqp_top.bit http://aqplus/cores/aqplus/aqplus.core
