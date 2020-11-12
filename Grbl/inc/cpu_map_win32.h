/*
  cpu_map.h - CPU and pin mapping configuration file
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

/* The cpu_map.h files serve as a central pin mapping selection file for different
   processor types or alternative pin layouts.
   This version of Grbl supports only WIN32
*/


#ifndef cpu_map_win32_h
#define cpu_map_win32_h

  // Define serial port pins and interrupt vectors.
  #ifdef CPU_MAP_WIN32
    // Define step pulse output pins. NOTE: All step bit pins must be on the same port.
    #define STEP_DDR        DDRD
    #define STEP_PORT       PORTD
    #define X_STEP_BIT      2  
    #define Y_STEP_BIT      3  
    #define Z_STEP_BIT      4  
    #define STEP_MASK       ((1<<X_STEP_BIT)|(1<<Y_STEP_BIT)|(1<<Z_STEP_BIT)) // All step bits

    // Define step direction output pins. NOTE: All direction pins must be on the same port.
    #define DIRECTION_DDR     DDRD
    #define DIRECTION_PORT    PORTD
    #define X_DIRECTION_BIT   5  
    #define Y_DIRECTION_BIT   6  
    #define Z_DIRECTION_BIT   7  
    #define DIRECTION_MASK    ((1<<X_DIRECTION_BIT)|(1<<Y_DIRECTION_BIT)|(1<<Z_DIRECTION_BIT)) // All direction bits

    // Define stepper driver enable/disable output pin.
    #define SetStepperDisableBit() 
    #define ResetStepperDisableBit() 


    // Define homing/hard limit switch input pins and limit interrupt vectors. 
    // NOTE: All limit bit pins must be on the same port, but not on a port with other input pins (CONTROL).
    #define LIMIT_DDR        DDRB
    #define LIMIT_PIN        PINB
    #define LIMIT_PORT       PORTB
    #define X_LIMIT_BIT      1  
    #define Y_LIMIT_BIT      2  
    #ifdef VARIABLE_SPINDLE // Z Limit pin and spindle enabled swapped to access hardware PWM on Pin 11.  
    #define Z_LIMIT_BIT	   4 
    #else
    #define Z_LIMIT_BIT    3  
    #endif
    #define LIMIT_MASK       ((1<<X_LIMIT_BIT)|(1<<Y_LIMIT_BIT)|(1<<Z_LIMIT_BIT)) // All limit bits
    #define LIMIT_INT        PCIE0  // Pin change interrupt enable pin
    #define LIMIT_INT_vect   PCINT0_vect 
    #define LIMIT_PCMSK      PCMSK0 // Pin change interrupt register

    // Define spindle enable and spindle direction output pins.
    #define SPINDLE_ENABLE_DDR    DDRB
    #define SPINDLE_ENABLE_PORT   PORTB
    // Z Limit pin and spindle PWM/enable pin swapped to access hardware PWM on Pin 11.
    #ifdef VARIABLE_SPINDLE 
    #ifdef USE_SPINDLE_DIR_AS_ENABLE_PIN
    // If enabled, spindle direction pin now used as spindle enable, while PWM remains on D11.
    #define SPINDLE_ENABLE_BIT    5  // Uno Digital Pin 13 (NOTE: D13 can't be pulled-high input due to LED.)
    #else
    #define SPINDLE_ENABLE_BIT    3  // Uno Digital Pin 11
    #endif
    #else
    #define SPINDLE_ENABLE_BIT    4  // Uno Digital Pin 12
    #endif
    #ifndef USE_SPINDLE_DIR_AS_ENABLE_PIN
    #define SPINDLE_DIRECTION_DDR   DDRB
    #define SPINDLE_DIRECTION_PORT  PORTB
    #define SPINDLE_DIRECTION_BIT   5  // Uno Digital Pin 13 (NOTE: D13 can't be pulled-high input due to LED.)
    #endif

    // Define flood and mist coolant enable output pins.
    // NOTE: Uno analog pins 4 and 5 are reserved for an i2c interface, and may be installed at
    // a later date if flash and memory space allows.
    #define COOLANT_FLOOD_DDR   DDRC
    #define COOLANT_FLOOD_PORT  PORTC
    #define COOLANT_FLOOD_BIT   3  // Uno Analog Pin 3
    #ifdef ENABLE_M7 // Mist coolant disabled by default. See config.h to enable/disable.
      #define COOLANT_MIST_DDR   DDRC
      #define COOLANT_MIST_PORT  PORTC
      #define COOLANT_MIST_BIT   4 // Uno Analog Pin 4
    #endif  

    // Define user-control controls (cycle start, reset, feed hold) input pins.
    // NOTE: All CONTROLs pins must be on the same port and not on a port with other input pins (limits).
    #define CONTROL_DDR       DDRC
    #define CONTROL_PIN       PINC
    #define CONTROL_PORT      PORTC
    #define CONTROL_RESET_BIT         0  // MEGA2560 Analog Pin 8
    #define CONTROL_FEED_HOLD_BIT     1  // MEGA2560 Analog Pin 9
    #define CONTROL_CYCLE_START_BIT   2  // MEGA2560 Analog Pin 10
    #define CONTROL_SAFETY_DOOR_BIT   3  // MEGA2560 Analog Pin 11
    #define CONTROL_INT       PCIE2  // Pin change interrupt enable pin
    #define CONTROL_INT_vect  PCINT2_vect
    #define CONTROL_PCMSK     PCMSK2 // Pin change interrupt register
    #define CONTROL_MASK      ((1<<CONTROL_RESET_BIT)|(1<<CONTROL_FEED_HOLD_BIT)|(1<<CONTROL_CYCLE_START_BIT)|(1<<CONTROL_SAFETY_DOOR_BIT))

    // Define probe switch input pin.
    #define PROBE_DDR       DDRC
    #define PROBE_PIN       PINC
    #define PROBE_PORT      PORTC
    #define PROBE_BIT       5  // Uno Analog Pin 5
    #define PROBE_MASK      (1<<PROBE_BIT)

    // Start of PWM & Stepper Enabled Spindle
    #ifdef VARIABLE_SPINDLE
      // Advanced Configuration Below You should not need to touch these variables
      #define PWM_MAX_VALUE    255.0
      #define TCCRA_REGISTER	 TCCR2A
      #define TCCRB_REGISTER	 TCCR2B
      #define OCR_REGISTER     OCR2A

      #define COMB_BIT	     COM2A1
      #define WAVE0_REGISTER	 WGM20
      #define WAVE1_REGISTER	 WGM21
      #define WAVE2_REGISTER	 WGM22
      #define WAVE3_REGISTER	 WGM23

        // NOTE: On the 328p, these must be the same as the SPINDLE_ENABLE settings.
      #define SPINDLE_PWM_DDR	  DDRB
      #define SPINDLE_PWM_PORT  PORTB
      #define SPINDLE_PWM_BIT	  3    // Uno Digital Pin 11
    #endif // End of VARIABLE_SPINDLE

    #define SPINDLE_PWM_MAX_VALUE     255 // Don't change. 328p fast PWM mode fixes top value as 255.
    #ifndef SPINDLE_PWM_MIN_VALUE
      #define SPINDLE_PWM_MIN_VALUE   1   // Must be greater than zero.
    #endif
    #define SPINDLE_PWM_OFF_VALUE     0
    #define SPINDLE_PWM_RANGE         (SPINDLE_PWM_MAX_VALUE-SPINDLE_PWM_MIN_VALUE)

    #define SetSpindleEnablebit()         // Set pin to high
    #define ResetSpindleEnablebit()      // Set pin to low
    #define SetSpindleDirectionBit()
    #define ResetSpindleDirectionBit()

  #endif

#endif