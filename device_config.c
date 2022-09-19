/*
 * device_config.c
 *
 * see FUSE_struct in iotn806.h or iotn1606.h 
 * C:\Program Files (x86)\Atmel\Studio\7.0\packs\atmel\ATtiny_DFP\1.8.332\include\avr
 *
 */ 

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include "serial.h"
#include "millis.h"
#include "data.h"
#include "eeprom.h"
#include "device_config.h"
#include "blade_state.h"
#include "dmode_handler.h"
#include "pwm.h"


// set the FUSES for the ATtiny806/1606; the default fuse values are used
// this exists so fuse data can be extracted from the compiled program and 
// written to the ATtiny IF NEEDED.
#ifndef ARDUINO
FUSES = {
	.WDTCFG			= WINDOW_OFF_gc | PERIOD_OFF_gc,										// FUSE BYTE 0: FUSE_WDTCFG_DEFAULT (0x00)
	.BODCFG			= LVL_BODLEVEL0_gc | SAMPFREQ_1KHZ_gc | ACTIVE_DIS_gc | SLEEP_DIS_gc,	// FUSE BYTE 1: FUSE_BODCFG_DEFAULT (0x00)
	.OSCCFG			= FREQSEL_20MHZ_gc,														// FUSE BYTE 2: FUSE_OSCCFG_DEFAULT (0x02)
	.reserved_1		= {0xFF,																// FUSE BYTE 3: reserved; set to 0xFF
					   0xFF},																// FUSE BYTE 4: reserved; set to 0xFF
	.SYSCFG0		= CRCSRC_NOCRC_gc | RSTPINCFG_UPDI_gc | FUSE_EESAVE_bm, 				// FUSE BYTE 5: FUSE_SYSCFG0_DEFAULT (0xC4) + enable EEPROM retention (FUSE_EESAVE_bm)
	.SYSCFG1		= SUT_64MS_gc,															// FUSE BYTE 6: FUSE_SYSCFG1_DEFAULT (0x07)
	.APPEND			= 0x00,																	// FUSE BYTE 7: FUSE BYTE 0: FUSE_APPEND_DEFAULT (0x00)
	.BOOTEND		= 0x00,																	// FUSE BYTE 8: FUSE_BOOTEND_DEFAULT (0x00)
																							// FUSE BYTE 9: reserved; not defined in FUSE_struct so skip
																							// FUSE BYTE 10: not defined in FUSE_struct so skip
};
#endif

uint8_t switch_config = 0;

void device_setup(void) {

	// set the CPU frequency; only care about 10MHz
	// 20MHz is here because while technically possible, datasheet says at 3.3V the ATtiny can't run at 20MHz
	#if (F_CPU == 20000000)
		_PROTECTED_WRITE(CLKCTRL_MCLKCTRLB, 0);											// disable clock prescaler (20MHz clock)
	#elif (F_CPU == 10000000)
		_PROTECTED_WRITE(CLKCTRL_MCLKCTRLB, (CLKCTRL_PEN_bm | CLKCTRL_PDIV_2X_gc));		// enable clock prescaler of 2 (20MHz/2 = 10MHz)
	#else
		#error "What's my F_CPU then?"
	#endif

	// seed the RNG
	srand_init();

	// read state of hardware switches and record them
	record_switch_config();

	// minimize power consumption by disabling peripherals and setting all pins to OUTPUT by default
	RTC.CTRLA = 0;		// disable RTC
	WDT.CTRLA = 0;		// disable WDT
	BOD.CTRLA = 0;		// disable BOD
	PORTA.DIR = 0xFF;	// set all pins as OUTPUT by default
	PORTB.DIR = 0xFF;
	PORTC.DIR = 0xFF;

	// setup millis() and micros() support
	millis_setup();

	// enable USART0
	serial_setup();

	#ifdef DEBUG_SERIAL_ENABLED

		// display banner
		serial_sendString("\r\n\r\n");
		serial_sendString("-----------------------------------------\r\n");
		serial_sendString(" Ruthsarian's Custom GE Blade Controller\r\n");
		serial_sendString("      contact: ruthsarian@gmail.com\r\n");
		serial_sendString("-----------------------------------------\r\n");

		// report startup
		serial_sendString("Starting Up...\r\n\r\n");

		// report VCC
		serial_sendString("VCC: ");
		dtostrf(measure_vcc(), 4, 2, serial_buf);
		serial_sendString(serial_buf);
		serial_sendString("V\r\n\r\n");
	#endif

	// set the sleep mode
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();

	// initialize the data pin
	data_setup();

	// initialize blade LEDs LDO control pin
	LDO_PORT.DIRSET = LDO_PIN_bm;

	// initialize EEPROM
	eeprom_setup();

	// if dmode is disabled and WP is not enabled, explicitly set dmode to stock 
	if ((switch_config ^ (1 << SW_DMODE_DISABLE_bp)) == 0) {
		blade.dmode = DMODE_STOCK;
		blade.dsubmode = DSUBMODE_NORMAL;
	}

	// initialize PWM pins and timers
	pwm_setup();

	// enable global interrupts
	sei();

	#ifdef DEBUG_SERIAL_ENABLED
		switch_report();
		srand_sample_report();
		serial_sendString("Startup complete. Controller is ready!\r\n\r\n");
	#endif
}

