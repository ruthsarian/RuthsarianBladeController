/* serial.c
 *
 * This code manages sending data over the serial port (USART0). This code 
 * moves USART0 to its alternative pins as the default pins are already in
 * use managing the blade's LEDs.
 * 
 * default USART0 pins:
 *    11 PB3 RXD
 *    12 PB4 TXD
 *
 * alt USART0 pins:
 *    20 PA1 TXD
 *     1 PA2 RXD
 */ 

#include <avr/io.h>
#include <string.h>
#include "serial.h"

char serial_buf[SERIAL_BUF_LEN];

void serial_setup( void ) {

  // move USART0 to alternative pins
  PORTMUX.CTRLB |= PORTMUX_USART0_ALTERNATE_gc;

  // set ALT USART0 TX (PA1) to output
  PORTA.DIRSET = PIN1_bm;

  // set USART0 baud rate  
  USART0.BAUD = (uint16_t)USART0_BAUD_RATE(SERIAL_BAUD_RATE);

  // enable TX for USART0
  USART0.CTRLB |= USART_TXEN_bm;
}

void USART0_sendChar(char c) {
  while (!(USART0.STATUS & USART_DREIF_bm)) {
    ;
  }
  USART0.TXDATAL = c;
}

void serial_sendString(char *str)
{
  for(size_t i = 0; i < strlen(str); i++)   {
    USART0_sendChar(str[i]);
  }
}
