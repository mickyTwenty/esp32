const int pwmpin = T0;
const int anapin = 34;


unsigned long max_freq = 10 * 1000;
unsigned long min_freq =  5 * 1000;

int pwm_ch = 0;
int resolution = 12;

int ana_val;

int mapping_ana(int ana) {
  float res = 4096.0;
  float min_pwm = res / max_freq * min_freq;

  return min_pwm + (res - min_pwm) * (ana / res);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  ledcSetup(pwm_ch, max_freq, resolution);

  ledcAttachPin(pwmpin, pwm_ch);
}

void loop() {
  // put your main code here, to run repeatedly:
  ana_val = analogRead(anapin);
  Serial.print("ana: ");
  Serial.print(ana_val);

  int led_val = mapping_ana(ana_val);
  Serial.print(" pwm: ");
  Serial.println(led_val);
  ledcWrite(pwm_ch, led_val);
  delay(15);
}
