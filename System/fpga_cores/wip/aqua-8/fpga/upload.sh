#!/bin/sh
curl -X DELETE http://aqplus-minivz.local/cores/aqua-8/aqua-8.core
curl -X PUT -T fpga_top.bit http://aqplus-minivz.local/cores/aqua-8/aqua-8.core
