/*
  Attiny85 Snowflake pendant necklace
  Jeff Glancy
  2/17/15
  Blinks 6 charliepexed LED's, sleeps, and repeats
  -----
  Much of the power conservation code comes from Nick Gammon:
  http://gammon.com.au/power
 */
 
/*
Copyright (c) 2015 Jeff Glancy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <avr/sleep.h>    // Sleep Modes
#include <avr/power.h>    // Power management
#include <avr/wdt.h>      // Watchdog timer
#include <EEPROM.h>      // EEPROM reading for randomseed

// pins - charlieplexed LED
// 2 - LEDs 1,2,3,4
// 3 - LEDs 1,2,5,6
// 4 - LEDs 3,4,5,6
short pinHigh[7] = {0,2,3,2,4,3,4};
short pinLow[7]  = {0,3,2,4,2,4,3};

// LED placement
//   5  6
// 4      1
//   3  2
short ledFlashOrder[6][6] = {{1,2,6,3,5,4},
                             {2,3,1,4,6,5},
                             {3,4,2,5,1,6},
                             {4,5,3,6,2,1},
                             {5,6,4,1,3,2},
                             {6,1,5,2,4,3}};

// flash the LEDs in pattern across the flake
void flash ()
{
  short pos = random(6);  //start at this LED
  short waitOn = random(90,241);  //LED On randomness
  short waitOff = random(180,481);  //LED Off randomness

  if (random(2) == 0)
  {
    //go across flake starting at one point
    ledPulse(ledFlashOrder[pos][0],waitOn/3);
    delay(waitOff/3);
    ledPulse2(ledFlashOrder[pos][1],ledFlashOrder[pos][2],waitOn/9);
    delay(waitOff/3);
    ledPulse2(ledFlashOrder[pos][3],ledFlashOrder[pos][4],waitOn/9);
    delay(waitOff/3);
    ledPulse(ledFlashOrder[pos][5],waitOn/3);
  }
  else
  {
    //go across flake starting at two points
    ledPulse2(ledFlashOrder[pos][0],ledFlashOrder[pos][1],waitOn/4);
    delay(waitOff/2);
    ledPulse2(ledFlashOrder[pos][2],ledFlashOrder[pos][3],waitOn/9);
    delay(waitOff/2);
    ledPulse2(ledFlashOrder[pos][4],ledFlashOrder[pos][5],waitOn/4);
  }
  
}  // end of flash()
  
// Turn on specified LED in the charlieplexed matrix for specified amount of time
void ledBlink(short led, short wait)
{
  //all pins start as inputs, no PU
  //set two pins to output
  pinMode(pinHigh[led], OUTPUT);
  pinMode(pinLow[led], OUTPUT);

  //set one pin high to turn led on
  digitalWrite(pinHigh[led], HIGH);
  //wait
  delay(wait);  //wait time
  //set high pin to low
  digitalWrite(pinHigh[led], LOW);
  
  //set two pins back to inputs
  pinMode(pinHigh[led], INPUT);
  pinMode(pinLow[led], INPUT);
 
} // end ledBlink()

// Pulse on/off specified LED in the charlieplexed matrix for specified amount of time
void ledPulse(short led, short wait)
{
  // pulse led(s) on/off 1/3 ms, 250 Hz, 25% duty
  for(short i = 0; (i * 4) < wait; i++)
  {
    ledBlink(led, 1);
    delay(3);
  }
 
} // end ledPulse()

// Pulse on/off two specified LEDs in the charlieplexed matrix for specified amount of time
void ledPulse2(short led1, short led2, short wait)
{
  // pulse led(s) on/off 1/3 ms, 250 Hz, 25% duty
  for(short i = 0; (i * 4) < wait; i++)
  {
    ledBlink(led1, 1);
    delay(1);
    ledBlink(led2, 1);
    delay(1);
  }
 
} // end ledPulse2()


// watchdog interrupt
ISR (WDT_vect) 
{
   wdt_disable();  // disable watchdog
}  // end of WDT_vect

void SleepyTime(int TimeToSleep) {  // This will power down the MCU

  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  // Configure sleep to full power down mode
//  ADCSRA = 0;            // turn off ADC
  power_all_disable ();  // power off ADC, Timer 0 and 1, serial interface
  noInterrupts ();       // timed sequence coming up
  
  MCUSR = 0;     // clear various "reset" flags
  WDTCR = bit (WDCE) | bit (WDE) | bit (WDIF);  // allow changes, disable reset, clear existing interrupt

  switch (TimeToSleep) 
  {
    case 16:
      WDTCR = bit (WDIE);    // set WDIE, and 16 ms delay
      break;  
    case 32:
      WDTCR = bit (WDIE) | bit (WDP0);    // set WDIE, and 32 ms delay
      break;
    case 64:
      WDTCR = bit (WDIE) | bit (WDP1);    // set WDIE, and 64 ms delay
      break;
    case 125:
      WDTCR = bit (WDIE) | bit (WDP1) | bit (WDP0);    // set WDIE, and 0.125 s delay
      break;
    case 250:
      WDTCR = bit (WDIE) | bit (WDP2);    // set WDIE, and 0.25 s delay
      break;
    case 500:
      WDTCR = bit (WDIE) | bit (WDP2) | bit (WDP0);    // set WDIE, and 0.5 s delay
      break;
    case 1000:
      WDTCR = bit (WDIE) | bit (WDP2) | bit (WDP1);    // set WDIE, and 1 s delay
      break;
    case 2000:
      WDTCR = bit (WDIE) | bit (WDP2) | bit (WDP1) | bit (WDP0);    // set WDIE, and 2 s delay
      break;
    case 4000:
      WDTCR = bit (WDIE) | bit (WDP3);    // set WDIE, and 4 s delay
      break;
    case 8000:
      WDTCR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 s delay
      break; 
    default:
      WDTCR = bit (WDIE) | bit (WDP3);    // set WDIE, and 4 s delay
  }
  
/* Reference chart for Watchdog Timer
 --------------------------------------------
 WDP3      WDP2      WDP1      WDP0      Time
 --------------------------------------------
 0        0        0        0        16ms
 0        0        0        1        32ms
 0        0        1        0        64ms
 0        0        1        1        0.125s
 0        1        0        0        0.25s
 0        1        0        1        0.5s
 0        1        1        0        1.0s
 0        1        1        1        2.0s
 1        0        0        0        4.0s
 1        0        0        1        8.0s
 */

  wdt_reset();          // pat the dog
  sleep_enable ();       // ready to sleep
  interrupts ();         // interrupts are required now
  sleep_cpu ();          // sleep  
  sleep_disable ();      // precaution
  
