#!/bin/sh
curl -X DELETE http://aqplus-minivz.local/cores/my32/my32.core
curl -X PUT -T fpga_top.bit http://aqplus-minivz.local/cores/my32/my32.core
