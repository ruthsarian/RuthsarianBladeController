/* blade_state.h
 *
 * define global variables relating to blade state
 * define functions that can be used to manipulate these global variables
 */

#ifndef BLADE_STATE_H_
#define BLADE_STATE_H_

// BLADE STATE
#define BLADE_STATE_OFF         0x00
#define BLADE_STATE_POWER_ON    0x10
#define BLADE_STATE_ON          0x20
#define BLADE_STATE_CLASH       0x30
#define BLADE_STATE_POWER_OFF   0x40
#define BLADE_STATE_REDFLICKER  0x50
#define BLADE_STATE_RESET       0xF0

// how many segments in a blade
#define BLADE_SEGMENTS	4

// BLADE COLOR

// blade color table R, G, B index values
#define RED_IDX	0
#define GRN_IDX	1
#define BLU_IDX	2

// how many values per RGB color
#define RGB_SIZE	3

// stock blade color lookup table details
#define STOCK_BLADE_COLORS_PER_TABLE	16	// There are 16 (4-bits) colors per color table
#define STOCK_BLADE_COLOR_TABLES		4	// number of stock blade color tables
#define STOCK_BLADE_COLOR_TABLE_SAVI	0	// Stock blade color table index value for Savi's Workshop lightsabers
#define STOCK_BLADE_COLOR_TABLE_LEGACY	2	// Stock blade color table index value for Legacy lightsabers

// blade color lookup table index values
#define STOCK_BLADE_COLOR_WHITE			0
#define STOCK_BLADE_COLOR_RED			1
#define STOCK_BLADE_COLOR_ORANGE		2
#define STOCK_BLADE_COLOR_YELLOW		3
#define STOCK_BLADE_COLOR_GREEN			4
#define STOCK_BLADE_COLOR_CYAN			5
#define STOCK_BLADE_COLOR_BLUE			6
#define STOCK_BLADE_COLOR_PURPLE		7
#define STOCK_BLADE_COLOR_DARK_PURPLE	8
#define STOCK_BLADE_COLOR_FLASH_WHITE	9
#define STOCK_BLADE_COLOR_FLASH_YELLOW	10
#define STOCK_BLADE_COLOR_FLASH_ORANGE	11
#define STOCK_BLADE_COLOR_LEN			12	// total number of colors supported by STOCK blades

// Display Modes (DMODE)
#define DMODE_STOCK						0
#define DMODE_COLOR_PICKER				1
#define DMODE_COLOR_PICKER_PICKED		2
#define DMODE_BLADE_WHEEL				3
#define DMODE_SEGMENT_WHEEL				4
#define DMODE_MULTI_MODE				5
#define DMODE_MAX						6	// maximum dmode value; once reached, dmode will reset to 0

// Display Sub-Modes (DSUBMODE)
#define DSUBMODE_NORMAL					0
#define DSUBMODE_FLICKER_FULL			1
#define DSUBMODE_FLICKER_SEGMENTED		2
#define DSUBMODE_FLICKER_BRIGHT			3
#define DSUBMODE_FLICKER_DARK			4
#define DSUBMODE_FLICKER_GRADIENT		5
#define DSUBMODE_BREATHING				6
#define DSUBMODE_BRIGHTNESS_66			7
#define DSUBMODE_BRIGHTNESS_33			8
#define DSUBMODE_BRIGHTNESS_10			9
#define DSUBMODE_MAX					10	// a cheap way to keep track of how many dsubmodes there are

// DMODE Timing Elements
#define DMODE_THRESHOLD_TIME			1000	// remain powered off for less than this value in milliseconds to increment display mode (DMODE)
#define DSUBMODE_THRESHOLD_TIME			3000	// remain powered off for less than this value in milliseconds, but more than DSUBMODE_THRESHOLD_TIME, to increment display sub-mode (DSUBMODE)

// MEM Operation
#define MEM_BLADE_BACKUP				0
#define MEM_BLADE_RESTORE				1

// RESET Configuration
#define RESET_THRESHOLD_COUNT			((DMODE_MAX*2)+1)	// how many on/off cycles before a reset is triggered
#define RESET_THRESHOLD_TIME			750					// maximum time blade must have been on/off in order to trigger a reset
#define RESET_STATE_PERIOD				5000				// how long to remain in a reset state

