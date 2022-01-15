/* serial.h
 *
 * Comment out the DEBUG_SERIAL_ENABLED define to remove serial support
 * from the firmware. This will reduce the size of the firmware by 2-3kb.
 *
 */ 


#ifndef SERIAL_H_
#define SERIAL_H_

//#define DEBUG_SERIAL_ENABLED
#define SERIAL_BAUD_RATE 115200
#define USART0_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)
#define SERIAL_BUF_LEN	64

#ifdef __cplusplus
extern "C" {
#endif

extern char serial_buf[SERIAL_BUF_LEN];

void serial_setup(void);
void USART0_sendChar(char);
void serial_sendString(char*);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* SERIAL_H_ */