//  power_all_enable ();   // power everything back on
  power_timer0_enable();  // timer0 used for delay()
}


// the setup routine runs once when you press reset:
void setup() 
{
  // initialize all pins as an inputs, no pull-up.
  for (int i = 0; i <=4; i++)
  {
    pinMode(i, INPUT);
    digitalWrite(i, LOW);
  }
  
  // power savings
  ADCSRA = 0;            // turn off ADC
  //power_all_disable ();  // power off ADC, Timer 0 and 1, serial interface
  power_adc_disable();
  //power_timer0_disable();  // Timer 0 is needed for delay() command
  power_timer1_disable();
  power_usi_disable();
  
  //seed random number generator with predetermined value
  unsigned int id = EEPROM.read(0);
  randomSeed(id);
  EEPROM.write(0, id+random(10));
}


// the loop routine runs over and over again forever:
void loop()
{

  flash ();

  switch (random (2,(3+1)))
  {
//    case 0:
//      SleepyTime(500);
//      break;
//    case 1:
//      SleepyTime(1000);
//      break;
    case 2:
      SleepyTime(2000);
      break;
    case 3:
      SleepyTime(4000);
      break;
//    case 4:
//      SleepyTime(8000);
//      break;
    default:
      SleepyTime(4000);
  }
  
} // end of loop
