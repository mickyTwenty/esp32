#include <filters.h>
#include <RBD_Capacitance.h>

const float cutoff_freq   = 500.0;  //Cutoff frequency in Hz
const float sampling_time = 0.00005; //Sampling time in seconds.
IIR::ORDER  order  = IIR::ORDER::OD4; // Order (OD1 to OD4)

// Low-pass filter
Filter f(cutoff_freq, sampling_time, order);
RBD::Capacitance cap_sensor(2, 15); // send, receive pin
void setup() {
  Serial.begin(115200);

}

void loop() {
   cap_sensor.update();
// Serial.println(cap_sensor.getValue());
 if(cap_sensor.onChange()) {
    //Serial.print("\t");
    Serial.println(cap_sensor.getValue());
  int value = cap_sensor.getValue();
 
  float filteredval = f.filterIn(value);
  //View with Serial Plotter
  Serial.print(value, DEC);
  Serial.print(",");
  Serial.println(filteredval, 4);
  delayMicroseconds(5); // Loop time will approx. match the sampling time.
}}