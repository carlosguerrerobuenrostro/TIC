#ifndef BOARD_H
#define BOARD_H

// TICC Time interval Counter based on TICC Shield using TDC7200
//
// Copyright John Ackermann N8UR 2016
// Portions Copyright George Byrkit K9TRV 2016
// Portions Copyright Jeremy McDermond NH6Z 2016
// Licensed under BSD 2-clause license


const int STOP_A =      2;   // PINE,4
const int STOP_B =      3;   // PINE,5
const int OUT1 =        12;  // spare output
const int OUT2 =        13;  // spare output
//
const int ENABLE_A = 	  4;
const int ENABLE_B =    5;
const int CSB_A  =      6;
const int CSB_B =       7;
const int INP1 =        8;   // spare input
const int INP2 =        9;   // spare input
const int INTB_A =  	  10;  // PINB,4
const int INTB_B =      11;  // PINB,5
const int D16 =         16;  // spare unassigned
const int D17 =         17;  // spare unassigned
const int COARSEint =   18;  // hardware interrupt for COARSE clock
const int SLAVE_SYNC =  A8;  // use to sync multiple boards
const int AN9 =         A9;  // spare unassigned
const int AN10 =        A10; // spare unassigned
const int EXT_LED_CLK = A11; // external LED shows 100kHz clock status -- PORTK,3
const int EXT_LED_A =   A12; // external LED tandem with LED_A -- PORTK,4
const int EXT_LED_B =   A13; // external LED tandem with LED_B -- PORTK,5
const int LED_A =       A14; // onboard LED -- PORTK,6
const int LED_B =       A15; // onboard LED -- PORTK,7

// These are macros to turn LEDs on and off really fast.
// We trade flexibility for speed.

#define CLR_LED_A (PORTK&=(~(1<<6)))
#define SET_LED_A (PORTK|=(1<<6))
#define CLR_LED_B (PORTK&=(~(1<<7)))
#define SET_LED_B (PORTK|=(1<<7))
#define CLR_EXT_LED_A (PORTK&=(~(1<<4)))
#define SET_EXT_LED_A (PORTK|=(1<<4))
#define CLR_EXT_LED_B (PORTK&=(~(1<<5)))
#define SET_EXT_LED_B (PORTK|=(1<<5))
#define CLR_EXT_LED_CLK (PORTK&=(~(1<<3)))
#define SET_EXT_LED_CLK (PORTK|=(1<<3))



#define PS_PER_SEC                (int64_t)  1000000000000   // ps/s

enum MeasureMode : unsigned char {Timestamp, Interval, Period, timeLab, Debug, Null};


///*****************************************************************/
// default values for config struct
#define DEFAULT_MODE              (MeasureMode) 1       // Measurement mode -- 0 is Timestamp
#define DEFAULT_POLL_CHAR         (char)    0x00        // In poll mode, wait for this before output
#define DEFAULT_CLOCK_HZ          (int64_t) 10000000    // 10 MHz
#define DEFAULT_PICTICK_PS        (int64_t) 100000000   // 100us
#define DEFAULT_CAL_PERIODS       (int16_t) 20          // CAL_PERIODS (2, 10, 20, 40)
#define DEFAULT_TIMEOUT           (int16_t)  0x05       // measurement timeout
#define DEFAULT_SYNC_MODE         (char)    'M'         // (M)aster or (S)lave
#define DEFAULT_START_EDGE_0      (char)    'R'         // (R)ising or (F)alling
#define DEFAULT_START_EDGE_1      (char)    'R'         // (R)ising or (F)alling
#define DEFAULT_TIME_DILATION_0   (int64_t) 2500        // SWAG that seems to work
#define DEFAULT_TIME_DILATION_1   (int64_t) 2500        // SWAG that seems to work
#define DEFAULT_FIXED_TIME2_0     (int64_t) 0           // 0 to calculate, or fixed (~1135)
#define DEFAULT_FIXED_TIME2_1     (int64_t) 0           // 0 to calculate, or fixed (~1135)
#define DEFAULT_FUDGE0_0          (int64_t) 0           // Fudge channel A value (ps)
#define DEFAULT_FUDGE0_1          (int64_t) 0           // Fudge channel B value (ps)












#endif	/* BOARD_H */
