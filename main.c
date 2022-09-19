/* GE Custom Blade Controller : v4.0 : ruthsarian@gmail.com
 *
 * This is firmware for an ATTiny806/1606 based replacement blade controller
 * for a Galaxy's Edge lightsaber blade.
 *
 * LICENSE: All code, unless otherwise noted, is released into the public
 *          domain.
 */

#include <stdlib.h>
#include <stdio.h>
#include "device_config.h"
#include "blade_state.h"
#include "data.h"
#include "dmode_handler.h"

// program setup
void setup() {
	device_setup();		// configure the device
}

// main program loop
void loop() {
	if (reset_handler() == 0) {	// avoid sleep and data handlers while in reset
		sleep_handler();		// put the blade to sleep if it's been off for X number of seconds
		data_handler();			// read data from DATA_PIN
	}
	command_handler();			// process command data

	// only call animate and dmode handlers if blade is not off
	if ((blade.state & 0xF0) != BLADE_STATE_OFF) {
		animate_handler();		// manipulate blade colors and segment brightness based on blade state

		// call dmode_handler unless ONLY the DMODE disable switch it set		
		if (switch_config ^ (1<<SW_DMODE_DISABLE_bp)) {
			dmode_handler();	// adds custom colors and effects to the blade
		}

		// calculate the true brightness of each segment
		//
		// this is done after both animate_handler() and dmode_handler() have had their
		// chance to manipulate segment brightness.
		//
		// it's a lot of effort, but doing this allows animation effects to persist through
		// ignition and extinguish, making the effects much cleaner.
		true_segment_brightness_handler();
	}
}

// main program
int main(void)
{
	setup();		// program setup
	while (1) {		// main program loop
		loop();
	}
}
