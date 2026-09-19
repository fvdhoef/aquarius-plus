#!/bin/sh
curl -X DELETE http://aqplus/cores/aqms/aqms.core
curl -X PUT -T aqp_top.bit http://aqplus/cores/aqms/aqms.core
