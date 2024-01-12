/* dmode_handler.c
 *
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "millis.h"
#include "serial.h"
#include "blade_state.h"
#include "dmode_handler.h"
#include "pwm.h"

// multi-color blade presets
uint8_t blade_multi_colors[][BLADE_SEGMENTS][RGB_SIZE] = {
  // outlandish
  {{  0, 255,   0}, {  0,   0, 255}, {255, 255, 255}, {255,   0,   0}}, // rocket popsicle

  // subtle
  {{255,   0,   0}, {224,  16,   0}, {224,  32,   0}, {192,  64,   0}}, // red-to-orange
  {{192, 144,   0}, {128, 176,   0}, { 64, 224,   0}, {  0, 255,   0}}, // orange-to-green

  // light-to-dark *
  {{112,   0, 224}, { 72,   0, 144}, { 48,   0,  96}, { 32,   0,  64}}, // light-to-dark purple
  {{224, 112,   0}, { 144, 72,   0}, { 96,  48,   0}, { 64,  32,   0}}, // light-to-dark orange

  // *-to-white
  {{  0, 255,   0}, { 32, 224,  32}, { 64, 192,  64}, { 96, 128,  96}}, // green-to-white
  {{255,   0,   0}, {224,  32,  32}, {192,  64,  64}, {128,  96,  96}}, // red-to-white
  {{  0,   0, 255}, { 32,  32, 224}, { 64,  64, 192}, { 96,  96, 128}}, // blue-to-white

  // misc
  {{192,  64,   0}, {128,  48,  32}, { 96,  16,  64}, { 48,   0,  96}}, // orange-to-purple
};
uint8_t blade_multi_colors_len = sizeof(blade_multi_colors) / sizeof(blade_multi_colors[0]);

// color picker step lookup table 
uint8_t dcp_step_table[DCP_STEP_TABLE_MAX];
uint8_t dcp_max_steps = 0;

// dump the contents of dcp_step_table[] to aid in validation/debugging
void dcp_step_table_dump(void) {
  uint8_t i;

  // dump all the macros/define values used for DCP step table calculations
  serial_sendString("\r\n");
  snprintf(serial_buf, SERIAL_BUF_LEN, "DCP_BRIGHTNESS_LEVELS: %d\r\n", DCP_BRIGHTNESS_LEVELS);
  serial_sendString(serial_buf);
  snprintf(serial_buf, SERIAL_BUF_LEN, "DCP_COLOR_COUNT:       %d\r\n", DCP_COLOR_COUNT);
  serial_sendString(serial_buf);
  snprintf(serial_buf, SERIAL_BUF_LEN, "DCP_FORMULA_SEPARATOR: %d\r\n", DCP_FORMULA_SEPARATOR);
  serial_sendString(serial_buf);
  snprintf(serial_buf, SERIAL_BUF_LEN, "DCP_MIDDLE_LEVEL:      %d\r\n", DCP_MIDDLE_LEVEL);
  serial_sendString(serial_buf);
  snprintf(serial_buf, SERIAL_BUF_LEN, "DCP_MAX_STEP:          %d\r\n", DCP_MAX_STEP);
  serial_sendString(serial_buf);

  // dump the step table contents
  serial_sendString("\r\n");
  serial_sendString("DCP_STEP_TABLE[] Contents:\r\n");
  for (i=0;i<dcp_max_steps;i++) {
    snprintf(serial_buf, SERIAL_BUF_LEN, "  dsubmode: %d, dcp_step %d\r\n", i, dcp_step_table[i]);
    serial_sendString(serial_buf);
  }
  serial_sendString("\r\n");
}

// precalculate the size of steps to take between colors when cycling through
// colors in color picker mode for a given value of blade.dsubmode
void precalc_dcp_step_table(void) {
  uint8_t prev_step = 255;
  uint8_t dsubmode_index = 0;
  uint8_t step;

  while(1) {
    step = DCP_FORMULA_SEPARATOR / (2 + dsubmode_index);
    if (step != prev_step) {
      dcp_step_table[dcp_max_steps++] = step;
      prev_step = step;
    }
    if (step == 1 || dcp_max_steps >= DCP_STEP_TABLE_MAX) {
      break;
    }
    dsubmode_index++;
  }

  #ifdef DEBUG_SERIAL_ENABLED
    dcp_step_table_dump();
  #endif
}

// similar to set_segment_color_by_wheel with the extra feature of
// generating not just color, but levels of brightness based on BRIGHTNESS_LEVELS
void set_dcp_segment_color(uint8_t segment, uint8_t wheel_value) {

  uint8_t red, green, blue, color, brightness;

  // special case, set blade to WHITE
  if (wheel_value == 255) {
    red   = stock_blade_colors[STOCK_BLADE_COLOR_WHITE][RED_IDX];
    green = stock_blade_colors[STOCK_BLADE_COLOR_WHITE][GRN_IDX];
    blue  = stock_blade_colors[STOCK_BLADE_COLOR_WHITE][BLU_IDX];
    color = 255;
    brightness = DCP_BRIGHTNESS_LEVELS;

  // else, calculate color like normal
  } else {

    // separate color and brightness values from the supplied wheel value
    color = wheel_value % DCP_COLOR_COUNT;
    brightness = (uint8_t)(wheel_value / DCP_COLOR_COUNT);

    // calculate the red, green, and blue components of the color based on color value.
    //
    // this formula is based on the wheel() function found here:
    // https://learn.adafruit.com/multi-tasking-the-arduino-part-3/utility-functions
    if (color < DCP_FORMULA_SEPARATOR) {
      green = color * 3 * DCP_BRIGHTNESS_LEVELS;
      red = ~green;
      blue = 0;
    } else if (color < (2 * DCP_FORMULA_SEPARATOR)) {
      color -= DCP_FORMULA_SEPARATOR;
      blue = color * 3 * DCP_BRIGHTNESS_LEVELS;
      green = ~blue;
      red = 0;
    } else {
      color -= (2 * DCP_FORMULA_SEPARATOR);
      red = color * 3 * DCP_BRIGHTNESS_LEVELS;
      blue = ~red;
      green = 0;
    }

    // no need to bother with this if DCP_BRIGHTNESS_LEVELS is set to 1
    #if (DCP_BRIGHTNESS_LEVELS > 1)

    // add an amount of white to the color based on the brightness level. use the stock white blade color as a reference.
    if (brightness > DCP_MIDDLE_LEVEL) {
      red   -= (uint8_t)((red   - stock_blade_colors[STOCK_BLADE_COLOR_WHITE][RED_IDX]) * pow((float)brightness / DCP_BRIGHTNESS_LEVELS, 2));
      green -= (uint8_t)((green - stock_blade_colors[STOCK_BLADE_COLOR_WHITE][GRN_IDX]) * pow((float)brightness / DCP_BRIGHTNESS_LEVELS, 2));
      blue  -= (uint8_t)((blue  - stock_blade_colors[STOCK_BLADE_COLOR_WHITE][BLU_IDX]) * pow((float)brightness / DCP_BRIGHTNESS_LEVELS, 2));

    // darken the color if brightness < middle_brightness_level
    } else if (brightness < DCP_MIDDLE_LEVEL) {
      red   = (uint8_t)(red   * pow((float)(brightness + 1)/(DCP_MIDDLE_LEVEL + 1), 2));
      green = (uint8_t)(green * pow((float)(brightness + 1)/(DCP_MIDDLE_LEVEL + 1), 2));
      blue  = (uint8_t)(blue  * pow((float)(brightness + 1)/(DCP_MIDDLE_LEVEL + 1), 2));
    }
    #endif
  }

  // set the segment's color using the component values that have been calculated
  set_custom_segment_color(segment, red, green, blue);

  #ifdef DEBUG_SERIAL_ENABLED
  if (segment == 0) {
    snprintf(serial_buf, SERIAL_BUF_LEN, "  [wheel:%3d, color:%3d, brightness:%d] R:%3d G:%3d B:%3d\r\n", wheel_value, color, brightness, red, green, blue);
    serial_sendString(serial_buf);
  }
  #endif
}

// similar to set_color_by_wheel except it takes BRIGHTNESS_LEVELS into consideration
void set_dcp_color(uint8_t color) {
  uint8_t i;

  // calculate color for first segment
  set_dcp_segment_color(0, color);
  
  // copy values from first segment to other 3 segments
  for (i=1;i<3;i++) {
    segment_color[i][RED_IDX] = segment_color[0][RED_IDX];
    segment_color[i][GRN_IDX] = segment_color[0][GRN_IDX];
    segment_color[i][BLU_IDX] = segment_color[0][BLU_IDX];
  }
}

void dmode_handler(void) {
  static uint8_t last_dmode = 0;
  static uint8_t last_dsubmode = 0;
  static uint8_t last_blade_state = 0;
  static uint32_t next_step_time = 0;
  static uint8_t step_backup = DCP_MIDDLE_LEVEL * DCP_COLOR_COUNT;
  uint8_t i;
  uint8_t dcp_step;
  uint8_t dcp_color_value;
  uint8_t dcp_formula_value;
  uint8_t dcp_brightness = 0;

  // one-time initialization of pdcp-step_table[];
  // probably should call this from some other function (setup?), but as it's just for dmode and
  // i want to keep dmode stuff together, it's here.
  if (dcp_max_steps == 0) {
    precalc_dcp_step_table();
  }

  // check for state in blade changes
  if (last_blade_state != blade.state) {

    // coming out of a clash
    if ((last_blade_state & 0xF0) == BLADE_STATE_CLASH && (blade.state & 0xF0) == BLADE_STATE_ON) {

      // which dmode are we in?
      switch (blade.dmode) {

        // color picker mode; allow clash to trigger manual color change
        case DMODE_COLOR_PICKER:
          next_step_time = millis() + 4000;      // pause auto-picker briefly after brightness change

          // stepping out of white, restore stored dmode_step value
          if (blade.dmode_step == 255) {
            blade.dmode_step = step_backup;

            #ifdef DEBUG_SERIAL_ENABLED
            snprintf(serial_buf, SERIAL_BUF_LEN, "WHITE mode ended! restored: %3d\r\n", step_backup);
            serial_sendString(serial_buf);
            #endif

            step_backup = DCP_MIDDLE_LEVEL * DCP_COLOR_COUNT;
          } else {

            // increment brightness level
            blade.dmode_step += DCP_COLOR_COUNT;

            // did we go past full brightness
            if (blade.dmode_step >= (DCP_COLOR_COUNT * DCP_BRIGHTNESS_LEVELS) || (blade.dmode_step < DCP_COLOR_COUNT)) {
              blade.dmode_step += 256 - (DCP_COLOR_COUNT * DCP_BRIGHTNESS_LEVELS);
              step_backup = blade.dmode_step;

              #ifdef DEBUG_SERIAL_ENABLED
              snprintf(serial_buf, SERIAL_BUF_LEN, "WHITE mode enabled! backup: %3d\r\n", step_backup);
              serial_sendString(serial_buf);
              #endif

              blade.dmode_step = 255;
            }
          }
          set_dcp_color(blade.dmode_step);  // display new color
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
      snprintf(serial_buf, SERIAL_BUF_LEN, "New DMODE detected: 0x%02X\r\n", blade.dmode);
      serial_sendString(serial_buf);
    #endif

    // record new dmode
    last_dmode = blade.dmode;

    // do not reset dmode_step if new dmode is color_picked as dmode_step contains the color that was picked
    if (blade.dmode != DMODE_COLOR_PICKER_PICKED) {
      blade.dmode_step = 0;  // reset dmode step
    }
    last_dsubmode = blade.dsubmode - 1;  // force new dsubmode code to execute by changing value of last_dsubmode

    // if blade state has just been loaded from eeprom
    if (state_loaded_from_eeprom != 0) {

      // perform actions to set blade into correct state for the given mode
      switch (blade.dmode) {
        case DMODE_COLOR_PICKER_PICKED:
          set_dcp_color(blade.dmode_step);  // set the blade color
          break;
      }

      state_loaded_from_eeprom = 0; // clear flag
    }

    // perform one-time initialization unique for the new dmode
    switch (blade.dmode) {
      case DMODE_SEGMENT_WHEEL:
        blade.dmode_step = (rand() % 16) * 17;  // pick a random starting color

      case DMODE_MULTI_MODE:
        set_multi_mode();
        break;

      case DMODE_BLADE_WHEEL:
      case DMODE_COLOR_PICKER:
        blade.dmode_step = DCP_MIDDLE_LEVEL * DCP_COLOR_COUNT;

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

      // step the blade through the color wheel
      case DMODE_COLOR_PICKER:

        /* The Color Picker: An Overly-Complicated Thing or My Decent Into Madness
         * A story by Ruthsarian
                 *
         * \\\ There MUST be a much simpler, more logical, all around BETTER way to achieve this
         * /// effect, but I can't figure it out. I think I've painted myself into a corner and
         * \\\ and can't escape. Anyways...
         * 
         * The color picker value is stored in the variable blade.dmode_step, an unsigned 
         * 8-bit value (0-255), while the blade's dmode state is DMODE_COLOR_PICKER or 
         * DMODE_COLOR_PICKER_PICKED.
         * 
         * The color picker supports multiple levels of brightness. How many levels
         * is determined by the define DCP_BRIGHTNESS_LEVELS. 
         *
         * "Brightness" in this context refers to adjusting the color towards or away from
         * the color white. It does not increase the luminance of the blade. This makes it
         * possible for the color picker to produce pastel colors as well as dim colors.
         *
         * The number of possible color values per level of brightness is calculated at compile time by 
         * the #define DCP_COLOR_COUNT. As the color picker value is an 8-bit value, 
         * DCP_COLOR_COUNT is calculated by dividing DCP_MAX_COLORSPACE by DCP_BRIGHTNESS_LEVELS.
         * 
         * The color picker begins at middle brightness (colors that are not dim and not pastel).
         * Brightness level can be changed by triggering a clash while the color picker is active.
         *
         * The size of the step taken during color picking (thus the color 'resolution') is controlled by 
         * the value of blade.dsubmode which increments with each long off/on.
         *
         * To simulate colors generated by kyber crystals, the code needs to make sure that as we
         * step through colors we land on pure red, pure green, pure blue. Purple and yellow colors are
         * covered by the first level of color resolution, at middle brightness; the configuration of
         * the color picker when it first starts.
         * 
         * In order to ensure that the color picker lands on blue, green, and red, the code needs to
         * be aware the formulas being used to generate color values so that it can adjust the value
         * of dmode_step as needed to land on red, green, and blue.
         *
         * These formulas are found in set_segment_color_by_wheel_with_brightness() which is located
         * in blade_state.c. Changes to those formulas may require updates to code here. 
         * (TODO) Perhaps that entire function should be moved into dmode_handler.c?
         *
         * In the variable names and comments in the code below, I make reference to 'color' and 'formula'
         * the meaning of which I need to explain first.
         *
         * The variable blade.dmode_step contains both color and brightness level. The 'color' value
         * represents the value used to generate the red, green, and blue components prior to adjusting
         * them to the current brightness level. This will be a value between 0 and DCP_COLOR_COUNT.
         * 
         * When calculating the RGB components of the color, each component has a separate formula that is 
         * applied to the color value to determine that component's value. When referring to the 'formula'
         * I am referring to the formula used to calculate a single component of the three RGB components.
         * Thus there are 3 'formulas'.
         *
         * 'formula' is taken into account to identify the point at which the color value would be pure
         * red, pure green, or pure blue. that way blade.dmode_step can be adjusted to land exactly at
         * that color when it would otherwise skip past it.
         * (TODO) Perhaps I need to replace 'formula' with 'component' in the comments and variable names.
         *
         * About White
         * With this setup, the max brightness level is all white. That's DCP_COLOR_COUNT worth of color space
         * lost to white. If we found some other way to handle white we could regain that space and offer
         * more colors with more brightness levels!
         *
         * How?
         *  - separate 'white' flag?
         *    then we have to store more than one 8-bit value in order to reproduce white the next time the
         *    blade is ignited.
         *
         *  - 255 = white, 0-254 = colors
         *    could work, except we lose the color we were previously on, so if we're brightness-changing through
         *    to dark for a color that you like, we lose that color, so it won't work.
         *
         *    unless we store the previous value prior to turning white somewhere temporarily and pull it back in 
         *    when we get out of white. We care about remembering that value during the color picking process, but
         *    not after. we'd have to store it in a variable that persists through function calls (a local static
         *    or global variable).
         *
         *    also need a way to tell the code what the max colorspace is (255 instead of 256 (0-255)) if we use
         *    255 to represent white. then we have to add support for this in the color setting function as well.
         *    this also means white will always exist as a brightness level. so even if brightness_levels is set to 1
         *    we really have 2 levels to work with.
         *
         */

        // only step to next color if blade is in an on state.
        // this prevents color-stepping during ignition and extinguish
        // (TODO) make sure blade.dmode_step can only reach 255 when swapping to white!
        if ((blade.state & 0xF0) == BLADE_STATE_ON && blade.dmode_step != 255) {

          // dcp_step value is based on value of blade.dsubmode which increments with a long off/on operation
          // as dsubmode increases, step will decrease, allowing for more colors to be selected
          //
          // these step values are pre-calculated and stored in dcp_step_table[] to make life easier on my 
          // very weak brain.
          dcp_step = dcp_step_table[blade.dsubmode % dcp_max_steps];

          // record current brightness level before proceeding. will use this to determine if we've
          // shot past the current brightness level and correct for that at the end
          dcp_brightness = (uint8_t)(blade.dmode_step / DCP_COLOR_COUNT);

          // increment to the next step of the color wheel
          blade.dmode_step += dcp_step;

          // determine the current color value that will be used when setting blade color
          dcp_color_value = blade.dmode_step % DCP_COLOR_COUNT;

          // derive the current formula value from the color value          
          dcp_formula_value = dcp_color_value % DCP_FORMULA_SEPARATOR;

          // if too close to next color formula, push dmode_step to next color formula
          if (DCP_FORMULA_SEPARATOR - dcp_formula_value <= (uint8_t)(dcp_step / 2)) {
            blade.dmode_step += DCP_FORMULA_SEPARATOR - dcp_formula_value;
            
            // recalculate dcp_color_value and dcp_formula_value since dmode_step has changed
            dcp_color_value = blade.dmode_step % DCP_COLOR_COUNT;
            dcp_formula_value = dcp_color_value % DCP_FORMULA_SEPARATOR;
          }

          // if too close to end of color space, reset to start of color space
          //
          // color space 'width' is being calculated by (DCP_FORMULA_SEPARATOR * 3) instead
          // of DCP_COLOR_COUNT because if DCP_COLOR_COUNT is not cleanly divisible
          // by 3 (R,G,B) we have as much a value of DCP_COLOR_COUNT that could be as much as 
          // 2 beyond the max color space, which would affects how we detect being near the end of the 
          // color space
          if (dcp_color_value >= ((DCP_FORMULA_SEPARATOR * 3) - ((uint8_t)(dcp_step / 2)))) {
            blade.dmode_step += (DCP_COLOR_COUNT - dcp_color_value);

          // if an increment has overshot to the next color formula, move back to start of next color formula
          } else if ((dcp_formula_value > 0) && (dcp_formula_value < dcp_step)) {
            blade.dmode_step -= dcp_formula_value;
          }

          // did incrementing dmode_step push us into the next brightness level? 
          if (blade.dmode_step != (dcp_brightness * DCP_COLOR_COUNT) && (blade.dmode_step % DCP_COLOR_COUNT) < dcp_step) {

            // then reduce by 1 brightness level so we maintain the same brightness
            blade.dmode_step -= DCP_COLOR_COUNT; 
          }
        }

        set_dcp_color(blade.dmode_step);
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
