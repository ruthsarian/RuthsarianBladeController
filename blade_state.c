/* blade_state.c
 *
 * initialize global variables relating to blade state
 * define functions that can be used to manipulate these global variables
 * define command_handler() which changes blade state based off commands from hilt
 */ 

#include <stdio.h>
#include "millis.h"
#include "serial.h"
#include "device_config.h"
#include "blade_state.h"
#include "data.h"

// initialization of global variables
// see blade_state.h for their purpose

struct blade_state_struct blade = {0, 0, 0, 0, 0};
uint8_t state_loaded_from_eeprom = 0;
volatile uint8_t segment_color[BLADE_SEGMENTS][RGB_SIZE] = {
  { 255,   0,   0},
  {   0, 255,   0},
  {   0,   0, 255},
  { 255, 255, 255}
};
uint8_t segment_brightness[BLADE_SEGMENTS] = { 0, 0, 0, 0 };
uint8_t max_segment_brightness[BLADE_SEGMENTS] = { 0, 0, 0, 0 };
volatile uint8_t true_segment_brightness[BLADE_SEGMENTS];
uint8_t stock_blade_colors[STOCK_BLADE_COLOR_LEN][RGB_SIZE] = {
  //  RED, GRN, BLU
  { 112, 112, 112 },  //  0:STOCK_BLADE_COLOR_WHITE
  { 255,   0,   0 },  //  1:STOCK_BLADE_COLOR_RED
  { 255, 102,   0 },  //  2:STOCK_BLADE_COLOR_ORANGE
  { 152, 152,   0 },  //  3:STOCK_BLADE_COLOR_YELLOW
  {   0, 255,   0 },  //  4:STOCK_BLADE_COLOR_GREEN
  {   0, 152, 152 },  //  5:STOCK_BLADE_COLOR_CYAN
  {   0,   0, 255 },  //  6:STOCK_BLADE_COLOR_BLUE
  { 152,   0, 152 },  //  7:STOCK_BLADE_COLOR_PURPLE
  {  16,   0,  32 },  //  8:STOCK_BLADE_COLOR_DARK_PURPLE
  { 192,  32,   0 },  //  9:STOCK_BLADE_COLOR_ORANGE_RED 
  { 128, 128, 128 },  // 10:STOCK_BLADE_COLOR_FLASH_WHITE
  { 255, 255,   0 },  // 11:STOCK_BLADE_COLOR_FLASH_YELLOW
  { 255,  64,   0 }   // 12:STOCK_BLADE_COLOR_FLASH_ORANGE
};
uint8_t stock_blade_color_table_lookup[STOCK_BLADE_COLOR_TABLES][STOCK_BLADE_COLORS_PER_TABLE] = {
  {  // Savi's Workship Lightsabers Colors
    STOCK_BLADE_COLOR_WHITE,
    STOCK_BLADE_COLOR_RED,
    STOCK_BLADE_COLOR_ORANGE,
    STOCK_BLADE_COLOR_YELLOW,
    STOCK_BLADE_COLOR_GREEN,
    STOCK_BLADE_COLOR_CYAN,
    STOCK_BLADE_COLOR_BLUE,
    STOCK_BLADE_COLOR_PURPLE,
    STOCK_BLADE_COLOR_DARK_PURPLE,
    STOCK_BLADE_COLOR_RED,
    STOCK_BLADE_COLOR_ORANGE,
    STOCK_BLADE_COLOR_YELLOW,
    STOCK_BLADE_COLOR_GREEN,
    STOCK_BLADE_COLOR_CYAN,
    STOCK_BLADE_COLOR_BLUE,
    STOCK_BLADE_COLOR_PURPLE
  },
  {  // Savi's Workship Lightsabers CLASH Colors
    STOCK_BLADE_COLOR_FLASH_YELLOW, // white
    STOCK_BLADE_COLOR_FLASH_ORANGE, // red
    STOCK_BLADE_COLOR_FLASH_WHITE,  // orange
    STOCK_BLADE_COLOR_FLASH_WHITE,  // yellow
    STOCK_BLADE_COLOR_FLASH_YELLOW, // green, Ahsoka (CW)
    STOCK_BLADE_COLOR_FLASH_YELLOW, // cyan
    STOCK_BLADE_COLOR_FLASH_YELLOW, // blue
    STOCK_BLADE_COLOR_FLASH_YELLOW, // purple
    STOCK_BLADE_COLOR_FLASH_ORANGE, // dark purple
    STOCK_BLADE_COLOR_FLASH_ORANGE, // red
    STOCK_BLADE_COLOR_FLASH_ORANGE, // orange
    STOCK_BLADE_COLOR_FLASH_WHITE,  // yellow
    STOCK_BLADE_COLOR_FLASH_YELLOW, // green
    STOCK_BLADE_COLOR_FLASH_ORANGE, // cyan
    STOCK_BLADE_COLOR_FLASH_YELLOW, // blue
    STOCK_BLADE_COLOR_FLASH_YELLOW  // purple
  },
  {  // Legacy Lightsabers Colors
    STOCK_BLADE_COLOR_YELLOW,       //  0 Temple Guard
    STOCK_BLADE_COLOR_RED,          //  1 Kylo Ren
    STOCK_BLADE_COLOR_BLUE,         //  2 Rey/Anakin, Rey/Anakin Reforged, Ahsoka (CW)
    STOCK_BLADE_COLOR_PURPLE,       //  3 Mace
    STOCK_BLADE_COLOR_RED,          //  4 Ventress
    STOCK_BLADE_COLOR_WHITE,        //  5 Ahsoka (Rebels)
    STOCK_BLADE_COLOR_GREEN,        //  6 Luke
    STOCK_BLADE_COLOR_RED,          //  7 Vader
    STOCK_BLADE_COLOR_RED,          //  8 Maul
    STOCK_BLADE_COLOR_BLUE,         //  9 Obi-wan, Ben
    STOCK_BLADE_COLOR_ORANGE_RED,   // 10 Baylan Skoll, Shin Hati
    STOCK_BLADE_COLOR_WHITE,        // 11 unused
    STOCK_BLADE_COLOR_WHITE,        // 12 unused
    STOCK_BLADE_COLOR_WHITE,        // 13 unused
    STOCK_BLADE_COLOR_WHITE,        // 14 unused
    STOCK_BLADE_COLOR_WHITE         // 15 unused
  },
  {  // Legacy Lightsabers CLASH Colors
    STOCK_BLADE_COLOR_FLASH_WHITE,  // yellow
    STOCK_BLADE_COLOR_FLASH_ORANGE, // red
    STOCK_BLADE_COLOR_FLASH_YELLOW, // blue
    STOCK_BLADE_COLOR_FLASH_YELLOW, // purple
    STOCK_BLADE_COLOR_FLASH_ORANGE, // red
    STOCK_BLADE_COLOR_FLASH_YELLOW, // white
    STOCK_BLADE_COLOR_FLASH_YELLOW, // green
    STOCK_BLADE_COLOR_FLASH_ORANGE, // red
    STOCK_BLADE_COLOR_FLASH_ORANGE, // red
    STOCK_BLADE_COLOR_FLASH_YELLOW, // blue
    STOCK_BLADE_COLOR_FLASH_YELLOW, // orange
    STOCK_BLADE_COLOR_FLASH_YELLOW, // white
    STOCK_BLADE_COLOR_FLASH_YELLOW, // white
    STOCK_BLADE_COLOR_FLASH_YELLOW, // white
    STOCK_BLADE_COLOR_FLASH_YELLOW, // white
    STOCK_BLADE_COLOR_FLASH_YELLOW  // white
  }
};

