//
#include <stdint.h>           // define unint16_t, uint32_t
#include <SPI.h>              // SPI support
//#include <avr/wdt.h>          // watchdog (wdt.h)
#include "board.h"            // LED macros
//
// install EnableInterrupt from the .zip file in the main TICC folder
// or download from https://github.com/GreyGnome/EnableInterrupt
// use "Sketch/Include Library/Add .ZIP Library" to install
#include <EnableInterrupt.h>  // use faster interrupt library
#include "tdc7200.h"          // TDC registers and structures

 
//
boolean ASCIImode = true;
//
//Variable Definitions
//
volatile int64_t PICcount;
int64_t tint;
int64_t CLOCK_HZ;
int64_t PICTICK_PS; 
int64_t CLOCK_PERIOD;
int16_t CAL_PERIODS;
//
//config_t config;
//MeasureMode MODE;
//
static tdc7200Channel channels[] = {
        tdc7200Channel('A', ENABLE_A, INTB_A, CSB_A, STOP_A,LED_A),
        tdc7200Channel('B', ENABLE_B, INTB_B, CSB_B, STOP_B,LED_B),
};

/****************************************************************
We don't use the default setup() routine -- see
ticc_setup() below
****************************************************************/
void setup() { }

/****************************************************************
Here is where setup really happensw
****************************************************************/
void ticc_setup() {
 // wdt_disable(); // Desactivar el watchdog mientras se configura
  int i;
  boolean last_pin;
  
  pinMode(COARSEint, INPUT);
  pinMode(OUT1, OUTPUT);
  pinMode(OUT2, OUTPUT);
  pinMode(EXT_LED_A, OUTPUT);  // need to set these here; on-board LEDs are set up in TDC7200::setup
  pinMode(EXT_LED_B, OUTPUT);
  
  // turn on the LEDs to show we're alive -- use macros from board.h
  SET_LED_A;
  SET_EXT_LED_A;
  SET_LED_B;
  SET_EXT_LED_B;
  SET_EXT_LED_CLK;
  
  
  // start the serial library
  Serial.end();               // first close in case we've come here from a break
  Serial.begin(115200);
  // start the SPI library:
  SPI.end();                  // first close in case we've come here from a break
  SPI.begin();
  
  /*******************************************
   * Configuration read/change/store
  *******************************************/
  CLOCK_HZ =      DEFAULT_CLOCK_HZ;
  CLOCK_PERIOD =  (PS_PER_SEC/CLOCK_HZ);
  PICTICK_PS =    DEFAULT_PICTICK_PS ;;
  CAL_PERIODS =   DEFAULT_CAL_PERIODS;
      
  for(i = 0; i < ARRAY_SIZE(channels); ++i) {
    // initialize the channels struct variables
    channels[i].totalize = 0;
    channels[i].PICstop = 0;
    channels[i].tof = 0;
    channels[i].ts = 0;
    channels[i].time_dilation = DEFAULT_TIME_DILATION_0;
    channels[i].fixed_time2 = DEFAULT_FIXED_TIME2_0;
    channels[i].fudge0 = DEFAULT_FUDGE0_0;

    // setup the chips
    channels[i].tdc_setup();
    channels[i].ready_next();
  }
  
  /*******************************************
   * Synchrnonize multiple TICCs sharing common 10 MHz and 10 kHz clocks.
  *******************************************/ 
 // if (config.SYNC_MODE == 'M') {                     // if we are master, send sync by sending SLAVE_SYNC (A8) high
    pinMode(SLAVE_SYNC, OUTPUT);                     // set SLAVE_SYNC as output (defaults to input)
    digitalWrite(SLAVE_SYNC, LOW);                   // make sure it's low
    delay(1000);                                     // wait a bit in case other boards need to catch up
    last_pin = digitalRead(COARSEint);               // get current state of COARSE_CLOCK
    while (digitalRead(COARSEint) == last_pin) {     // loop until COARSE_CLOCK changes
      delayMicroseconds(5);                          // wait a bit
      if (i <= 50) {                                 // should never get above 20 (100us)
        i++;
        if (i == 50) {                               // something's probably wrong
         // Serial.println("");
         // Serial.println("");
          Serial.println("# No COARSE_CLOCK... is 10 MHz connected?");
        }
      }
    }
    digitalWrite(SLAVE_SYNC, HIGH);                  // send sync pulse
 
  while (!digitalRead(SLAVE_SYNC)) {}                // whether master or slave, spin until SLAVE_SYNC asserts
  PICcount = 0;                                      // initialize counter
  enableInterrupt(COARSEint, coarseTimer, FALLING);  // enable counter interrupt
  enableInterrupt(STOP_A, catch_stopA, RISING);      // enable interrupt to catch channel A
  enableInterrupt(STOP_B, catch_stopB, RISING);      // enable interrupt to catch channel B
  digitalWrite(SLAVE_SYNC, LOW);                     // unassert -- results in ~22uS sync pulse
  pinMode(SLAVE_SYNC, INPUT);                        // set back to input just to be neat

// turn the LEDs off
  CLR_LED_A;
  CLR_EXT_LED_A;
  CLR_LED_B;
  CLR_EXT_LED_B;
  CLR_EXT_LED_CLK;
    
//wdt_enable(WDTO_2S); // Configurar a dos segundos
   
} // ticc_setup  

