/* command_handler.c
 *
 */ 

#include <stdio.h>
#include "serial.h"
#include "millis.h"
#include "device_config.h"
#include "data.h"
#include "blade_state.h"

void command_handler(void) {
	static uint32_t last_off_time = 0;
	static uint32_t last_on_time = 0;
	static uint8_t reset_count = RESET_THRESHOLD_COUNT;
	uint8_t cmd, color;
	uint32_t time_now = millis();

	// data_cmd is populated by data_handler() and reset to 0 after being processed by command_handler()

	// is a new command available for processing?
	if (data_cmd != 0) {

		// parse command
		cmd  = data_cmd & 0xF0;
		color = data_cmd & 0x0F;

		// reset data_cmd which implicitly acknowledges the command has been received
		data_cmd = 0;

		switch (cmd) {
			case DATA_CMD_ON:
			case DATA_CMD_ON_LEGACY:
				last_on_time = time_now;
				blade.state = BLADE_STATE_POWER_ON;
				set_blade_brightness(0);

				// do not alter dmode or reset variables if dmode is disabled
				if ((switch_config & (1 << SW_DMODE_DISABLE_bp)) == 0) {

					// increment dmode or dsubmode if blade was off for less than DSUBMODE_THRESHOLD_TIME
					if ((time_now - last_off_time) < DSUBMODE_THRESHOLD_TIME && last_off_time > 0) {

						// increment dmode and reset dsubmode if blade was off for less than DMODE_THRESHOLD_TIME
						if ((time_now - last_off_time) < DMODE_THRESHOLD_TIME) {
							blade.dmode++;
							blade.dsubmode = 0;
							if (blade.dmode >= DMODE_MAX) {
								blade.dmode = 0;
							}

						// otherwise just increment dsubmode
						} else {
							blade.dsubmode++;
						}
					}

					// announce current state of dmode system
					#ifdef DEBUG_SERIAL_ENABLED
						serial_sendString("DMODE: ");
						snprintf(serial_buf, SERIAL_BUF_LEN, "%02x", blade.dmode);
						serial_sendString(serial_buf);
						serial_sendString(", DSUBMODE: ");
						snprintf(serial_buf, SERIAL_BUF_LEN, "%02x", blade.dsubmode);
						serial_sendString(serial_buf);
						serial_sendString(", TIME: ");
						snprintf(serial_buf, SERIAL_BUF_LEN, "%lu\r\n", (millis() - last_off_time));
						serial_sendString(serial_buf);
					#endif

					//
					// RESET COUNTER MANAGEMENT - PART 1
					//
					// if blade was off less than RESET_THRESHOLD_TIME, decrement reset counter by 1
					if ((time_now - last_off_time) < RESET_THRESHOLD_TIME && last_off_time > 0) {

						// do not decrement to 0; only do then when powering off so the blade is in an off state when it disconnects from the hilt during reset
						if (reset_count > 1) {
							reset_count--;
							#ifdef DEBUG_SERIAL_ENABLED
								serial_sendString("Decremented reset_count to ");
								snprintf(serial_buf, SERIAL_BUF_LEN, "%d\r\n", reset_count);		// EEPROM_SIZE is in BYTES
								serial_sendString(serial_buf);
							#endif
						}

					// otherwise reset the counter
					} else {
						reset_count = RESET_THRESHOLD_COUNT;
						#ifdef DEBUG_SERIAL_ENABLED
							serial_sendString("Resetting reset_count to ");
							snprintf(serial_buf, SERIAL_BUF_LEN, "%d\r\n", RESET_THRESHOLD_COUNT);		// EEPROM_SIZE is in BYTES
							serial_sendString(serial_buf);
						#endif
					}
				}

				// if in stock mode, set blade color based off color table
				if (blade.dmode == DMODE_STOCK) {
					if (cmd == DATA_CMD_ON) {
						blade.color_state = color + (STOCK_BLADE_COLOR_TABLE_SAVI << 4);
					} else {
						blade.color_state = color + (STOCK_BLADE_COLOR_TABLE_LEGACY << 4);
					}
					set_blade_color();
				}
				break;

			case DATA_CMD_COLOR:
				if (blade.dmode == DMODE_STOCK && blade.dsubmode == DSUBMODE_NORMAL) {    // ignore command unless blade is in stock mode
					blade.state = BLADE_STATE_ON;
					blade.color_state = color + (STOCK_BLADE_COLOR_TABLE_SAVI << 4);
					set_blade_color();
					set_blade_brightness(100);
				}
				break;

			case DATA_CMD_COLOR_LEGACY:
				if (blade.dmode == DMODE_STOCK && blade.dsubmode == DSUBMODE_NORMAL) {    // ignore command unless blade is in stock mode
					blade.state = BLADE_STATE_ON;
					blade.color_state = color + (STOCK_BLADE_COLOR_TABLE_LEGACY << 4);
					set_blade_color();
					set_blade_brightness(100);
				}
				break;

			case DATA_CMD_REDFLICKER_1:
				if (blade.dmode == DMODE_STOCK && (blade.state & 0xF0) == BLADE_STATE_ON) { // only flicker when the blade is on and in stock mode
					blade.state = BLADE_STATE_REDFLICKER;
					blade.color_state = color;
				}
				break;

			case DATA_CMD_REDFLICKER_2:
				if (blade.dmode == DMODE_STOCK && (blade.state & 0xF0) == BLADE_STATE_ON) { // only flicker when the blade is on and in stock mode
					blade.state = BLADE_STATE_REDFLICKER | (1<<2);
					blade.color_state = color;
				}
				break;

			case DATA_CMD_FLASH:
			case DATA_CMD_FLASH_LEGACY:
				if ((blade.state & 0xF0) == BLADE_STATE_ON) {
					blade.state = BLADE_STATE_CLASH;            // only clash when the blade is on
				}
				break;

			case DATA_CMD_OFF:
			case DATA_CMD_OFF_LEGACY:
				if (blade.state != BLADE_STATE_OFF && blade.state != BLADE_STATE_POWER_OFF) {  // prevent going into BLADE_STATE_POWER_OFF if blade is already off or turning off
					last_off_time = time_now;
					blade.state = BLADE_STATE_POWER_OFF;
				}
				break;

			case DATA_CMD_4:
			case DATA_CMD_4_LEGACY:
				if (blade.state != BLADE_STATE_OFF && blade.state != BLADE_STATE_POWER_OFF) {
					last_off_time = time_now;
					set_blade_brightness(0);        // shut off blade
					blade_power_off();              // shut off LDO that powers RGB LEDs
					blade.state = BLADE_STATE_OFF;  // set blade state to off
				}

			default:
				break;
		}

		// do not alter reset variables if dmode is disabled
		if ((switch_config & (1 << SW_DMODE_DISABLE_bp)) == 0) {

			//
			// RESET COUNTER MANAGEMENT - PART 2
			//
			// blade is powering off
			if (blade.state == BLADE_STATE_POWER_OFF) {

				// if blade was on for less than RESET_THRESHOLD_TIME decrement reset counter
				if ((time_now - last_on_time) < RESET_THRESHOLD_TIME && last_on_time > 0) {
					reset_count--;
					#ifdef DEBUG_SERIAL_ENABLED
						serial_sendString("Decremented reset_count to ");
						snprintf(serial_buf, SERIAL_BUF_LEN, "%d\r\n", reset_count);		// EEPROM_SIZE is in BYTES
						serial_sendString(serial_buf);
					#endif

					// if reset_count reaches 0, trigger a reset
					if (reset_count == 0) {
						blade.dmode = DMODE_STOCK;
						blade.dsubmode = DSUBMODE_NORMAL;
						blade.dmode_step = 0;

						// change blade state to RESET
						blade.state = BLADE_STATE_RESET;

						#ifdef DEBUG_SERIAL_ENABLED
							serial_sendString("RESET TRIGGERED\r\n\r\n");
						#endif
					}

				// blade was on for longer than RESET_THRESHOLD_TIME so reset the counter
				} else {
					reset_count = RESET_THRESHOLD_COUNT;
					#ifdef DEBUG_SERIAL_ENABLED
						serial_sendString("Resetting reset_count to ");
						snprintf(serial_buf, SERIAL_BUF_LEN, "%d\r\n", RESET_THRESHOLD_COUNT);		// EEPROM_SIZE is in BYTES
						serial_sendString(serial_buf);
					#endif
				}
			}
		}
	}
}
