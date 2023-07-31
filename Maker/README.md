![Aquarius+ Logo](../EndUser/images/aquarius_plus_logo_BLUE.png)

**Maker's Guide to Manufacturing an Aquarius+**

## Abstract:

This folder contains all the items necessary for a Maker to manufacture and assemble an Aquarius+ computer. For simplicity, this guide is currently geared towards using [JLCPCB](https://jlcpcb.com). Other vendors can absolutely be used, but this is the easiest path we have found.

## Overview of the Manufacturing Process:

1. Upload the BOM to order and pay for the components for your JLCPCB parts inventory.
2. Order the PCB, upload the BOM and CPL1 files, verify placement, pay for the order.
3. Receive, clean, and install PCBs into cases, load firmware.

## Folder Contents

| Directory           | Description                                                                                                                                                                     |
| ------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [EndUser](EndUser/) | These files are meant for the normal end-user, that just wants to play and work with the system. You can find some nice sample programs, games and music to run on system here. |
| [SDK](SDK/)         | If you want to develop your own software for the Aquarius+, look here.                                                                                                          |
| [System](System/)   | This contains all other files related to the system, including: Schematics/PCB, FPGA, ESP32, Emulator                                                                           |
