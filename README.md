# Ruthsarian Blade Controller
The Ruthsarian Blade Controller is a replacement blade controller for [lightsaber blades](https://www.shopdisney.com/galaxys-edge-lightsaber-blade-36-star-wars-galaxys-edge-400021020917.html) from [Galaxy's Edge](https://en.wikipedia.org/wiki/Star_Wars:_Galaxy%27s_Edge). It replaces [the blade controller](https://www.flickr.com/photos/ruthsarian/albums/72157719940747147) and reuses the rest of the blade's hardware including the strip of RGB LEDs.

To make full use of this project you will need to assemble a blade controller, program it, then install it into a Galaxy's Edge lighstaber blade. Documentation on assembly and programming are contained within this README. [This YouTube playlist](https://www.youtube.com/playlist?list=PL1LWtA1iy9hvn_TC9zpnyU5pupyrw3pX3) contains videos documenting the development, assembly, and installation process of this PCB. A more in-depth series of videos on assembly, programming, and installation are forthcoming.

This project was started after it was discovered by [Dead Bothans Society](https://www.youtube.com/channel/UC8jRkAtfVRbUhOumBRoJw4g) that Savi's Workshop lightsaber hilts [recognize orange and cyan kyber crystals](https://www.youtube.com/watch?v=fNlXbNlJkZo) and will command the blade to ignite that color, but the blade will ignore the command. The goal was to enable blades to support those commands. That goal has been achieved, and more.

## Blade Controller Operation
This blade controller provides the ability to apply different colors and animation effects to the blade. There is a blade display mode (referred to as 'DMODE' in the code) and a blade animation mode (referred to as 'DSUBMODE' in the code).

Changing modes is controlled by powering off the blade and then turning it back on. The time the blade is off determines how the blade changes (if at all). If the blade is off for less than 1 second (defined by DMODE_THRESHOLD_TIME in the code) then the display mode changes.

If the blade is off for more than 1 second (DMODE_THRESHOLD_TIME), but less than 3 seconds (defined by DSUBMODE_THRESHOLD_TIME in the code) the animation mode changes. If the blade is off for longer than 3 seconds (DSUBMODE_THRESHOLD_TIME), nothing changes.

### Display Modes

|        Display Mode | Effect |
| ------------------: | :----- |
| Stock               | The default display mode. The blade behaves as a stock blade would. |
| Color Picker        | The blade cycles through different colors. The number of colors available to choose from can be increased by triggering an animation mode change (off for more than 1, but less than 3 seconds) while in color picker mode. The brightness of the color may also be adjusted by triggering a clash (move the hilt in some direction, then quickly stop). |
| Color Picker Picked | The blade behaves the same as in stock mode, except with the color that has been picked rather than the color the hilt commanded the blade to ignite. |
| Blade Wheel         | The blade cycles through all the colors of the color wheel. The speed of the cycle can be altered by triggering an animation mode change. |
| Segmented Wheel     | Each segment of the blade cycles through the color wheel, but each segment is slightly offset from the others, producing a color gradient down length of the blade. The speed of the cycle can be altered by triggering an animation mode change. |
| Multi-Segment Mode  | The blade displays a different color at each segment of the blade. The colors are hard-coded into the firmware. Triggering an animation mode change will cause the blade to cycle to the next color pattern. |

### Animation Modes
Animation modes only apply to the stock and color picker picked modes.

|     Animation Mode | Effect |
| -----------------: | :----- |
| Full Blade Flicker | The blade flickers randomly. |
| Segmented Flicker  | Each segment flickers randomly and independently of the other segments. |
| Bright Pulse       | Bright pulses flow down the length of the blade at random intervals. |
| Dark Pulse         | Dark pulses flow down the length of the blade at random intervals. |
| Gradient Pulse     | The base of the blade pulses at random. The pulse degrades as it flows down the length of the blade. |
| Breathing          | The blade fades in and out as if it is breathing. |
| Static Gradient 1  | The brightness of each segment decreases the further away it is from the base of the blade. |
| Static Gradient 2  | The brightness of each segment decreases the further away it is from the base of the blade. |
| 66% Brightness     | The blade is set to 66% of its normal brightness. |
| 33% Brightness     | The blade is set to 33% of its normal brightness. |
| 10% Brightness     | The blade is set to 10% of its normal brightness. |

Note: the brightness modes may be removed in the future as the color picker's ability to set brightness make make these modes redundant.

### It Remembers
If a blade is left off for longer than 10 seconds (defined by OFF_TO_SLEEP_TIME in the code) while still plugged into the hilt, the display and animation settings that were set when the blade powered off will be stored in the blade. These settings will be used the next time the blade is inserted into a hilt.

### Reset The Blade Controller
If, for any reason, you wish to simply reset the blade controller to it's stock functionality, power the blade off and on again several times very quickly. The hilt will eventually make a noise as if the blade has been removed. This indicates the blade is in reset. When the hilt makes the blade insertion noise, the reset is complete.

### Optional Switches
These switches are *optional* and are not required for the blade to operate.

The PCB includes a 3-position header which can be used to attach two switches. The pins on this header are marked as B0, B1, and GND. B1 and B0 should connect to one end of the switch and GND to the other end of both switches. The switch is considered 'on' when B1 or B0 is connected to GND.

|  Switch | Effect               | Description |
| ------: | :------------------- | :---------- |
| B0      | Write Protect        | When enabled the blade's stored settings will not be overwritten when the blade is left off for longer than 10 seconds (OFF_TO_SLEEP_TIME). |
| B1      | Force Stock Behavior | When enabled the blade behaves as a stock blade would with the exception that the blade will ignite orange and cyan if an appropriate crystal is inserted into a Savi's Workshop hilt. |
| B0 & B1 | Lock Blade Settings  | If both switches are enabled the blade is locked to whatever display and animation modes are stored in the blade. This is useful if you want the blade to always be a certain color or have a certain animation effect and you do not want to ever change it (until you disable one of the two switches). |

#### Recommended Switch Hardware
A [2-position DIP switch](https://www.ebay.com/sch/i.html?_nkw=2+position+dip+switch&_sacat=0) glued to the end of the inner blade connector housing with a hole drilled through the outer blade connector housing to provide access to the switches is one possible approach, but not the only one. See [these  images](https://twitter.com/ruthsarian/status/1479264695085326337/photo/1) for an example of this setup.

## Blade Controller PCB
The PCB for the replacement blade controller may be [ordered through OSHPark](https://oshpark.com/shared_projects/UBNsoFjS). The PCB design files are [available through EasyEDA](https://easyeda.com/ruthsarian/ruthsarian-blade-controller) if you wish to make your own changes or modifications to the PCB.

### PCB Versions
PCBs are versioned by a number followed by a letter. The number will indicate the PCB revision while the letter indicates the voltage regulator used to drive blade's LEDs.

There are currently two different letter versions: [an 'H' version](https://oshpark.com/shared_projects/NbIljZq0) and [an 'L' version](https://oshpark.com/shared_projects/UBNsoFjS). 
The 'H' version was developed first, however limited availability of ['H' version's voltage regulator](https://www.mouser.com/ProductDetail/511-LD39200PU33R) necessitated a modification to support [a different voltage regulator](https://www.mouser.com/ProductDetail/511-LDL112PV33R), thus the 'L' version. Unfortunately both regulators (as of October 2022) are on backorder. There are currently no plans to add a third letter version of this PCB.

The current major version of this PCB is 4. There are a few different minor versions (4.0, 4.1, 4.4). All minor versions are functionally identical and the differences between each only relate to resolving issues with the solder paste layer that would have a small impact on board assembly. 

### [PCB](https://en.wikipedia.org/wiki/Printed_circuit_board) [BOM](https://en.wikipedia.org/wiki/Bill_of_materials)
The table below contains a list of all the components needed to assemble the PCB. The rows with gray backgrounds contain example components that meet the criteria. The linked components be on backorder or no longer available. However alternatives will likely be available. For example, R8 is a 10k resistor in an 0402 package. Searching for other 10k resistors in an 0402 package will yield dozens of available options, any of which would work. You are not limited to just the components linked to in the BOM so long as they meet the criteria given for that component.

|         Designator | Component |
| -----------------: | :-------- |
|             **U1** | ATtiny1606, VQFN20 package |
|                    | [ATTINY1606-MF](https://www.mouser.com/ProductDetail/556-ATTINY1606-MF), [ATTINY1606-MN](https://www.mouser.com/ProductDetail/556-ATTINY1606-MN), [ATTINY1606-MFR](https://www.mouser.com/ProductDetail/556-ATTINY1606-MFR), [ATTINY1606-MNR](https://www.mouser.com/ProductDetail/556-ATTINY1606-MNR) |
|             **U2** | voltage regulator, 3.3V, SOT23 package quiescent current <= 1uA  <br>pinout: 1-GND, 2-OUT, 3-IN |
|                    | [AP2138N-3.3TRG1](https://www.mouser.com/ProductDetail/621-AP2138N-3.3TRG1), [XC6206P331MR-G](https://www.mouser.com/ProductDetail/865-XC6206P331MR-G), [XC6206P332MR-G](https://www.mouser.com/ProductDetail/865-XC6206P332MR-G) |
|             **U3** | voltage regulator, 3.3V, DFN-6 package, current rated > 1A  <br>'L' version pinout: 1-EN, 2-GND, 3-NC, 4-OUT, 5-NC, 6-IN <br> 'H' version pinout: 1-OUT, 2-SENSE, 3-GND, 4-PG, 5-EN, 6-IN |
|                    | 'L' Version: [LDL112PV33R](https://www.mouser.com/ProductDetail/511-LDL112PV33R)  <br>'H' Version: [LD39200PU33R](https://www.mouser.com/ProductDetail/511-LD39200PU33R)|
|             **L1** | inductor, >= 3.3uH, 0805 package, current rated >= 1A |
|                    | [LQM21PN3R3NGRD](https://www.mouser.com/ProductDetail/81-LQM21PN3R3NGRDRD), [LQM21PN3R3MGRD](https://www.mouser.com/ProductDetail/81-LQM21PN3R3MGRD) |
|        **Q1 - Q4** | PNP transistor, SOT23 package, pinout: 1-B, 2-E, 3-C |
|                    | [MMSS8550](https://www.mouser.com/ProductDetail/833-MMSS8550-L-TP), [ZXTP2041FTA](https://www.mouser.com/ProductDetail/621-ZXTP2041FTA), [PMMT591A](https://www.mouser.com/ProductDetail/771-PMMT591AT%2FR) |
|        **Q5 - Q7** | Nch MOSFET, SOT23 package, >=1A, pinout: 1-G, 2-S, 3-D |
|                    | [SI2302](https://www.mouser.com/ProductDetail/%2078-SI2302CDS-T1-BE3), [NXV55UNR](https://www.mouser.com/ProductDetail/771-NXV55UNR), [BSH105](https://www.mouser.com/ProductDetail/771-BSH105215) |
|         **C1, C7** | capacitor, 330uf electrolytic, 2.5mm lead spacing, >=6.3V |
|                    | [10ZLH330MEFC6.3X11](https://www.mouser.com/ProductDetail/232-10ZLH330MEFC63X1) |
|         **C2, C3** | capacitor, 1uf ceramic, 0402 package, >=6.3V |
|                    | [GRM155R61C105KA2D](https://www.mouser.com/ProductDetail/81-GRM155R61C105KA2D) |
|         **C5, C6** | capacitor, 'L':1uf or 'H':10uf ceramic, 0402 package, >=6.3V |
|                    | 'L' Version: [GRM155R61C105KA2D](https://www.mouser.com/ProductDetail/81-GRM155R61C105KA2D)  <br> 'H' Version: [GRJ155R60J106ME1D](https://www.mouser.com/ProductDetail/81-GRJ155R60J106ME1D)|
|             **C4** | capacitor,0.1uf ceramic, 0402 package, >=6.3V |
|                    | [GCM155L8EH104KE07D](https://www.mouser.com/ProductDetail/81-GRM155R71C104KA88) |
|        **R1 - R7** | resistor, 1kOhm, 0402 package |
|                    | [CR0402-JW-102GLF](https://www.mouser.com/ProductDetail/652-CR0402JW-102GLF) |
|             **R8** | resistor, 10kOhm, 0402 package |
|                    | [CR0402-JW-103GLF](https://www.mouser.com/ProductDetail/652-CR0402-JW-103GLF) |

## Blade Controller Firmware
The project is centered on an [ATtiny1606 microcontroller](https://ww1.microchip.com/downloads/en/DeviceDoc/ATtiny806_1606_Data_Sheet_40002029A.pdf) microcontroller in a VQFN-20 package. Other ATtiny microcontrollers that share the same footprint and pinout, such as the 806, may be used however there may be program space limitations that you will need to address. For example, the 806 has 8kb of program space. Currently this firmware is larger than 8k, therefore some features would need to be disabled in order to get it to fit on an 806. 

### Compiling the Firmware
This code can be compiled using the [Arduino IDE](https://www.arduino.cc/en/software) as long as you've installed [megaTinyCore](https://github.com/SpenceKonde/megaTinyCore). This code could also be compiled with [Microchip Studio](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio). There may be other environments that this code will compiled, but Arduino IDE and Microchip Studio are verified to work *with slight modifications* as noted in the IDE Setup section below.

### IDE Setup
Below are required steps to configure your IDE environment for compiling this firmware. Not following these steps may result in the firmware not compiling or behaving unexpectedly.

#### Arduino IDE
- Install megaTinyCore (https://github.com/SpenceKonde/megaTinyCore)
- Tools -\> Board: ATtiny1606, Chip: ATtiny1606, Clock: 10MHz (internal), millis()/micros() Timer: TCB0
- Locate platform.txt within the megaTinyCore installation.
	- Windows: %LOCALAPPDATA%\Arduino15\packages\megaTinyCore\hardware\megaavr\\*\<version\>*\platform.txt
- Edit platform.txt as follows:
	- locate line that begins "compiler.c.elf.flags=" and remove "-mrelax" 
	- if you see "{build.mrelax}" instead of "-mrelax" you have a newer version of megaTinyCore
- For newer versions of megaTinyCore locate boards.txt in the same directory
	- locate the line that beings with "atxy6.build.mrelax" and delete "-mrelax"

#### Microchip Studio
- Project -\> Properties (ALT+F7) 
	- -\> Toolchain -\> Configuration: All Configurations 
	- -\> Toolchain -\> AVR/GNU C Compiler -\> Symbols -\> Defined Symbols, ADD: F_CPU=10000000UL
	- -\> Build Events -\> Post-build event command line -\> "<span>$</span>(ToolchainDir)"\avr-objcopy.exe -O ihex -j .fuse --change-section-lma .fuse=0 "<span>$</span>(OutputFileName).elf" "<span>$</span>(OutputFileName)\_fuses.hex"
	- -\> Toolchain -\> AVR/GNU C Compiler -\> Miscellaneous -\> Other Flags -\> add "-flto"
	- -\> Toolchain -\> AVR/GNU C Link -\> Miscellaneous -\> Other Flags -\> add "-mrelax"

### Programming the Blade Controller
The ATtiny1606 uses the UPDI programming interface/protocol to program the microcontroller. [megaTinyCore documentation](https://github.com/SpenceKonde/megaTinyCore#UPDI-Programming) covers UPDI programming and recommends using SerialUPDI which is bundled with megaTinyCore. This requires a USB-to-Serial device and creating a cable with
a diode and resistor to connect it to your UPDI-programmable device. See [this document](https://github.com/SpenceKonde/AVR-Guidance/blob/master/UPDI/jtag2updi.md#Wiring-the-hardware) 
for further information on wiring up a SerialUPDI adapter.

Another, now deprecated, but still function option is installing [jtag2updi](https://github.com/SpenceKonde/jtag2updi) an Arduino device (I use a Nano). Digital pin 6 of the Arduino device is then connected to the UPDI pin of the device to be programmed. 

Both options given above have been tested and work. There may be more options available, these are just two that are known to work.

#### Programming Header

Whichever programming option you choose will need to interface with the blade controller PCB via a 2x3, 1.27mm pitch programming header. While there are 6 pins in the header, only 3 are needed to program the microcontroller: VCMU (power), UPDI, and GND. The pinout of this header is described on the bottom of the PCB. 

I recommend using a POGO pin adapter to connect your programmer to the programming header on the PCB. You could (temporarily) solder wires to blade controller PCB's programming header instead of using a POGO pin adapter, however I find the POGO pin adapter to be an easier solution.

#### POGO Pin Programming Adapter
I use a 2x3, 1.27mm pitch POGO pin adapter to interface with that header. The [particular one I use](https://www.tindie.com/products/electronutlabs/pogoprog-model-d-pogo-pin-programmer-2-pack/) is not currently in stock, however alternatives may be found elsewhere. It's
possible you could even [build your own](https://github.com/Pnoxi/AVR-ISP-Pogo-Pin-Adapter). 

[I have designed this PCB](https://oshpark.com/shared_projects/KFOWave7) as another alternative. The board may be ordered through OSH Park or the Gerber files may be downloaded via the ACTIONS menu on the OSH Park project page and you can order it elsewhere. The project description on OSH Park includes what type of POGO pins and header pins you'll need to order (widely available on eBay or Amazon) as well as tips on how to assemble it.

## Miscellaneous Documentation

### About Blade LEDs
Galaxy's Edge lightsaber blades contain common-anode RGB LEDs. Each LED has 4 pins, one pin is the common anode, the other three pins are the cathode of a color LED (red, green, blue). Sending power into the common anode and then connecting the cathode of a color to ground will illuminate that color.

The common-anodes of the blade LEDs are tied together in 4 separate groups which are referred to as 'segments' in the code. Power to each group of common-anodes is controlled via a PNP transistor on the blade controller. Ignition and extinguish animations are achieved by turning on each segment's control transistor in order.

The cathodes of each color are all tied together and controlled via an N-channel MOSFETs on the controller. As an example of operation, to ignite the blade blue, the FET that controls the blue channel is turned on, connecting all the cathodes of all blue LEDs to ground. Then the first segment's common-anode PNP transistor is turned on which sends power to the common-anode pin of all the RGB LEDs in the first segment of the blade. The first segment of the blade illuminates blue. After a brief delay, the second segment's common-anode transistor is turned on, then the third, then the last. This creates the effect of the blade igniting from the base to the tip of the blade.

To achieve the effect of each segment having its own color, one segment at a time must be powered on, the color for that segment set, then after a brief delay, that segment is turned off and the next segment is turned on with the appropriate color for that segment. This technique is referred to as multiplexing. Perform these steps fast enough and, to the human eye, it looks like the entire blade is ignited with different colors on each segment.

## License
The code is presented as-is with no guarantee or warranty. Use at your own risk. Unless otherwise noted, code within this repository is released into the public domain.
