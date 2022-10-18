#!/bin/bash
rsync --include='**.gitignore' --exclude='.git/' --exclude='*.vcd' --filter=':- .gitignore' --progress -a server:Work/aquarius-plus/fpga/ ./
