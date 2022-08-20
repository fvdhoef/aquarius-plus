#!/bin/bash
rsync --exclude='.git/' --exclude='*.vcd' --progress -a ./ server:Work/aquarius-plus/fpga/
