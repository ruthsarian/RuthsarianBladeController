/* eeprom.c
 *
 * This code manages the EEPROM area of the microcontroller where settings
 * will be stored.
 *
 * References:
 * http://www.fourwalledcubicle.com/AVRArticles.php
 * https://raw.githubusercontent.com/abcminiuser/avr-tutorials/master/EEPROM/Output/EEPROM.pdf
 *
 * TODO: 
 * save/load blade state to EEPROM
 *
 */ 

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <avr/eeprom.h>
#include "serial.h"
#include "eeprom.h"
#include "blade_state.h"
#include "device_config.h"

const char eeprom_magic[EEPROM_MAGIC_LEN] = "SWGE";

void eeprom_dump(void) {
	uint16_t addr;

	// display size of EEPROM
	serial_sendString("EEPROM size: ");
	snprintf(serial_buf, SERIAL_BUF_LEN, "%d", EEPROM_SIZE);		// EEPROM_SIZE is in BYTES
	serial_sendString(serial_buf);
	serial_sendString(" bytes\r\n");
	serial_sendString("EEPROM Contents:");

	// loop through each byte of EEPROM
	for(addr=0; addr<EEPROM_SIZE; addr++) {

		// add whitespace for readability
		if (addr % 16 == 0) {
			snprintf(serial_buf, SERIAL_BUF_LEN, "\r\n  [%04X] ", addr);
			serial_sendString(serial_buf);
		} else if (addr % 8 == 0 ) {
			serial_sendString("   ");
		} else {
			serial_sendString(" ");
		}

		// output current byte of EEPROM
		snprintf(serial_buf, SERIAL_BUF_LEN, "%02X", eeprom_read_byte((uint8_t *)addr));	// eeprom_read_byte((uint8_t *)(uint16_t)addr)
																							// pointers are 16-bit, so need to cast addr to a 16-bit value before casting to a pointer to an 8-bit value
																							// alternatively, just make addr a 16 bit value; which is what was done here
		serial_sendString(serial_buf);
	}
	serial_sendString("\r\n\r\n");
}

void eeprom_setup(void) {
	#ifdef DEBUG_SERIAL_ENABLED
		eeprom_dump();
	#endif
	eeprom_load_state();
}

void eeprom_reset(void) {
	uint16_t addr = EEPROM_START_ADDR;
	uint8_t i;

	#ifdef DEBUG_SERIAL_ENABLED
		serial_sendString("Initializing EEPROM\r\n");
	#endif

	// always use eeprom_update over eeprom_write in order to save lifespan of EEPROM
	//eeprom_update_block((void*)eeprom_magic, (void*)0, sizeof(char)*EEPROM_MAGIC_LEN);
	for(i=0; i<EEPROM_MAGIC_LEN; i++) {
		eeprom_update_byte((uint8_t *)addr, eeprom_magic[i]);
		addr++;
	}

	// initialize blade struct area of EEPROM with all zeroes
	for (i=0; i<sizeof(struct blade_state_struct); i++) {
		eeprom_update_byte((uint8_t *)addr, 0);
		addr++;
	}
}

// load the blade state from eeprom
void eeprom_load_state(void) {
	uint16_t addr = EEPROM_START_ADDR;
	uint8_t i;
	uint8_t *bs = (uint8_t*)&blade;

	// test for EEPROM magic
	for(i=0;i<EEPROM_MAGIC_LEN;i++) {

		// test for magic at start of EEPROM
		if ( eeprom_magic[i] != eeprom_read_byte((uint8_t *)addr) ) {

			#ifdef DEBUG_SERIAL_ENABLED
				serial_sendString("EEPROM Magic Missing! Initializing EEPROM.");
			#endif

			// reset EEPROM if magic is missing
			eeprom_reset();
			break;
		}
		addr++;
	}

	#ifdef DEBUG_SERIAL_ENABLED
		serial_sendString("Loading blade state from EEPROM...\r\n");
	#endif

	// load blade state from eeprom
	addr = EEPROM_START_ADDR + sizeof(eeprom_magic);
	for(i=0;i<sizeof(struct blade_state_struct);i++) {
		*bs = eeprom_read_byte((uint8_t *)addr);
		bs++;
		addr++;
	}

	// set flag that blade state has been loaded from eeprom
	state_loaded_from_eeprom = 1;

	// report loaded blade state
	#ifdef DEBUG_SERIAL_ENABLED
		dump_blade_state();
	#endif
}

// store contents of blade state struct to EEPROM
void eeprom_store_state(void) {
	uint16_t addr = EEPROM_START_ADDR;
	uint8_t i;

	// do not store to EEPROM if write-protect is enabled or dmode is enabled
	if (switch_config == 0) {

		#ifdef DEBUG_SERIAL_ENABLED
			serial_sendString("\r\nStoring blade state in EEPROM...\r\n");
			dump_blade_state();
		#endif

		// write 'magic' to start of EEPROM
		for(i=0; i<EEPROM_MAGIC_LEN; i++) {
			eeprom_update_byte((uint8_t *)addr, eeprom_magic[i]);
			addr++;
		}

		// write (raw) contents of blade state struct to EEPROM
		for (i=0; i<sizeof(struct blade_state_struct); i++) {
			eeprom_update_byte((uint8_t *)addr, ((uint8_t*)&blade)[i]);
			addr++;
		}
	}
}

