/* data.h
 *
 * Manage reception and decoding of commands from the hilt.
 */ 

#ifndef DATA_H_
#define DATA_H_

// I/O pin configuration for data reception
#define DATA_PORT     PORTC
#define DATA_PIN_bm   PIN3_bm
#define DATA_PIN_CTRL PIN3CTRL
#define DATA_PIN_ISR  PORTC_PORT_vect
#define DATA_ACTIVE   0
#define DATA_IDLE     DATA_PIN_bm

// HILT COMMANDS
#define DATA_CMD_0            0x00  // does nothing, unknown
#define DATA_CMD_0_LEGACY     0x10  //
#define DATA_CMD_ON           0x20  // ignite, 2nd nibble is color, uses savi saber color table
#define DATA_CMD_ON_LEGACY    0x30  // ignite, 2nd nibble is color, uses legacy saber color table
#define DATA_CMD_OFF          0x40  // animates current color to off
#define DATA_CMD_OFF_LEGACY   0x50  //
#define DATA_CMD_REDFLICKER_1 0x60  // set red blade color, color nibble affects brightness; nearly off to mid brightness
#define DATA_CMD_REDFLICKER_2 0x70  //                                                        mid brightness to full brightness
#define DATA_CMD_4            0x80  // turns blade off; keep off until only another ignite command? it is sent after a power off for legacy (and savi's?)
#define DATA_CMD_4_LEGACY     0x90  //
#define DATA_CMD_COLOR        0xA0  // set color, uses savi lightsaber/kyber crystal color table
#define DATA_CMD_COLOR_LEGACY 0xB0  //             uses legacy lightsaber color table
#define DATA_CMD_FLASH        0xC0  // animates burst with current color
#define DATA_CMD_FLASH_LEGACY 0xD0  //
#define DATA_CMD_7            0xE0  // turns blade off; (disable blade until it is unplugged?)
#define DATA_CMD_7_LEGACY     0xF0  //

// DATA RECEPTION CIRCLE BUFFER
#define DATA_CBUF_LEN         8     // length of the circle buffer used to store bits sent from the hilt; should be some power of 2
#define DATA_BIT_MAX_LEN      5000  // maximum length of time, in microseconds, that data pin should be held active to indicate a bit; 5000uS was an arbitrary choice, it's less than the length of the preamble (12ms), but more than the length of a '1' bit (1.2ms)
#define DATA_BIT_ONE_MAX_LEN  1800  // maximum length of time, in microseconds, that data pins should be held active to indicate a bit value of ONE; any longer and it's a bit value of ZERO

#ifdef __cplusplus
extern "C" {
#endif

// GLOBAL: data_cmd - store the current command from the data line
extern uint8_t data_cmd;

struct data_cbuf_struct {
  uint8_t state;            // data pin state
  uint32_t state_time;      // time the state was recorded
};

// setup the DATA pin for reception of commands from the hilt
void data_setup(void);

// disable the data pin
void disable_data_pin(void);

// enable the data pin
void enable_data_pin(void);

// manage commands coming from hilt
void data_handler(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* DATA_H_ */