#ifdef __cplusplus
extern "C" {
#endif

// a struct that contains the blade state varaibles
struct blade_state_struct {
	uint8_t state;              // HIGH nibble = state, LOW nibble = state step counter or other variable for use with the state
	uint8_t color_state;        // HIGH nibble = color lookup table OR multi-color flag, LOW nibble = color
	uint8_t dmode;              // display mode
	uint8_t dsubmode;           // display submode
	uint8_t dmode_step;         // multipurpose variable, usage depends on the current dmode
};

// GLOBAL: blade - the current state of the blade
extern struct blade_state_struct blade;

// flag that tells the blade the blade state has been loaded from eeprom
extern uint8_t state_loaded_from_eeprom;

// GLOBAL: segment_color[4][3] - keep track of the blade segments' color
extern uint8_t segment_color[BLADE_SEGMENTS][RGB_SIZE];

// GLOBAL: segment_brightness[4] - keep track of the blade segments' brightness
extern uint8_t segment_brightness[BLADE_SEGMENTS];

// GLOBAL: stock_blade_colors[9][3] - blade color lookup table
//         this table is used to lookup RGB color values for specific colors produced by STOCK blades
//         value/255 = PWM duty cycle needed to produce the color
extern uint8_t stock_blade_colors[STOCK_BLADE_COLOR_LEN][RGB_SIZE];

// GLOBAL: blade_color_table_lookup - this 2D table contains color lookup values for stock Galaxy's Edge lightsabers
//         each color has a corresponding CLASH color, used to produce a brief flash when the blade hits another blade
//         the organization of this table is every EVENT number table contains the normal color value and every odd value table
//         contains the clash color values of its preceding normal color table.
extern uint8_t stock_blade_color_table_lookup[STOCK_BLADE_COLOR_TABLES][STOCK_BLADE_COLORS_PER_TABLE];

// GLOBAL: multi-color blade presets
extern uint8_t blade_multi_colors[][BLADE_SEGMENTS][RGB_SIZE];

// GLOBAL: keep track of how many multi-color blade presets there are
extern uint8_t blade_multi_colors_len;

// set a custom color value for a specific segment
void set_custom_segment_color(uint8_t segment, uint8_t red, uint8_t green, uint8_t blue);

// set a custom color value for the entire blade
void set_blade_custom_color(uint8_t red, uint8_t green, uint8_t blue);

// remove the color values for the specified segment
void clear_segment_color(uint8_t segment);

// remove color values from all segments
void clear_blade_color(void);

// shift the colors of the individual segments by 1
//	direction = 0: shift backwards
// direction != 0: shift forwards
void rotate_segment_color(uint8_t direction);

// set RGB color values based on a single byte
// design of this function is such that incrementing the value
// will produce a smooth transition through the color wheel
void set_segment_color_by_wheel(uint8_t segment, uint8_t wheel_value);

// set the blade's color based on a single byte value
void set_color_by_wheel(uint8_t color);

// set the blade color using the stock color lookup tables and the value stored 
// in the global blade_state variable
void set_blade_color(void);

// set segment brightness by percent
void set_segment_brightness(uint8_t segment, uint8_t amount);

// set blade brightness by percent
void set_blade_brightness(uint8_t amount);

// backup/restore the blade's color from memory (NOT EEPROM)
//	operation = 0: backup current blade state
// operation != 0: restore current blade state
void mem_segment_color(uint8_t operation);

// backup/restore the blade's brightness from memory (NOT EEPROM)
//	operation = 0: backup current blade state
// operation != 0: restore current blade state
void mem_segment_brightness(uint8_t operation);

// use to create or restore a backup of the blade's state
//	operation = 0: backup current blade state
// operation != 0: restore current blade state
void mem_blade(uint8_t operation);

// dump the contents of blade struct to serial
void dump_blade_state(void);

// process any new command from the hilt and adjust the state of the blade as needed.
void command_handler(void);

// manage the blade if it's in a state that requires animation, such as power-on, power-off, clash
void animate_handler(void);

// manage custom display modes and animations
void dmode_handler(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* BLADE_STATE_H_ */