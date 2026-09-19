# Aquarius<sup>+</sup> PCBs
Development of PCBs (Printed Circuit Boards) for the **Aquarius<sup>+</sup>** platform

## Introduction
The **Aquarius<sup>+</sup>** was developed as an open-source platform, including the hardware that comprises the system. To accomplish this and to make tools for the development and manufacture of the platform as accessible as possible, [KiCAD](https://github.com/KiCad/kicad-source-mirror) is used for PCB development, as it is a free, open source tool that has anyone can learn and use.

## Releases
Releases from this repo are by model/PCB and are geared specifically for manufacturing rather than PCB development.

> For example, if you want to make your own **Aquarius<sup>+</sup> Mini**, you would go to the [**Releases**](https://github.com/1stage/aquarius-plus-pcbs/releases) section, find the newest release for an **Aquarius<sup>+</sup> Mini**, and download it.

The **Source code (zip)** or **Source code (tar.gz)** payload will consist of the following components:
 * Gerber files, compressed in ZIP format (includes PCB, Drill, and Map files)
 * PDFs of the SCHEMATIC and PCB
 * BOM (Bill of Materials) and CPL1 (Component Placement Layout) files in XLSX (MS Excel) format, organized for JLCPCB manufacturing.
 * Release notes and other instructions in TXT format

## Development
If you want to explore how the **Aquarius<sup>+</sup>** is designed using KiCAD, you will need to Clone or Fork this repo to your local system. From there you can review and experiment with the schematic, PCB, and other files within the KiCAD environment.

****

## PCB Manufacturing

### PCB Terms
 - BOM - Bill of Materials
 - CPL - Component Placement Layout/List (sometimes referred to as CPL1)
 - PCB - Printed Circuit Board
 - SMT - Surface mount (sometimes referred to as SMD)
 - THT - Through hole

### Process
This guide assumes that the maker will be having a third-party manufacturer create and assemble the PCB. It does not cover user-manufactered PCBs or assembly using a reflow oven or other manual tasks.

#### **JLCPCB** (Cheapest)
1. **Order the parts**
    - On the [JLCPCB](https://jlcpcb.com) main site, log in and go to Parts Manager, then Order Parts, then JLCPCB Parts.
    - From the JLCPCB Assembly Parts page, select BOM Tool.
    - Upload the BOM file to the site. From the pop-up, select the number of PCBs you want to manufacture. This multiplies the quantities for you for each item.
    - Add to cart, pay for items, and wait.
    - Over the next several days, you will get email notifications as items arrive into your parts inventory. Sometimes, there will be an increase or decrease in the amount you paid, so JLCPCB will either ask you to pay the difference (part is more expensive than estimated) or they will credit your purchase (part was less expensive than estimated). Acknowledge these messages, pay the difference (as needed), and wait.
    - Once all of the parts are in your inventory, proceed to the next step.
3. **Order the assembled PCB**
    - Log into JLCPCB, and start a new order.
    - Upload the Gerber files.
    - Mark the order as SMT Assembly.
    - Upload the BOM and CPL files.
    - Ensure there are sufficient quantities of the parts you need. If not, go back to [Order the parts](#JLCPCB) section.
    - Review the placement of all parts. Note that some of the items (ESP32, switch) don't have a 3D model to go with their part, so a small 2x2 checkerboard is used. Don't worry about these items. The CPL1 file has been tuned to accurately place the components, so this step is for QA to make sure nothing has gone wrong.
      - If you select a part (blue highlight), you can nudge it with the arrow keys. RIGHT nudges increase the X. LEFT nudges decrease the X. UP nudges increase the Y. DOWN nudges decrease the Y.
      - For THT parts, it's easier to align the parts from the BOTTOM view, but remember that your X axis is now flipped, so a RIGHT nudge decreases the X and a LEFT nudge increases it.
      - Each "nudge" is a difference of 0.0635mm, and can be added or subtracted from the CPL file accordingly... i.e. if you nudge a part UP (increase y axis) five times, you're modifying it 5 x 0.0635mm or 3.175. So if it's value in the CPL file is -45.025, the new value should be -44.7075.
    - JLCPCB will automatically set the "confirm PCB" and "confirm placement" options, and let you know that there will be a small charge for this.
    - Add to cart, pay for items, and wait.
    - You will get at least two emails. The first will be to confirm your PCB file. This is done in your Order History window. Confirm it. The second email will be to confirm the layout of the components. In your Order History window, the lower Confirm Placement button brings a pop-up which shows the placement of the SMT parts (THT or throughhole is done separately). Everything should be fine, but reply to the email and ask to see a 3D of the cartridge port placement. This will take a day or so, but so long as the port is shown in the far right columns of pins, you can confirm placement.
    - Check the progress of your order daily to make sure there's nothing you need to confirm. Once the product has shipped, you can move onto Assembly.

#### **PCBWay** (PROCESS IN DEVELOPMENT)
1. **Order the assembled PCBs**
    - Visit the PCBWay Aquarius+ Shared Project page for the version you want to manufacture.
	    - [Aquarius+ Standard](https://www.pcbway.com/project/shareproject/Aquarius_Computer_Standard_PCB_0087c6b7.html) 
		- Aquarius+ Mini (coming soon)
	- Click on each of the following buttons to download the manufacturing files:
		- BOM(Bill of materials) - AQP_xxx_BOM_PCBWay_PCBWay Community.xlsx
		- Centroid file - AQP_xxx_CPL1_PCBWay_PCBWay Community.xlsx
		- The xxx in these filenames will be either STD or MINI, depending on the version.
	- Select **PCB+Assembly** and click **Add to Cart**
	- On the Order Page, in the PCB Specification Selection area, confirm the following selections (recommended):
		- Quantity (single): 5 pcs (this is the minimum order of PCBs, but they don't all have to be assembled, and you may have spares if not all are assembled)
		- Solder mask: Blue
		- Remove product No.: Yes (extra+$ 1.5)
		- All other defaults are fine
	- Click on the Assembly Service checkbox to expand/activate it, and confirm the following selections (recommended):
		- 3 flexible options: Turnkey
		- Quantity: 1 pcs (1 is the minimum assembled quantity; you can assemble up to 5 pcs without increasing the number of PCBs above)
		- Copy and paste the contents of the "Mfg_Assembly_Notes_PCBWay.txt" file into the Detailed information of assembly: field.
		- All other defaults are fine
	- Save to Cart
	- In the Cart, under the "Assembly for PCB" section, click on the blue Add File icon to add these files to the Assembly order (you downloaded them above:
		- Click the Parts List (BOM) Upload button and attach  AQP_xxx_BOM_PCBWay_PCBWay Community.xlsx
		- Click the Upload Centroid file and attach AQP_xxx_CPL1_PCBWay_PCBWay Community.xlsx
		- The xxx in these filenames will be either STD or MINI, depending on the version.
		- Click Submit Order now button.
	- You will wait up to a day or so for the files to be reviewed and approved, and a Quote issued.
	- Once you've been approved, return to your Cart and click Proceed to Checkout to pay the amount due
	- Over the next day or two, check your email and answer any questions they might have. 
2. **Wait for about 30 days** (seriously).
3. **Approve final assembly**
	- Reply to the email that will be sent for you to visually inspect/approve parts placement for the board(s), approving the continuation of manufacturing.
	- You will receive shipping confirmation after the boards are completed. Note that for larger orders, you MAY have to pay additional import taxes.
4. **Receive and verify PCBs**
	- You should visually inspect the PCBs once you get them. Make sure all parts are properly installed and soldered in place.
	- Continue onto the Assembly section below.

#### Assembling the Aquarius+
 - Make sure your shipment package has no obvious damage. If it does, take pictures before you open the box and save them for later. Most of the time, a ding or puncture will not be a problem, as the contents are typically very well wrapped.
 - There are sometimes small beads or blobs of solder that get deposited on the top of the board. With a magnifier, check ALL components to ensure there are no obvious solder blobs or bridges on the components, both SMT and THT. Now, check the BACK side of the board to ensure all THT components have solid, well-tented solder joints. Check the metal shrouds around the USB, controllers (DB9), VGA, power switch, and SD card socket to remove any accidental blobs or splashes. They should pop free with minimal force.
 - To assist in the SMT mounting process, manufacturers add additional "rails" to the PCB for handling. Using a small pair of needle-nose pliers, carefully remove these rails by bending along the V cut that has been cut into the PCB next to these rails. Be cautious not to damage any components that rest on this rail (LED, SD card socket, etc.). You can use a small file to remove any remaining burrs along these new edges. The rails can be thrown away.
 -  The wave solder that is used for the THT components leaves a sticky residue, as does the flux used for the SMT components. This residue should be cleaned using a two-step process, first with an alcohol-based solvent and a soft-bristle brush (an old toothbrush works great), and second with a mild de-greasing detergent, also agitated with a soft-bristle brush. Clean BOTH sides of the PCB thoroughly, first with the alcohol (I use left-over hand sanitizer from COVID), and then with the detergent (I use Krud Kutter), rinsing with water after each scrub down. Take care around delicate parts: the goal is to remove residue, not PCB components. Afterwards, either allow to air dry for 24 hours, or use a gentle, low-pressure compressed air source to drive the remaining water from all areas of the PCB (ports, power switch, under chips), again, taking care to not damage PCB components.
 - Once PCB has been cleaned and thoroughly dried, it's time to program the initial image for the Aquarius+. You're programming the ESP32 with the image that then gets sent to the Spartan 6 FPGA. From the GitHub, download the newest AquariusPlus-Firmware-xxx.zip bundle, decompress it, and follow the instructions for setting up the ESP software on your system. Using a USB to USB Micro cable and three 0.1" / 2.54mm pitch jumpers, connect the pins at JP1, JP2, and JP3, and plug the cable into the Aquarius+ power port and into your computer. Switch on the Aquarius+ power supply and note which COM port it appears on.
