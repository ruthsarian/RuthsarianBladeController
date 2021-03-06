/* dmode_handler.c
 *
 */ 

#include <stdio.h>
#include <stdlib.h>
#include "millis.h"
#include "serial.h"
#include "blade_state.h"
#include "pwm.h"

// multi-color blade presets
uint8_t blade_multi_colors[][BLADE_SEGMENTS][RGB_SIZE] = {
	// outlandish
	{{  0, 255,   0}, {  0,   0, 255}, {255, 255, 255}, {255,   0,   0}},   // rocket popsicle

	// subtle
	{{255,   0,   0}, {224,  16,   0}, {224,  32,   0}, {192,  64,   0}},   // red-to-orange
	{{192, 144,   0}, {128, 176,   0}, { 64, 224,   0}, {  0, 255,   0}},   // orange-to-green

	// light-to-dark *
	{{112,   0, 224}, { 72,   0, 144}, { 48,   0,  96}, { 32,   0,  64}},   // light-to-dark purple
	{{224, 112,   0}, { 144, 72,   0}, { 96,  48,   0}, { 64,  32,   0}},   // light-to-dark orange

	// *-to-white
	{{  0, 255,   0}, { 32, 224,  32}, { 64, 192,  64}, { 96, 128,  96}},   // green-to-white
	{{255,   0,   0}, {224,  32,  32}, {192,  64,  64}, {128,  96,  96}},   // red-to-white
	{{  0,   0, 255}, { 32,  32, 224}, { 64,  64, 192}, { 96,  96, 128}},   // blue-to-white

	// misc
	{{192,  64,   0}, {128,  48,  32}, { 96,  16,  64}, { 48,   0,  96}},   // orange-to-purple
};
uint8_t blade_multi_colors_len = sizeof(blade_multi_colors) / sizeof(blade_multi_colors[0]);