/****************************************************************/
void loop() {

  ticc_setup();                                     // initialize

  Serial.println("Start");

  while (1) { 
    int i;
    static  int32_t last_micros = 0;                // Loop watchdog timestamp
    static  int64_t last_PICcount = 0;              // Counter state memory
    
    // Ref Clock indicator:
    // Test every 2.5 coarse tick periods for PICcount changes,
    // and turn on EXT_LED_CLK if changes are detected
    if( (micros() - last_micros) > 250 ) {  // hard coded to avoid fp math; breaks if PICTICK_PS changes 
      last_micros = micros();               // Update the watchdog timestamp
      if(PICcount != last_PICcount) {       // Has the counter changed since last sampled?
        SET_EXT_LED_CLK;                    // Yes: LED goes on
        last_PICcount = PICcount;           // Save the current counter state
      } 
      else 
      {
       CLR_EXT_LED_CLK;               // No: LED goes off
     // break;
      }
    }
 
    for(i = 0; i < ARRAY_SIZE(channels); ++i) {
     
      // Only need to do anything if INTB is low, otherwise no work to do.
       if(digitalRead(channels[i].INTB)==0) {     
         // turn LED on -- use board.h macro for speed
         if (i == 0) {SET_LED_A;SET_EXT_LED_A;};
         if (i == 1) {SET_LED_B;SET_EXT_LED_B;};

         channels[i].last_tof = channels[i].tof;  // preserve last value
         channels[i].last_ts = channels[i].ts;    // preserve last value
         channels[i].tof = channels[i].read();    // get data from chip
         channels[i].ts = (channels[i].PICstop * (int64_t)PICTICK_PS) - channels[i].tof;
         channels[i].period = channels[i].ts - channels[i].last_ts;
         channels[i].totalize++;                  // increment number of events   
         channels[i].ready_next();                // Re-arm for next measurement, clear TDC INTB
       
      if ( (channels[i].totalize > 2)) {   
             if ( (channels[0].ts > 1) && (channels[1].ts > 1) ) { // need both channels
             
              if (ASCIImode)
              {
                  print_signed_picos_as_seconds(channels[1].ts - channels[0].ts);
                  Serial.println("");
              }
              else
                {
                  Serial.write(0x02);
                  print_signed_picos_as_seconds(channels[1].ts - channels[0].ts);
                  Serial.write(0x03);
                }
               channels[0].ts = 0; // set to zero for test next time
               channels[1].ts = 0; // set to zero for test next time
             }
 } // print result

       // turn LED off
       if (i == 0) {CLR_LED_A;CLR_EXT_LED_A;};
       if (i == 1) {CLR_LED_B;CLR_EXT_LED_B;};

      } // if INTB
    } // for
  } // while (1) loop

  // Serial.println(" Clock Error");
  
} // main loop()

/****************************************************************
 * Functions
 ****************************************************************/

void print_unsigned_picos_as_seconds (uint64_t x) {
  uint64_t sec, secx, frac, frach, fracx, fracl;    
  char str[128];
  
  sec = abs(x / 1000000000000);
  secx = sec * 1000000000000;
  frac = x - secx;

  // break fractional part of seconds into two 6 digit numbers

  frach = frac / 1000000;
  fracx = frach * 1000000;
  fracl = frac - fracx;

  sprintf(str,"%lu.",sec), Serial.print(str);
  sprintf(str, "%06lu", frach), Serial.print(str);
  sprintf(str, "%06lu", fracl), Serial.print(str);  
} 

void print_signed_picos_as_seconds (int64_t x) {
  int64_t sec, secx, frac, frach, fracx, fracl;    
  char str[128];
  
  sec = abs(x / 1000000000000);  // hopefully avoid double negative sign.  Thanks, Curt!
  secx = sec * 1000000000000;
  frac = abs(x - secx);
  
  // break fractional part of seconds into two 6 digit numbers

  frach = frac / 1000000;
  fracx = frach * 1000000;
  fracl = frac - fracx;

  if (x < 0) {
    Serial.print("-");
  }
  sprintf(str,"%ld.",sec), Serial.print(str);
  sprintf(str, "%06ld", frach), Serial.print(str);
  sprintf(str, "%06ld", fracl), Serial.print(str);  
}

/****************************************************************
 * Interrupt Service Routines
 ****************************************************************/
 
// ISR for timer. Capture PICcount on each channel's STOP 0->1 transition.
void coarseTimer() {
  PICcount++;
}  

void catch_stopA() {
  channels[0].PICstop = PICcount;
}

void catch_stopB() {
  channels[1].PICstop = PICcount;  
}
/****************************************************************/
       