// functions that manipulate the blade's color and brightness
// see blade_state.h for their purpose
void set_custom_segment_color(uint8_t segment, uint8_t red, uint8_t green, uint8_t blue) {
  //segment_color[segment][RED_IDX] = red;
  segment_color[segment][RED_IDX] = (uint8_t)((float)red * RED_ADJUST);
  segment_color[segment][GRN_IDX] = green;
  segment_color[segment][BLU_IDX] = blue;
}

void set_blade_custom_color(uint8_t red, uint8_t green, uint8_t blue) {
  uint8_t i;
  for (i=0;i<4;i++) {
    set_custom_segment_color(i, red, green, blue);
  }
}

void clear_segment_color(uint8_t segment) {
  uint8_t i;
  for (i=0;i<3;i++) {
    segment_color[segment][i] = 0;
  }
}

void clear_blade_color(void) {
  uint8_t i;
  for (i=0;i<4;i++) {
    clear_segment_color(i);
  }
}

void rotate_segment_color(uint8_t direction) {
  uint8_t tmp[3];

  // zero is a shift backwards
  if (direction == 0) {
    tmp[0] = segment_color[0][0];
    tmp[1] = segment_color[0][1];
    tmp[2] = segment_color[0][2];

    segment_color[0][0] = segment_color[1][0];
    segment_color[0][1] = segment_color[1][1];
    segment_color[0][2] = segment_color[1][2];

    segment_color[1][0] = segment_color[2][0];
    segment_color[1][1] = segment_color[2][1];
    segment_color[1][2] = segment_color[2][2];

    segment_color[2][0] = segment_color[3][0];
    segment_color[2][1] = segment_color[3][1];
    segment_color[2][2] = segment_color[3][2];

    segment_color[3][0] = tmp[0];
    segment_color[3][1] = tmp[1];
    segment_color[3][2] = tmp[2];

  // any other value is a shift forwards
  } else {
    tmp[0] = segment_color[3][0];
    tmp[1] = segment_color[3][1];
    tmp[2] = segment_color[3][2];

    segment_color[3][0] = segment_color[2][0];
    segment_color[3][1] = segment_color[2][1];
    segment_color[3][2] = segment_color[2][2];

    segment_color[2][0] = segment_color[1][0];
    segment_color[2][1] = segment_color[1][1];
    segment_color[2][2] = segment_color[1][2];

    segment_color[1][0] = segment_color[0][0];
    segment_color[1][1] = segment_color[0][1];
    segment_color[1][2] = segment_color[0][2];

    segment_color[0][0] = tmp[0];
    segment_color[0][1] = tmp[1];
    segment_color[0][2] = tmp[2];
  }
}

