<!-- https://docs.github.com/en/github/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax -->
# Ruthsarian Blade Controller
The Ruthsarian Blade Controller is a replacement blade controller for 
lightsaber blades from [Galaxy's Edge](https://en.wikipedia.org/wiki/Star_Wars:_Galaxy%27s_Edge). 
It replaces only [the blade controller](https://www.flickr.com/photos/ruthsarian/albums/72157719940747147), 
but reuses the rest of the blade's hardware including the strip of RGB LEDs.

This project was started after it was discovered by [Dead Bothans
Society](https://www.youtube.com/channel/UC8jRkAtfVRbUhOumBRoJw4g)
that Savi's Workshop lightsaber hilts [recognize orange and cyan kyber 
crystals](https://www.youtube.com/watch?v=fNlXbNlJkZo) and will command
the blade to ignite that color, but the blade will ignore the command. The
goal was to enable blades to support those commands. That goal has been
achieved, and more.

## State of the Project
This project comprises of two components, the PCB and the firmware.

The [PCB design](https://easyeda.com/ruthsarian/ruthsarian-blade-controller) 
is more or less finalized. There are multiple versions of the PCB that utilize 
different voltage regulators due to component availability, but the overall 
design is complete and any future versions of the PCB will work with this 
firmware.

The firmware is functional, but will likely see changes as features are added or 
altered to better suit the project as well including changes to optimize 
speed and program size.

## How To Use
The stock blade controller in a Galaxy's Edge lightsaber blade needs to
be [replaced](https://www.youtube.com/watch?v=M-DL8tl_s_o) with a
[Ruthsarian Blade Controller](https://easyeda.com/ruthsarian/ruthsarian-blade-controller). 
The new blade controller is programmed with this firmware. 

## The Microcontroller
The project is centered on an [ATtiny806/1606 microcontroller](https://ww1.microchip.com/downloads/en/DeviceDoc/ATtiny806_1606_Data_Sheet_40002029A.pdf). 
The difference between the 1606 and 806 is the 1606 has 16kb of programminmg 
space, while the 806 has only 8kb. The 1606 is the preferred choice as some 
functionality may need to be disabled in order to get this firmware to fit on
an 806. However either one will work for this project.

## Compiling the Firmware
This code can be compiled using the [Arduino IDE](https://www.arduino.cc/en/software)
as long as you've installed [megaTinyCore](https://github.com/SpenceKonde/megaTinyCore). 
This code could also be compiled with [Microchip Studio](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio).
There may be other environments that this code will compiled, but Arduino IDE 
and Microchip Studio are verified to work.

## IDE Setup Tips
### Microchip Studio
- Project -\> Properties (ALT+F7) 
	- -\> Toolchain -\> Configuration: All Configurations 
	- -\> Toolchain -\> AVR/GNU C Compiler -\> Symbols -\> Defined Symbols, ADD: F_CPU=10000000UL
	- -\> Build Events -\> Post-build event command line -\> "$(ToolchainDir)"\avr-objcopy.exe -O ihex -j .fuse --change-section-lma .fuse=0 "$(OutputFileName).elf" "$(OutputFileName)_fuses.hex"
	- -\> Toolchain -\> AVR/GNU C Compiler -\> Miscellaneous -\> Other Flags -\> add "-flto"
	- -\> Toolchain -\> AVR/GNU C Link -\> Miscellaneous -\> Other Flags -\> add "-mrelax"

### Arduino IDE
- Install megaTinyCore (https://github.com/SpenceKonde/megaTinyCore)
- Tools -\> Board: ATtiny1606, Chip: ATtiny1606, Clock: 10MHz (internal), millis()/micros() Timer: TCB0
- Edit megaTinyCore platform.txt located at %LOCALAPPDATA%\Arduino15\packages\megaTinyCore\hardware\megaavr\\*\<version\>*\platform.txt
	- locate line that begins "compiler.c.elf.flags=" and remove "-mrelax"

## Programming the Blade Controller
The ATtiny806/1606 uses the UPDI programming interface/protocol to program
the microcontroller.

There are several options for programming the board. I have been using 
[jtag2updi](https://github.com/SpenceKonde/jtag2updi) installed on an Arduino
Nano for programming, however other solutions are certainly available. See 
[megaTinyCore](https://github.com/SpenceKonde/AVR-Guidance/blob/master/UPDI/jtag2updi.md) 
for options instructions on how to program the microcontroller. 

There is a 2x3, 1.27mm pitch programming header on the PCB. This header can be 
used to program the ATtiny806/1606 microcontroller. The pinout of this header 
is described on the bottom of the PCB. While there are 6 pins in the header, 
only 3 are needed to program the microcontroller: VCMU (power), UPDI, and GND.

I use a 2x3, 1.27mm pitch pogo pin adapter to interface with that header. The
[particular one I use](https://www.tindie.com/products/electronutlabs/pogoprog-model-d-pogo-pin-programmer-2-pack/)
is not currently in stock, however alternatives may be found elsewhere. It's
possible you could even [build your own](https://github.com/Pnoxi/AVR-ISP-Pogo-Pin-Adapter). 

## Firmware (Blade Controller) Operation
The firmware provides the ability to apply different colors and animation 
effects to the blade. There is a blade display mode (referred to as 'DMODE'
in the code) and an animation mode (referred to as 'DSUBMODE' in the code).

Changing modes is controlled by powering off the blade and then turning it 
back on. The time the blade is off determines how the blade changes (if at 
all). If the blade is off for less than the number of milliseconds
specified by DMODE_THRESHOLD_TIME in the code (1000) then the DMODE changes.
If the blade is off or more than DMODE_THRESHOLD_TIME, but less than
DSUBMODE_THRESHOLD_TIME (3000) the animation mode changes. If the blade is
off for longer than DSUBMODE_THRESHOLD_TIME, nothing changes.

As this project is ongoing, a detailed list of different display modes
and animation effects will not be provided here. You can either look through
the code or experiment.

### It Remembers
If a blade is left off for longer than OFF_TO_SLEEP_TIME (10 seconds) while 
still plugged into the hilt, the display and animation settings that were set
when the blade powered off will be stored in the blade. These settings will be 
used the next time the blade is inserted into a hilt.

### Reset The Blade Controller
If, for any reason, you wish to simply reset the blade controller to it's 
stock functionality, power the blade off and on again several times very
quickly. The hilt will eventually make a noise as if the blade has been
removed. This indicates the blade is in reset. When the hilt makes the blade
insertion noise, the reset is complete.

### Optional Switches
These switches are *optional* and are not required for the blade to operate.

The PCB includes a 3-position header which can be used to attach two switches.
The pins on this header are marked as B0, B1, and GND. B1 and B0 should
connect to one end of the switch and GND to the other end of both switches.
The switch is considered 'on' when B1 or B0 is connected to GND.

B0 enables write protect. When enabled, the blade's stored settings will 
not be overwritten when the blade is left off for longer than OFF_TO_SLEEP_TIME
(10 seconds). 

B1 disables the custom display modes. When enabled, the blade behaves like a
stock blade would, with the exception that the blade will ignite orange and
cyan if an appropriate crystal is inserted into a Savi's Workshop hilt.

If both B0 and B1 are enabled, the blade is locked to whatever custom
display mode is saved to the blade. This is useful if you want the blade
to always be a certain color or have a certain animation effect and you
do not want to ever change it (until you disable one of the two switches).

A [2-position DIP 
switch](https://www.ebay.com/sch/i.html?_nkw=2+position+dip+switch&_sacat=0) 
glued to the end of the inner blade connector housing with a hole drilled 
through the outer blade connector housing to provide access to the switches is 
one possible approach, but not the only one. See [these 
images](https://twitter.com/ruthsarian/status/1479264695085326337/photo/1)
for an example of this setup.

## The Replacement Blade Controller PCB
The PCB for the replacement blade controller may be [ordered through 
OSHPark](https://oshpark.com/shared_projects/asa59Q8P). The PCB design 
files are [available through EasyEDA](https://easyeda.com/ruthsarian/ruthsarian-blade-controller) 
if you wish to make your own changes or modifications to the PCB.

### PCB Versions
There are currently two different versions of the PCB: [an 'H' version](https://oshpark.com/shared_projects/NbIljZq0)
and [an 'L' version](https://oshpark.com/shared_projects/asa59Q8P).
The two versions are the same in every way except in which
voltage regulator is used to drive the LEDs. The 'H' version was developed first,
however limited availability of ['H' version's voltage regulator](https://www.mouser.com/ProductDetail/511-LD39200PU33R)
necessitated a modification to support [a different voltage regulator](https://www.mouser.com/ProductDetail/511-LDL112PV33R), 
thus the 'L' version. Unfortunately both regulators (as of February 2022) are 
now on backorder. 

### [PCB](https://en.wikipedia.org/wiki/Printed_circuit_board) [BOM](https://en.wikipedia.org/wiki/Bill_of_materials)
The table below contains a list of all the components needed to assemble the 
PCB. The rows with gray backgrounds contain example components that meet the 
criteria, but other options may be available.

Some components may be on backorder for several months. U2 and U3 in particular 
may be difficult to obtain. Alternative LDOs may be available, but they may not 
be pin-compatible. A partial redesign of the PCB to fit an available LDO may be 
a solution to this problem. Also note that different LDOs may have different
input and output capacitor requirements. Consult the datasheet and adjust this
BOM as necessary.

|         Designator | Component |
| -----------------: | :-------- |
|             **U1** | ATtiny1606, VQFN20 package |
|                    | [ATTINY1606-MF](https://www.mouser.com/ProductDetail/556-ATTINY1606-MF) |
|             **U2** | 3.3V LDO, SOT23 package, <=1uA quiescent current, PINS: 1-GND, 2-OUT, 3-IN |
|                    | [AP2138N-3.3TRG1](https://www.mouser.com/ProductDetail/621-AP2138N-3.3TRG1), [XC6206P331MR-G](https://www.mouser.com/ProductDetail/865-XC6206P331MR-G), [XC6206P332MR-G](https://www.mouser.com/ProductDetail/865-XC6206P332MR-G) |
|             **U3** | 3.3V LDO, DFN-6 package, >1A, PINS: 1-EN, 2-GND, 3-NC, 4-OUT, 5-NC, 6-IN |
|                    | [LDL112PV33R](https://www.mouser.com/ProductDetail/511-LDL112PV33R) |
|             **L1** | >=3.3uH, 0805 package, >=1A |
|                    | [LQM21PN3R3NGRD](https://www.mouser.com/ProductDetail/81-LQM21PN3R3NGRDRD), [LQM21PN3R3MGRD](https://www.mouser.com/ProductDetail/81-LQM21PN3R3MGRD) |
|        **Q1 - Q4** | PNP, SOT23 package, PINS: 1-B, 2-E, 3-C |
|                    | [MMSS8550](https://www.mouser.com/ProductDetail/833-MMSS8550-L-TP), [ZXTP2041FTA](https://www.mouser.com/ProductDetail/621-ZXTP2041FTA), [PMMT591A](https://www.mouser.com/ProductDetail/771-PMMT591AT%2FR) |
|        **Q5 - Q7** | Nch MOSFET, SOT23 package, >=1A, PINS: 1-G, 2-S, 3-D |
|                    | [BSH105](https://www.mouser.com/ProductDetail/771-BSH105215) |
|        **C1, C7**  | 330uf electrolytic, 2.5mm lead spacing, >=6.3V |
|                    | [6.3ZLG330MEFC6.3X11](https://www.mouser.com/ProductDetail/232-63ZLG330MEFC63X1) |
| **C2, C3, C5, C6** | 1uf ceramic, 0402 package, >=6.3V |
|                    | [GRM155R61C105KA12D](https://www.mouser.com/ProductDetail/81-GRM155R61C105KA2D) |
|             **C4** | 0.1uf ceramic, 0402 package, >=6.3V |
|                    | [GCM155L8EH104KE07D](https://www.mouser.com/ProductDetail/81-GCM155L8EH104KE7D) |
|        **R1 - R7** | 1kOhm, 0402 package |
|                    | [CR0402-FX-1001GLF](https://www.mouser.com/ProductDetail/652-CR0402FX-1001GLF) |
|             **R8** | 10kOhm, 0402 package |
|                    | [CR0402-JW-103GLF](https://www.mouser.com/ProductDetail/652-CR0402-JW-103GLF) |

## About Blade LEDs
Galaxy's Edge lightsaber blades contain common-anode RGB LEDs. Each LED has
4 pins, one pin is the common anode, the other three pins are the cathode 
of a color LED (red, green, blue). Sending power into the common anode and
then connecting the cathode of a color to ground will illuminate that color.

The common-anodes of the blade LEDs are tied together in 4 separate groups
which are referred to as 'segments' in the code. Power to each group of
common-anodes is controlled via a PNP transistor on the blade controller. 
Ignition and extinguish animations are achieved by turning on each segment's
control transistor in order.

The cathodes of each color are all tied together and controlled via an
N-channel MOSFETs on the controller. As an example of operation, to ignite
the blade blue, the FET that controls the blue channel is turned on, 
connecting all the cathodes of all blue LEDs to ground. Then the first
segment's common-anode PNP transistor is turned on which sends power to the
common-anode pin of all the RGB LEDs in the first segment of the blade. The
first segment of the blade illuminates blue. After a brief delay, the second
segment's common-anode transistor is turned on, then the third, then the last.
This creates the effect of the blade igniting from the base to the tip of
the blade.

To achieve the effect of each segment having its own color, one segment at a
time must be powered on, the color for that segment set, then after a brief 
delay, that segment is turned off and the next segment is turned on with the
appropriate color for that segment. This technique is referred to as multiplexing. 
Perform these steps fast enough and, to the human eye, it looks like the entire 
blade is ignited with different colors on each segment.

## License
The code is presented as-is with no guarantee or warranty. Use at your own risk.

Unless otherwise noted, code within this repository is released into the public 
domain.
