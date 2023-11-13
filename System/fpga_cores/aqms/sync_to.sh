#!/bin/bash
cd "${0%/*}"
rsync --delete --include='**.gitignore' --exclude='.git/' --exclude='*.vcd' --filter=':- .gitignore' --progress -a ./ server:Work/aquarius-plus/System/fpga_cores/aqms/
