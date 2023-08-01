![Aquarius+ Logo](../EndUser/images/aquarius_plus_logo_BLUE.png)

**Maker's Guide to Manufacturing an Aquarius+**

## Abstract:

This folder contains all the items necessary for a Maker to manufacture and assemble an Aquarius+ computer. For simplicity, this guide is currently geared towards using [JLCPCB](https://jlcpcb.com). Other vendors can absolutely be used, but this is the easiest path we have found.

## Overview of the Manufacturing Process:

1. Order the parts.
    - On the [JLCPCB](https://jlcpcb.com) main site, log in and go to Parts Manager, then Order Parts, then JLCPCB Parts.
    - From the JLCPCB Assembly Parts page, select BOM Tool.
    - Upload the aqplus_revX_BOM.xlsx file to the site. From the pop-up, select the number of PCBs you want to manufacture. This multiplies the quantities for you for each item.
    - (more to come)
3. Order the assembled PCBs.
4. Assemble the Aquarius+.

## Folder Contents

| Directory                   | Description                                                                                           |
| --------------------------- | ----------------------------------------------------------------------------------------------------- |
| aqp_prod_label_template.pdf | Template for creating a label for the bottom of the Aquarius+.                                        |
| aqplus_revX_XXXXXX.zip      | Gerber file, contains files that describe the various layers and manufacturing details of the PCB.    |
| aqplus_revX_BOM.xlsx        | Bill of Materials file, formatted to JLCPCB specifications, including their part numbers.             |
| aqplus_revX_CPL1.xlsx       | Component Placement Listing file, giving the coordinates, rotation, and part identifiers for the PCB. |