void dmode_handler(void) {
	static uint8_t last_dmode = 0;
	static uint8_t last_dsubmode = 0;
	static uint8_t last_blade_state = 0;
	static uint32_t next_step_time = 0;
	uint8_t i;

	// check for state in blade changes
	if (last_blade_state != blade.state) {

		// coming out of a clash
		if ((last_blade_state & 0xF0) == BLADE_STATE_CLASH && (blade.state & 0xF0) == BLADE_STATE_ON) {

			// which dmode are we in?
			switch (blade.dmode) {

				// color picker mode; allow clash to trigger manual color change
				case DMODE_COLOR_PICKER:
					next_step_time = millis() + 10000;						// pause auto-picker for 10 seconds
					blade.dmode_step += (16 - ((blade.dsubmode % 8) * 2));	// dsubmode controls color 'resolution'
					set_color_by_wheel(blade.dmode_step);					// display that color
					break;

				default:
					break;
			}
		}
		last_blade_state = blade.state;
	}

	// don't do anything while clash is in effect.
	//if ( (blade.state & 0xF0) == BLADE_STATE_CLASH || (blade.state & 0xF0) == BLADE_STATE_OFF ) {
	if ( (blade.state & 0xF0) == BLADE_STATE_CLASH ) {
		return;
	}

	// do any needed initialization for the new dmode
	if (blade.dmode != last_dmode) {

		#ifdef DEBUG_SERIAL_ENABLED
			serial_sendString("New DMODE detected: ");
			snprintf(serial_buf, SERIAL_BUF_LEN, "0x%02X\r\n", blade.dmode);
			serial_sendString(serial_buf);
		#endif

		// record new dmode
		last_dmode = blade.dmode;

		// do not reset dmode_step if new dmode is color_picked as dmode_step contains the color that was picked
		if (blade.dmode != DMODE_COLOR_PICKER_PICKED) {
			blade.dmode_step = 0;	// reset dmode step
		}
		last_dsubmode = blade.dsubmode - 1;	// force new dsubmode code to execute by changing value of last_dsubmode

		// if blade state has just been loaded from eeprom
		if (state_loaded_from_eeprom != 0) {

			// perform actions to set blade into correct state for the given mode
			switch (blade.dmode) {
				case DMODE_COLOR_PICKER_PICKED:
					set_color_by_wheel(blade.dmode_step); // set the blade color
					break;
			}

			state_loaded_from_eeprom = 0;             // clear flag
		}

		// perform one-time initialization unique for the new dmode
		switch (blade.dmode) {
			case DMODE_SEGMENT_WHEEL:
				blade.dmode_step = (rand() % 16) * 17;     // pick a random starting color

			case DMODE_MULTI_MODE:
				set_multi_mode();
				break;

			case DMODE_BLADE_WHEEL:
			case DMODE_COLOR_PICKER:
				blade.dmode_step = (rand() % 16) * 17;     // pick a random starting color

			case DMODE_STOCK:
			case DMODE_COLOR_PICKER_PICKED:
			default:
				set_single_mode();
				break;
		}
	}

	// do any needed initialization for the new dsubmode
	if (blade.dsubmode != last_dsubmode) {

		#ifdef DEBUG_SERIAL_ENABLED
			serial_sendString("New DSUBMODE detected: ");
			snprintf(serial_buf, SERIAL_BUF_LEN, "0x%02X\r\n", blade.dsubmode);
			serial_sendString(serial_buf);
		#endif

		switch (blade.dmode) {
			case DMODE_MULTI_MODE:
				for (uint8_t i=0;i<4;i++) {
					set_custom_segment_color(i,
						blade_multi_colors[blade.dsubmode % blade_multi_colors_len][i][0], 
						blade_multi_colors[blade.dsubmode % blade_multi_colors_len][i][1],
						blade_multi_colors[blade.dsubmode % blade_multi_colors_len][i][2]
					);
				}
				break;
		}
		last_dsubmode = blade.dsubmode;
		next_step_time = 0;
	}

	// manage dmode properties and animation;
	// how often this block of code gets executed is controlled by the next_step_time variable
	if (millis() >= next_step_time) {

		// set a default next_step_time of 1 second; this means this second of code will, by default, be executed once a second
		next_step_time = millis() + 1000;

		switch (blade.dmode) {

			case DMODE_STOCK:
			case DMODE_COLOR_PICKER_PICKED:

				switch (blade.dsubmode % DSUBMODE_MAX) {
					case DSUBMODE_NORMAL:
						break;

					// full blade flicker
					case DSUBMODE_FLICKER_FULL:
						// dim blade; use segment 0 to represent the brightness of the entire blade
						if (segment_brightness[0] > 40) {
							segment_brightness[0] *= .8;
						}

						// randomly pop blade
						if ((rand() % 3) == 0) {
							segment_brightness[0] += (255 - segment_brightness[0])*.5;
						}

						// copy brightness of segment 0 to rest of segments so entire blade has same brightness
						segment_brightness[1] = segment_brightness[0];
						segment_brightness[2] = segment_brightness[0];
						segment_brightness[3] = segment_brightness[0];

						next_step_time = millis() + 50;
						break;

					// segmented flicker
					case DSUBMODE_FLICKER_SEGMENTED:
						// dim blade
						for (i=0; i<4; i++) {
							if (segment_brightness[i] > 40) {
								segment_brightness[i] *= .8;
							}
						}

						// brighten a random segment (why am i reusing a variable? oh well...)
						next_step_time = (rand() % 4);
						segment_brightness[next_step_time] += (255 - segment_brightness[next_step_time])*.5;

						next_step_time = millis() + 50;
						break;

					case DSUBMODE_BREATHING:

						// determine increment or decrement based on even/odd of segment 0
						if (segment_brightness[0] % 2) {

							if (segment_brightness[0] < 12) {
								segment_brightness[0] = 8;
							} else {
								segment_brightness[0] -= 4;
							}
						} else {

							if (segment_brightness[0] > 250) {
								segment_brightness[0] = 255;
							} else {
								segment_brightness[0] += 4;
							}
						}

						// propagate values
						segment_brightness[1] = segment_brightness[0];
						segment_brightness[2] = segment_brightness[0];
						segment_brightness[3] = segment_brightness[0];

						// set time between steps
						next_step_time = millis() + 12;

						// pause breathing at full brightness for a bit
						if (segment_brightness[0] == 255) {
							next_step_time += 1500;
						}
						break;

					case DSUBMODE_BRIGHTNESS_66:
						set_blade_brightness(66);
						break;

					case DSUBMODE_BRIGHTNESS_33:
						set_blade_brightness(33);
						break;

					case DSUBMODE_BRIGHTNESS_10:
						set_blade_brightness(10);
						break;

					case DSUBMODE_STATIC_GRADIENT_1:
						set_segment_brightness(0, 100);
						set_segment_brightness(1, 80);
						set_segment_brightness(2, 60);
						set_segment_brightness(3, 40);
						break;

					case DSUBMODE_STATIC_GRADIENT_2:
						set_segment_brightness(0, 100);
						set_segment_brightness(1, 75);
						set_segment_brightness(2, 50);
						set_segment_brightness(3, 25);
						break;

					case DSUBMODE_FLICKER_DARK:
						// propagate flicker
						segment_brightness[3] += ((segment_brightness[2] - segment_brightness[3]));
						segment_brightness[2] += ((segment_brightness[1] - segment_brightness[2]));
						segment_brightness[1] += ((segment_brightness[0] - segment_brightness[1])*.8);

						// randomly flicker segment 0
						if ((rand() % 10) == 0) {
							segment_brightness[0] = (rand() % 60)+20;

							// otherwise brighten segment 0
						} else {
							segment_brightness[0] += ((255 - segment_brightness[0])*.5);
						}

						next_step_time = millis() + 50;
						break;

					// only dim segment 0, then propagate
					case DSUBMODE_FLICKER_BRIGHT:

						// propagate flicker
						for (i = 3; i > 0; i--) {
							segment_brightness[i] += ((segment_brightness[i-1] - segment_brightness[i]));
						}

						// randomly flicker segment 0
						if ((rand() % 5) == 0) {
							segment_brightness[0] += (rand() % (255-segment_brightness[0]));

						// otherwise dim segment 0
						} else if (segment_brightness[0] > 20) {
							segment_brightness[0] *= 0.8;
						}

						//as first segment gets dimmer the chance of a flicker increases
						//if (random(8 - (segment_brightness[0]/32)) == 0) {

						next_step_time = millis() + 50;
						break;

					// segment 0 flickers, other segments are some % of segment 0
					case DSUBMODE_FLICKER_GRADIENT:

						// propagate flicker
						segment_brightness[1] = segment_brightness[0] * .7;
						segment_brightness[2] = segment_brightness[0] * .5;
						segment_brightness[3] = segment_brightness[0] * .3;

						// flicker at random
						if ((rand() % 5) == 0) {
							segment_brightness[0] = 255;

							// or dim
						} else if (segment_brightness[0] > 40) {
							segment_brightness[0] *= 0.8;
						}

						next_step_time = millis() + 50;
						break;

					default:
						blade.dsubmode = DSUBMODE_NORMAL;
						break;
				}
				break;

			case DMODE_COLOR_PICKER:

				// only step to next color if blade is in an on state.
				// this prevents color-stepping during ignition and extinguish
				if ((blade.state & 0xF0) == BLADE_STATE_ON) {
					blade.dmode_step += 16 >> (blade.dsubmode % 5);					// 16,32,64,128,256 color resolution
				}
				set_color_by_wheel(blade.dmode_step);
				next_step_time = millis() + 2000;
				break;

			case DMODE_BLADE_WHEEL:
				blade.dmode_step++;
				set_color_by_wheel(blade.dmode_step);
				next_step_time = millis() + 25 - ((blade.dsubmode % 6) * 4);  // dsubmode controls speed of wheel
				break;

			case DMODE_SEGMENT_WHEEL:
				blade.dmode_step++;
				set_segment_color_by_wheel(3, blade.dmode_step);
				set_segment_color_by_wheel(2, blade.dmode_step + 16);
				set_segment_color_by_wheel(1, blade.dmode_step + 32);
				set_segment_color_by_wheel(0, blade.dmode_step + 48);
				next_step_time = millis() + 25 - ((blade.dsubmode % 6) * 4);  // dsubmode controls speed of wheel
				break;
		}
	}
}
