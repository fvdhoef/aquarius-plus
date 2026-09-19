#!/bin/sh
curl -X DELETE http://aqplus.local/cores/aqbase/aqbase.core
curl -X PUT -T fpga_top.bit http://aqplus.local/cores/aqbase/aqbase.core
