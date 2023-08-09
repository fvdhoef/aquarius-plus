![Aquarius+ Logo](../EndUser/images/aquarius_plus_logo_BLUE.png)

# The Maker's Guide to Manufacturing an Aquarius+ #
Updated 08 AUG 2023

by Sean Harrington, sph@1stage.com, 

## Abstract:

The subfolders in this directory contain all the items necessary for a Maker to manufacture and assemble an Aquarius+ computer. For simplicity, the PCB section of this guide is geared towards using [JLCPCB](https://jlcpcb.com). Other vendors can absolutely be used, but this is the easiest path we have found.

## Case Manufacturing:
This guide is only an overview and is not meant to be an exhaustive guide for ordering or making your own case.

### 3D Printing Terms:
 - FDM - Fused Deposition Modeling, a process of 3D printing where molten plastic is squeezed out as lines of plastic, characterized by its low-cost and ease of use. Common plastic materials used are PLA and ABS.
 - SLA - Sterolithography, a process of 3D printing where a photosensitive resin is exposed to UV light in successive cross-sections of a model, building layers of the print. The finished print must be cleaned of excess resin and then fully cured/hardened by a strong source of UV light, such as the sun or a curing chamber.
 - SLT - Stereo Lithography file, a format for describing the exterior shell of a 3D object
 - STEP - A 3D model file format which is more detailed and transportable than STL

### Process:
 - **Using a Predefined Online Project**
   1. Go to the project at PCBWay.
   2. Add top and bottom pieces, and select your material of choice.
   3. Order and pay.
 - **Printing Through a Service Bureau**
   1. Download the STL or STEP files for the TOP and BOTTOM.
   2. Upload to your service bureau of choice.
   3. Select your material of choice.
   4. Order and pay.
 - **Printing yourself**
   1. Download the STL or STEP files for the TOP and BOTTOM. Use the FDM version of the TOP piece to aid in bed-adhesion for FDM printers, otherwise use the standard TOP for SLA.
   2. Slice it in your printer's slicing tool.
      - **TOP** does NOT require supports.
      - **BOTTOM** DOES require supports.
   3. Print, clean, cure, etc. and install your PCB after programming it with the starter image.


------
## PCB Manufacturing:
This guide assumes that the case will be handled separately.

### PCB Terms:
 - BOM - Bill of Materials
 - CPL - Component Placement Layout/List (sometimes referred to as CPL1)
 - PCB - Printed Circuit Board
 - SMT - Surface mount (sometimes referred to as SMD)
 - THT - Through hole

### Process:
This guide assumes that the maker will be having a third-party manufacturer create and assemble the PCB. It does not cover user-manufactered PCBs or assembly using a reflow oven or other manual tasks.
1. **Order the parts**
    - On the [JLCPCB](https://jlcpcb.com) main site, log in and go to Parts Manager, then Order Parts, then JLCPCB Parts.
    - From the JLCPCB Assembly Parts page, select BOM Tool.
    - Upload the BOM file to the site. From the pop-up, select the number of PCBs you want to manufacture. This multiplies the quantities for you for each item.
    - **(more details needed here)**
    - Add to cart, pay for items, and wait.
    - Over the next several days, you will get email notifications as items arrive into your parts inventory. Sometimes, there will be an increase or decrease in the amount you paid, so JLCPCB will either ask you to pay the difference (part is more expensive than estimated) or they will credit your purchase (part was less expensive than estimated). Acknowledge these messages, pay the difference (as needed), and wait.
    - Once all of the parts are in your inventory, proceed to the next step.
3. **Order the assembled PCB**
    - Log into JLCPCB, and start a new order.
    - Upload the Gerber files.
    - Mark the order as SMT Assembly.
    - Upload the BOM and CPL files.
    - Ensure there are sufficient quantities of the parts you need. If not, go back to Order the parts section.
    - Review the placement of all parts. Note that some of the items (ESP32, switch, Z80, expansion port) don't have a 3D model to go with their part, so a small 2x2 checkerboard is used. Don't worry about these items. The CPL1 file has been tuned to accurately place the components, so this step is for QA to make sure nothing has gone wrong.
      - If you select a part (blue highlight), you can nudge it with the arrow keys. RIGHT nudges increase the X. LEFT nudges decrease the X. UP nudges increase the Y. DOWN nudges decrease the Y.
      - For THT parts, it's easier to align the parts from the BOTTOM view, but remember that your X axis is now flipped, so a RIGHT nudge decreases the X and a LEFT nudge increases it.
      - Each "nudge" is a difference of 0.0635mm, and can be added or subtracted from the CPL file accordingly... i.e. if you nudge a part UP (increase y axis) five times, you're modifying it 5 x 0.0635mm or 3.175. So if it's value in the CPL file is -45.025, the new value should be -44.7075.
    - JLCPCB will automatically set the "confirm PCB" and "confirm placement" options, and let you know that there will be a small charge for this.
    - Add to cart, pay for items, and wait.
    - You will get at least two emails. The first will be to confirm your PCB file. This is done in your Order History window. Confirm it. The second email will be to confirm the layout of the components. In your Order History window, the lower Confirm Placement button brings a pop-up which shows the placement of the SMT parts (THT or throughhole is done separately). Everything should be fine, but reply to the email and ask to see a 3D of the cartridge port placement. This will take a day or so, but so long as the port is shown in the far right columns of pins, you can confirm placement.
    - Check the progress of your order daily to make sure there's nothing you need to confirm. Once the product has shipped, you can move onto the next step.
4. **Assemble the Aquarius+**
    - Make sure your shipment package has no obvious damage. If it does, take pictures before you open the box and save them for later. Most of the time, a ding or puncture will not be a problem, as the contents are typically very well wrapped.
    - There are sometimes small beads or blobs of solder that get deposited on the top of the board. With a magnifier, check ALL components to ensure there are no obvious solder blobs or bridges on the components, both SMT and THT. Now, check the BACK side of the board to ensure all THT components have solid, well-tented solder joints. Check the metal shrouds around the USB, controllers (DB9), VGA, power switch, and SD card socket to remove any accidental blobs or splashes. They should pop free with minimal force.
    - To assist in the SMT mounting process, manufacturers add additional "rails" to the PCB for handling. Using a small pair of needle-nose pliers, carefully remove these rails by bending along the V cut that has been cut into the PCB next to these rails. Be cautious not to damage any components that rest on this rail (LED, SD card socket, etc.). You can use a small file to remove any remaining burrs along these new edges. The rails can be thrown away.
    -  The wave solder that is used for the THT components leaves a sticky residue, as does the flux used for the SMT components. This residue should be cleaned using a two-step process, first with an alcohol-based solvent and a soft-bristle brush (an old toothbrush works great), and second with a mild de-greasing detergent, also agitated with a soft-bristle brush. Clean BOTH sides of the PCB thoroughly, first with the alcohol (I use left-over hand sanitizer from COVID), and then with the detergent (I use Krud Kutter), rinsing with water after each scrub down. Take care around delicate parts: the goal is to remove residue, not PCB components. Afterwards, either allow to air dry for 24 hours, or use a gentle, low-pressure compressed air source to drive the remaining water from all areas of the PCB (ports, power switch, under chips), again, taking care to not damage PCB components.
    - Once PCB has been cleaned and thoroughly dried, it's time to program the initial image for the Aquarius+. You're programming the ESP32 with the image that then gets sent to the Spartan 6 FPGA. From the GitHub, download the newest AquariusPlus-Firmware-xxx.zip bundle, decompress it, and follow the instructions for setting up the ESP software on your system. Using a USB to USB Micro cable and three 0.1" / 2.54mm pitch jumpers, connect the pins at JP1, JP2, and JP3, and plug the cable into the Aquarius+ power port and into your computer. Switch on the Aquarius+ power supply and note which COM port it appears on.
    - **(more details needed here)**