void set_segment_color_by_wheel(uint8_t segment, uint8_t wheel_value) {
  if (wheel_value < 85 ) {
    set_custom_segment_color(segment, ~(wheel_value * 3), (wheel_value * 3), 0);
  } else if (wheel_value < 170) {
    wheel_value -= 85;
    set_custom_segment_color(segment, 0, ~(wheel_value * 3), (wheel_value * 3));
  } else {
    wheel_value -= 170;
    set_custom_segment_color(segment, (wheel_value * 3), 0, ~(wheel_value * 3));
  }
}

void set_color_by_wheel(uint8_t color) {
  uint8_t i;

  // calculate color for first segment
  set_segment_color_by_wheel(0, color);

  // copy values from first segment to other 3 segments
  for (i=1;i<3;i++) {
    segment_color[i][RED_IDX] = segment_color[0][RED_IDX];
    segment_color[i][GRN_IDX] = segment_color[0][GRN_IDX];
    segment_color[i][BLU_IDX] = segment_color[0][BLU_IDX];
  }
}

void set_blade_color(void) {
  uint8_t color, color_table, i;
  color = blade.color_state & 0x0F;
  color_table = blade.color_state >> 4;

  for (i=0;i<BLADE_SEGMENTS;i++) {
    segment_color[i][RED_IDX] = stock_blade_colors[stock_blade_color_table_lookup[color_table][color]][RED_IDX];
    segment_color[i][GRN_IDX] = stock_blade_colors[stock_blade_color_table_lookup[color_table][color]][GRN_IDX];
    segment_color[i][BLU_IDX] = stock_blade_colors[stock_blade_color_table_lookup[color_table][color]][BLU_IDX];
  }
}

void set_segment_brightness(uint8_t segment, uint8_t amount) {

  // ensure segment is a value between 0 and BLADE_SEGMENTS.
  segment %= BLADE_SEGMENTS;

  // amount should be a percentage value from 0 to 100. 
  // ensure amount is not greater than 100
  if (amount > 100) {
    amount = 100;
  }

  // segment_brightness is an 8-bit integer with a max value of 255
  // calculate the amount percentage of 255
  segment_brightness[segment] = (uint8_t)(((float)amount/100) * 255);
}

void set_blade_brightness(uint8_t amount) {
  for (uint8_t i=0;i<4;i++) {
    set_segment_brightness(i, amount);
  }
}

void set_max_segment_brightness(uint8_t segment, uint8_t amount) {
  segment %= 4;
  if (amount > 100) {
    amount = 100;
  }
  max_segment_brightness[segment] = (uint8_t)(((float)amount/100) * 255);
}

