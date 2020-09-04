/*
 * Example of the Timer Input Capture mode combined with one pulse mode
 *  
 * This example uses:
 * - Timer2 channel 1 as capture input
 * - Timer2 channel 2 to generate the pulses,
 * - Timer 3 to generate a PWM trigger signal for capture input
 */

//#include <Streaming.h>
#include "RBD_Capacitance.h"

const uint16_t pulseDelay = 5;
const uint16_t min_freq = 1;
const uint32_t system_clock_frequency = 72000000;

RBD::Capacitance cap_sensor(PA12, PB7); // send, receive pin

String inString = "";    // string to hold input

//-----------------------------------------------------------------------------
//void toggle_led()


void generate_pwm(uint16_t prescale, uint16_t reg_counts)
{
  
    Serial.println("prescale: ");
    Serial.print(prescale);
    Serial.print(",");
    Serial.println(reg_counts);
    
  Timer2.pause();

  Timer2.setPrescaleFactor(prescale); // 1 µs resolution
  Timer2.setCompare(TIMER_CH2, reg_counts / 2);
  Timer2.setOverflow(reg_counts - 1);

  // channel 2: invert polarity (we want low for CNT<CCR2)
  Timer2.setPolarity(TIMER_CH2, 1);

  // start timer 2
  Timer2.refresh();
  Timer2.resume(); // let timer 2 run
}

void set_frequency(uint32_t frequency)
{
    if(frequency < min_freq)
    {
    Serial.println("Invalid Frequency(f>=10)!");
    }
    else
    {
      /* code */
      Serial.print("Target Frequency: ");
      Serial.println(frequency);
      uint32_t count_value = system_clock_frequency / frequency;
      uint32_t min_count = count_value / 0x10000;
      uint32_t max_count = sqrt(count_value) + 1;
      if(min_count == 0)
        min_count = 1;
  
      uint32_t optimized_regs = 0, optimized_prescale = 0, optimized_counts = 0;
      for(uint16_t p = min_count; p<=max_count; p++)
      {
        uint32_t regs = count_value / p;
        uint32_t cur_count = regs * p;
        if(regs < 0x10000 && cur_count > optimized_counts)
        {
          optimized_regs = regs;
          optimized_prescale = p;
          optimized_counts = cur_count;
          if(optimized_counts == count_value) break;
        }
      }
      
      generate_pwm(optimized_prescale, optimized_regs );
      
      Serial.println(frequency*optimized_regs*optimized_prescale);
    }    
}
//-----------------------------------------------------------------------------
void setup()
{
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // send an intro:
  Serial.println("\n\nPWM Generation by Uart data");
  pinMode(PC13, OUTPUT);
  // setup PA1 (Timer2 channel 2) to PWM (one pulse mode)
  pinMode(PA1, PWM);
  

  // stop the timers before configuring them
  //generate_pwm(72, 100);
  //set_frequency(75);
}

  uint32_t t=0, ff=0;
//-----------------------------------------------------------------------------
void loop()
{
  if ( (millis()-t)>=1000 )
  {
    t = millis();
  
    ff++;
    if ( ff%5 == 0 )
    {
      if ( ff > 100000 )
        {ff=10;}

      //set_frequency(ff);
    }
  }
  
  cap_sensor.update();

  if(cap_sensor.onChange()) {
    unsigned long capValue = cap_sensor.getValue();
    unsigned long moddcapv = 6500 - capValue;
    set_frequency(moddcapv);
  }
}
