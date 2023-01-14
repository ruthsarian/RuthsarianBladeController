/*
 * Ruthsarian Blade Controller
 * ruthsarian@gmail.com
 * 
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
 *  millis()/micros() Timer: TCB0
 *             Startup Time: 8ms
 *        BOD Voltage Level: 2.6V
 *
 *  the rest of the settings you can leave at their default values
 *  
 * -- NOTICE --
 *   you MUST edit platform.txt located at:
 *     %LOCALAPPDATA%\Arduino15\packages\megaTinyCore\hardware\megaavr\<megaTinyCore version>\
 *   locate line that begins "compiler.c.elf.flags=" and remove "-mrelax"
 *   if you fail to do this, your blade will ignite, but the special features will not work
 *   
 * -- NOTICE UPDATE --
 *   newer versions of megaTinyCore have "{build.mrelax}" instead of "-mrelax". you could delete that 
 *   or edit boards.txt in the same directory, locate the line that begins "atxy6.build.mrelax"
 *   and delete "-mrelax"
 * 
 */
