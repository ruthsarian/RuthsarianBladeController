/* data.c
 *
 */ 

#include <stdio.h>
#include <avr/interrupt.h>
#include "data.h"
#include "millis.h"
#include "serial.h"

// initialize global variables
uint8_t data_cmd = 0;
volatile struct data_cbuf_struct cbuf[DATA_CBUF_LEN];
volatile uint8_t data_cbuf_rpos = 0;
volatile uint8_t data_cbuf_wpos = 0;

// Data Pin State Change Interrupt Service Request
// triggered when the state of the data pin changes
// ISRs need to be as brief as possible, so just record the state and time of 
// change to a buffer which will be processed later by data_handler()
ISR(DATA_PIN_ISR) {
	cbuf[data_cbuf_wpos].state = DATA_PORT.IN & DATA_PIN_bm;		// record pin state to the buffer
	cbuf[data_cbuf_wpos].state_time = micros();						// record time (in microseconds) to the buffer
	data_cbuf_wpos = (data_cbuf_wpos + 1) & (DATA_CBUF_LEN - 1);	// increment write buffer position
	DATA_PORT.INTFLAGS |= DATA_PIN_bm;								// clear the interrupt
}

void data_setup(void) {

	// initialize DATA pin as input and enable pullup
	DATA_PORT.DIRCLR = DATA_PIN_bm;
	DATA_PORT.DATA_PIN_CTRL |= PORT_PULLUPEN_bm;

	// enable interrupt for data pin
	DATA_PORT.DATA_PIN_CTRL |= PORT_ISC_BOTHEDGES_gc;
}

void disable_data_pin(void) {
	DATA_PORT.DATA_PIN_CTRL &= ~(PORT_PULLUPEN_bm);
}

void enable_data_pin(void) {
	DATA_PORT.DATA_PIN_CTRL |= PORT_PULLUPEN_bm;
}

void data_handler(void) {
	static uint8_t cmd = 0;			// a variable to hold the command byte as it's being received from the hilt
	static uint8_t bit_cnt = 0;		// counting the number of bits received
	static uint32_t last_time = 0;	// store the last time the state of the data pin changed
	uint8_t data_state;				// get the current state of the data pin
	uint32_t time_diff;
	uint32_t current_time;

	if (data_cbuf_rpos == data_cbuf_wpos) {		// only proceed if there is data available to process
		return;
	}
	data_state = cbuf[data_cbuf_rpos].state;                        // read state from buffer
	current_time = cbuf[data_cbuf_rpos].state_time;                 // read time of state from buffer
	data_cbuf_rpos = (data_cbuf_rpos + 1) & (DATA_CBUF_LEN - 1);    // increment read buffer position

	// calculate time difference since last state change on data pin
	// this is convoluted because we have to worry about the microsecond counter overflowing between measurements
	if (current_time >= last_time) {
		time_diff = current_time - last_time;
	} else {
		time_diff = current_time + ~last_time;
	}

	// record new state change time for next time this function gets called
	last_time = current_time;

	// data pin is IDLE, meaning it has just changed from ACTIVE to IDLE
	// based on how long the pin was active, either add a bit to the command value or reset the command
	//
	// assumptions are being made and some aspects of the data command protocol are being ignored
	// this is probably not the "correct" way to do this, but it works well enough
	if (data_state == DATA_IDLE) {
		if (time_diff < DATA_BIT_MAX_LEN) {			// active for less than DATA_BIT_MAX_LEN microseconds indicates a bit
			cmd <<= 1;								// shift the byte left by 1 position
			if (time_diff < DATA_BIT_ONE_MAX_LEN) {	// active for less than 1.8ms we'll assume indicates a bit value of 1 (longer time = bit value 0)
				cmd++;								// add one to the byte value
			}
			if (++bit_cnt == 8) {					// if 8 bits have been recorded, send it to the program and reset the local bit count and command byte values
				data_cmd = cmd;						// copy decoded command to the global variable which will be picked up by command_handler()
				bit_cnt = 0;
				cmd = 0;
				
/*				#ifdef DEBUG_SERIAL_ENABLED
					serial_sendString("CMD: ");
					snprintf(serial_buf, SERIAL_BUF_LEN, "%02x", data_cmd);
					serial_sendString(serial_buf);
					serial_sendString("\r\n");
				#endif
*/			}
		} else {	// if active over 10ms then it's likely a preamble to an incoming command, so reset bit count and command byte values
			bit_cnt = 0;
			cmd = 0;
		}
	}
}