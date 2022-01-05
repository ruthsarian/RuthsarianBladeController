<!-- https://docs.github.com/en/github/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax -->
# Ruthsarian Blade Controller
Firmware for a replacement Galaxy's Edge lightsaber blade controller.

## State of the Code
It works.

## About
This is firmware for an ATtiny806/1606 based replacement blade controller 
for a Galaxy's Edge lightsaber blade. This project was conceived soon after
it was discovered that Savi's Workshop lightsaber hilts can recognize orange
and cyan kyber crystals, but the blades do not. The original goal was to
add support for those colors to the blade. Once that was achieved, the goal
expanded to add more features.

## How To Use
You'll need to [replace the blade controller in a Galaxy's Edge lightsaber blade](https://www.youtube.com/watch?v=M-DL8tl_s_o) 
with a [replacement controller](https://oshpark.com/shared_projects/OngJWKF3). 

This code can be compiled using the [Arduino IDE](https://www.arduino.cc/en/software)
as long as you've installed [megaTinyCore](https://github.com/SpenceKonde/megaTinyCore). 
This code could also be compiled with 
[Microchip Studio](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio).

See [megaTinyCore](https://github.com/SpenceKonde/megaTinyCore) for options 
([some old](https://github.com/SpenceKonde/AVR-Guidance/blob/master/UPDI/jtag2updi.md)) 
on how to program the microcontroller. There is a 2x3, 1.27mm pitch programming header
built into the [replacement controller PCB](https://oshpark.com/shared_projects/OngJWKF3)
for this purpose. The header can be accessed using an adapter such as 
[this pogo pin based adapter](https://www.tindie.com/products/electronutlabs/pogoprog-model-d-pogo-pin-programmer-2-pack/).

## IDE Setup Tips
### Microchip Studio
- Project -> Properties (ALT+F7) 
	- -> Toolchain -> Configuration: All Configurations 
	- -> Toolchain -> AVR/GNU C Compiler -> Symbols -> Defined Symbols, ADD: F_CPU=10000000UL
	- -> Build Events -> Post-build event command line -> "$(ToolchainDir)"\avr-objcopy.exe -O ihex -j .fuse --change-section-lma .fuse=0 "$(OutputFileName).elf" "$(OutputFileName)_fuses.hex"
	- -> Toolchain -> AVR/GNU C Compiler -> Miscellaneous -> Other Flags -> add "-flto"
	- -> Toolchain -> AVR/GNU C Link -> Miscellaneous -> Other Flags -> add "-mrelax"

### Arduino IDE
- Install megaTinyCore (https://github.com/SpenceKonde/megaTinyCore)
- Tools -> Board: ATtiny1606, Chip: ATtiny1606, Clock: 10MHz (internal), millis()/micros() Timer: TCB0
- Edit megaTinyCore platform.txt located at %LOCALAPPDATA%\Arduino15\packages\megaTinyCore\hardware\megaavr\\*\<version\>*\platform.txt
	- locate line that begins "compiler.c.elf.flags=" and remove "-mrelax"

## License
The code is presented as-is with no guarantee or warranty. Use at your own risk.

Unless otherwise noted, code within this repository is released into the public 
domain.