void srand_init(void) {
	uint16_t seed = 0;
	uint8_t sample = 0;
	
	ADC0.CTRLA = 1 << ADC_ENABLE_bp;					// enable ADC
	ADC0.CTRLB = ADC_SAMPNUM_ACC64_gc;					// accumulate 64 samples per conversion
	ADC0.CTRLC = 1 << ADC_SAMPCAP_bp					// reduced sampling capacitance (recommended for voltages over 1V)
	           | ADC_REFSEL_VDDREF_gc					// use VDD as a voltage reference; CRITICAL when sampling unused pin for srand seeding
	           | ADC_PRESC_DIV32_gc;					// prescale the ADC clock (is this necessary? probably not)
	ADC0.CTRLD = ADC_INITDLY_DLY256_gc;					// delay initialization 256 CLK_ADC cycles (let things settle down)
	ADC0.MUXPOS = ADC_MUXPOS_AIN3_gc ;					// connect AIN3 (PA3; an unused pin) to the ADC

	// srand() takes an unsigned integer (16-bit) value.  the ADC sample returns a 10-bit value.
	//
	// this loop takes 16 sample results (each result comprised of 64 samples), extracting the 
	// LSB of each sample and shifting it into seed
	for (sample=0; sample<16; sample++) {
		ADC0.INTFLAGS = 1 << ADC_RESRDY_bp;					// clear the results ready flag
		ADC0.COMMAND = 1 << ADC_STCONV_bp;					// start ADC sample
		while (!(ADC0.INTFLAGS & (1 << ADC_RESRDY_bp) )) {}	// wait for results
		seed = ((seed << 1) | (ADC0.RESL & 1));				// accumulate the seed
	}

	ADC0.CTRLA &= ~(1 << ADC_ENABLE_bp);				// disable ADC
	ADC0.MUXPOS = 0;									// disconnect pins from ADC

	srand(seed);										// seed the RNG
}

void srand_sample_report(void) {
	uint8_t i;
	
	serial_sendString("RNG TEST:\r\n");
	for (i=0;i<100;i++) {
		if (i>0 && i%10 == 0) {
			serial_sendString("\r\n");
		}
		snprintf(serial_buf, SERIAL_BUF_LEN, "  %04X", rand());	// RAND_MAX = 0x7FFF
		serial_sendString(serial_buf);
	}
	serial_sendString("\r\n\r\n");
}

void blade_power_on(void) {
	LDO_PORT.OUTSET = LDO_PIN_bm;
}

void blade_power_off(void) {
	LDO_PORT.OUTCLR = LDO_PIN_bm;
}

void sleep_handler(void) {
	static uint32_t last_off_time = 0;
	static uint8_t last_state = 0xFF;

	// is the blade off?
	if ((blade.state & 0xF0) == BLADE_STATE_OFF) {

		// did the blade just turn off?
		if (last_state != blade.state) {
			last_off_time = millis();

		// has it been off for more than X seconds?
		} else if ((millis() - last_off_time) > OFF_TO_SLEEP_TIME) {

			// i want to minimize EEPROM writes; a write after EVERY dmode or dsubmode change is not desireable.
			// so i will store blade state to eeprom when blade is going to sleep. this means blade has been off for
			// a significant amount of time and it's unlikely the user is spamming dmode/dsubmode changes
			eeprom_store_state();

			// put the blade to sleep.
			// no further code is executed after sleep_cpu() until the blade wakes up
			#ifdef DEBUG_SERIAL_ENABLED
				serial_sendString("Going to sleep.\r\n\r\n");
				last_off_time = millis() + 100;
				while (millis() < last_off_time) {
					;
				}
			#endif

			// put the microcontroller to sleep; no further code is executed after sleep_cpu() until the mcu wakes up
			sleep_cpu();

			// restart the power off timer
			last_off_time = millis();

			#ifdef DEBUG_SERIAL_ENABLED
				serial_sendString("\r\n\r\nWaking up!\r\n\r\n");
				dump_blade_state();
			#endif
		}
	}
	last_state = blade.state;
}

