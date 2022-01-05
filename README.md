<!-- https://docs.github.com/en/github/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax -->
# Ruthsarian Blade Controller
Firmware for a replacement Galaxy's Edge lightsaber blade controller.

## State of the Code
It works. But probaly not useful to most people until the PCB design is released
and components become available.

## About
This is firmware for an ATtiny806/1606 based replacement blade controller 
for a Galaxy's Edge lightsaber blade. This project was conceived soon after
it was discovered that Savi's Workshop lightsaber hilts can recognize orange
and cyan kyber crystals, but the blades do not. The original goal was to
add support for those colors to the blade. Once that was achieved, the goal
expanded to add more features.

## How To Use
You'll need to [replace the blade controller in a Galaxy's Edge lightsaber blade with 
a replacement controller](https://www.youtube.com/watch?v=M-DL8tl_s_o). 

This code can be compiled using the [Arduino IDE](https://www.arduino.cc/en/software)
as long as you've installed [megaTinyCore](https://github.com/SpenceKonde/megaTinyCore). 
This code could also be compiled with 
[Microchip Studio](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio).

See [megaTinyCore](https://github.com/SpenceKonde/megaTinyCore) for options 
([some old](https://github.com/SpenceKonde/AVR-Guidance/blob/master/UPDI/jtag2updi.md)) 
on how to program the microcontroller. 

## License
The code is presented as-is with no guarantee or warranty. Use at your own risk.

Unless otherwise noted, code within this repository is released into the public 
domain.