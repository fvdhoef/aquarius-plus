#!/bin/sh
INCLUDES=$(cat .vscode/settings.json | jq -r '."verilog.linting.verilator.includePath"[]' | sed -e 's/^/-I/')
verilator --quiet-stats -Wall -Wpedantic --lint-only verilator.vlt $INCLUDES fpga_top.v