uint8_t reset_handler(void) {
	static uint32_t reset_time = 0;

	// is the blade in a reset state?
	if (blade.state == BLADE_STATE_RESET) {

		// has the blade just entered a reset state?
		if (reset_time == 0) {

			// record start of reset period
			reset_time = millis();

			// shut off blade
			clear_blade_color();            // remove any blade color information
			blade_power_off();              // shut off LDO that powers RGB LEDs

			// disable data pin
			disable_data_pin();

			// reset the data in EEPROM only if blade is not in locked or stock mode
			//if (((lock_and_stock & (1 << LOCK_BLADE_bp)) | (lock_and_stock & (1 << STOCK_BLADE_bp))) == 0) {
			if (switch_config == 0) {

				// reset the data in EEPROM
				eeprom_reset();
			}

		// time to bring the blade out of a reset state
		} else if ((millis() - reset_time) >= RESET_STATE_PERIOD) {
			reset_time = 0;

			// load blade state from EEPROM
			eeprom_load_state();

			//blade.state = BLADE_STATE_OFF;

			// enable data pin
			enable_data_pin();

			return 0;
		}
		return 1;
	}
	return 0;
}

// http://ww1.microchip.com/downloads/en/AppNotes/00002447A.pdf
float measure_vcc(void) {

	float vcc_value = 0;
	uint8_t old_vref_ctrla = VREF.CTRLA;

	// select 1.1V reference voltage for ADC0
	VREF.CTRLA = VREF_ADC0REFSEL_1V1_gc;

	// attach selected internal voltage reference to ADC
	ADC0.MUXPOS = ADC_MUXPOS_INTREF_gc;

	ADC0.CTRLA = 1 << ADC_ENABLE_bp;		// enable ADC, full resolution
	ADC0.CTRLB = ADC_SAMPNUM_ACC64_gc;		// accumulate 64 samples

	ADC0.CTRLC = ADC_PRESC_DIV2_gc			// set sample rate of F_CPU/2
	           | ADC_REFSEL_VDDREF_gc		// use Vcc(Vdd) as voltage reference
	           | 1 << ADC_SAMPCAP_bp;		// enable reduced sampling capacitance per datasheet recommendation

	ADC0.CTRLD = ADC_INITDLY_DLY256_gc;		// delay ADC initialization

	ADC0.INTFLAGS = 1 << ADC_RESRDY_bp;		// clear the results ready flag
	ADC0.COMMAND = 1 << ADC_STCONV_bp;		// start ADC sample

	// wait for results
	while (!(ADC0.INTFLAGS & (1 << ADC_RESRDY_bp) )) {}

	// calculate Vcc: (ref voltage * max ADC value * # of samples) / ADC result
	// 10-bit ADC, thus max value is 0x03FF
	vcc_value = (1.1 * 0x400 * 64) / ADC0.RES;

	VREF.CTRLA = old_vref_ctrla;			// restore original VREF selection
	ADC0.CTRLA &= ~(1 << ADC_ENABLE_bp);	// disable ADC
	ADC0.MUXPOS = 0;						// disconnect pins from ADC

	return vcc_value;
}

void record_switch_config(void) {

	// set switch pins as INPUT
	SW_WRITE_PROTECT_PORT.DIRCLR = SW_WRITE_PROTECT_PIN_bm;
	SW_DMODE_DISABLE_PORT.DIRCLR = SW_DMODE_DISABLE_PIN_bm;
	
	// enable internal pullup on switch pins
	SW_WRITE_PROTECT_PORT.SW_WRITE_PROTECT_PIN_CTRL |= PORT_PULLUPEN_bm;
	SW_DMODE_DISABLE_PORT.SW_DMODE_DISABLE_PIN_CTRL |= PORT_PULLUPEN_bm;

	// read state of write protect switch and record
	if ((SW_WRITE_PROTECT_PORT.IN & SW_WRITE_PROTECT_PIN_bm) == 0) {
		switch_config |= (1 << SW_WRITE_PROTECT_bp);
	}

	// read state of dmode disable switch and record
	if ((SW_DMODE_DISABLE_PORT.IN & SW_DMODE_DISABLE_PIN_bm) == 0) {
		switch_config |= (1 << SW_DMODE_DISABLE_bp);
	}
	
	// disable pullups to save power
	SW_WRITE_PROTECT_PORT.SW_WRITE_PROTECT_PIN_CTRL &= ~PORT_PULLUPEN_bm;
	SW_DMODE_DISABLE_PORT.SW_DMODE_DISABLE_PIN_CTRL &= ~PORT_PULLUPEN_bm;

	// reconfigure pins as output to save power
	SW_WRITE_PROTECT_PORT.DIRSET = SW_WRITE_PROTECT_PIN_bm;
	SW_DMODE_DISABLE_PORT.DIRSET = SW_DMODE_DISABLE_PIN_bm;
}

void switch_report(void) {
	serial_sendString("SWITCH STATE:\r\n");
	if (switch_config & (1 << SW_WRITE_PROTECT_bp)) {
		serial_sendString("  Write Protect: enabled\r\n");
	} else {
		serial_sendString("  Write Protect: disabled\r\n");
	}

	if (switch_config & (1 << SW_DMODE_DISABLE_bp)) {
		serial_sendString("  Disable DMODE: enabled\r\n");
	} else {
		serial_sendString("  Disable DMODE: disabled\r\n");
	}
	serial_sendString("\r\n");
}