void set_max_blade_brightness(uint8_t amount) {
  for (uint8_t i=0;i<4;i++) {
    set_max_segment_brightness(i, amount);
  }
}

void mem_segment_color(uint8_t operation) {
  static uint8_t backup_segment_color[BLADE_SEGMENTS][RGB_SIZE] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
  uint8_t s, c;

  if (operation == MEM_BLADE_BACKUP) {
    for (s=0;s<BLADE_SEGMENTS;s++) {
      for (c=0;c<RGB_SIZE;c++) {
        backup_segment_color[s][c] = segment_color[s][c];
      }
    }
  } else {
    for (s=0;s<BLADE_SEGMENTS;s++) {
      for (c=0;c<RGB_SIZE;c++) {
        segment_color[s][c] = backup_segment_color[s][c];
      }
    }
  }
}

void mem_segment_brightness(uint8_t operation) {
  static uint8_t backup_segment_brightness[BLADE_SEGMENTS] = {0,0,0,0};
  static uint8_t backup_max_segment_brightness[BLADE_SEGMENTS] = {0,0,0,0};
  uint8_t s;

  if (operation == MEM_BLADE_BACKUP) {
    for (s=0;s<BLADE_SEGMENTS;s++) {
      backup_segment_brightness[s] = segment_brightness[s];
      backup_max_segment_brightness[s] = max_segment_brightness[s];
    }
  } else {
    for (s=0;s<BLADE_SEGMENTS;s++) {
      segment_brightness[s] = backup_segment_brightness[s];
      max_segment_brightness[s] = backup_max_segment_brightness[s];
    }
  }
}

void mem_blade(uint8_t operation) {
  static uint8_t backup_blade_state = 0;
  static uint8_t backup_blade_color_state = 0;

  if (operation == MEM_BLADE_BACKUP) {
    backup_blade_state = blade.state;
    backup_blade_color_state = blade.color_state;
  } else {
    blade.state = backup_blade_state;
    blade.color_state = backup_blade_color_state;
  }

  // these two are broken into their own, separate functions as they may prove useful down the road.
  mem_segment_color(operation);
  mem_segment_brightness(operation);
}

void dump_segment_brightness(void) {
  uint8_t i;
  
  serial_sendString("segment_brightness = {");
  for(i=0;i<BLADE_SEGMENTS;i++) {
    snprintf(serial_buf, SERIAL_BUF_LEN, " %d", segment_brightness[i]);
    serial_sendString(serial_buf);
  }
  serial_sendString("}\r\nmax_segment_brightness = {");
  for(i=0;i<BLADE_SEGMENTS;i++) {
    snprintf(serial_buf, SERIAL_BUF_LEN, " %d", max_segment_brightness[i]);
    serial_sendString(serial_buf);
  }
  serial_sendString("}\r\ntrue_segment_brightness = {");
  for(i=0;i<BLADE_SEGMENTS;i++) {
    snprintf(serial_buf, SERIAL_BUF_LEN, " %d", true_segment_brightness[i]);
    serial_sendString(serial_buf);
  }
  serial_sendString(" }\r\n\r\n");
}

// dump contents of blade state struct to serial
void dump_blade_state(void) {
  serial_sendString("BLADE STATE:\r\n");
  
  serial_sendString("  blade.state: ");
  snprintf(serial_buf, SERIAL_BUF_LEN, "0x%02X\r\n", blade.state);
  serial_sendString(serial_buf);

  serial_sendString("  blade.color_state: ");
  snprintf(serial_buf, SERIAL_BUF_LEN, "0x%02X\r\n", blade.color_state);
  serial_sendString(serial_buf);

  serial_sendString("  blade.dmode: ");
  snprintf(serial_buf, SERIAL_BUF_LEN, "0x%02X\r\n", blade.dmode);
  serial_sendString(serial_buf);

  serial_sendString("  blade.dsubmode: ");
  snprintf(serial_buf, SERIAL_BUF_LEN, "0x%02X\r\n", blade.dsubmode);
  serial_sendString(serial_buf);

  serial_sendString("  blade.dmode_step: ");
  snprintf(serial_buf, SERIAL_BUF_LEN, "0x%02X\r\n", blade.dmode_step);
  serial_sendString(serial_buf);

  serial_sendString("\r\n");
}
