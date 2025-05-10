/*
 * GE Custom Blade Controller : v4.2.1 : ruthsarian@gmail.com
 * A replacement blade controller for Galaxy's Edge lightsaber blades.
 * 
 * To compile and upload with the Arduino IDE, this project requires the
 * megaTinyCore Arduino core. For download and installation instructions
 * see https://github.com/ruthsarian/RuthsarianBladeController
 * 
 * -- Arduino IDE Board Configuration --
 *                    Board: ATtiny3226/3216/1626/1616/1606/826/816/806/426/416...
 *                     Chip: ATtiny1606
 *                    Clock: 10 MHz internal
 *  millis()/micros() Timer: Disabled
 *             Startup Time: 8ms
 *        BOD Voltage Level: 2.6V
 *                 BOD Mode: Sampled/Sampled (125Hz)
 *
 *  the rest of the settings you can leave at their default values
 *
 *
 * -- NOTICES --
 * 
 *   you MUST edit platform.txt located at:
 *     %LOCALAPPDATA%\Arduino15\packages\megaTinyCore\hardware\megaavr\<megaTinyCore version>\
 *   locate line that begins "compiler.c.elf.flags=" and remove "-mrelax"
 *   if you fail to do this, your blade will ignite, but the special features will not work
 *   
 *   newer versions of megaTinyCore have "{build.mrelax}" instead of "-mrelax". you could delete that 
 *   or edit boards.txt in the same directory, locate the line that begins "atxy6.build.mrelax"
 *   and delete "-mrelax"
 *   
 *   
 *   when programming with Arduino IDE, you need to first run the Burn Bootloader option under tools
 *   then perform a regular upload. the burn bootloader option only needs to be issued once, unless
 *   you change options that have "require burn bootloader" listed
 *   
 *
 *   there is a type in boards.txt for megaTinyCore earlier then at least 2.6.10.
 *   line 955 of boards.txt should look like this:
 *   
 *     atxy6.menu.bodmode.sampledslow.bootloader.bodmodebits=11010
 *     
 *   older megaTinyCore versions are missing the second 's' in sampledslow, which causes the
 *   sampled/sampled (125Hz) option to fail the burn bootloader operation. inserting the 's' 
 *   and restarting the IDE will fix the issue.
 * 
 */
