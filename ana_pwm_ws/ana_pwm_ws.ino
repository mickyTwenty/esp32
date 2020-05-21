// Load Wi-Fi library
#include <WiFi.h>
#include "ESPAsyncWebServer.h"

const int pwmpin = T0;
const int anapin = 34;

const int allowed_freq = 5 * 1000 * 1000;

unsigned long max_freq =  12 * 1000;
unsigned long min_freq = 1.2 * 1000;

int pwm_ch = 0;
int resolution = 12;

int ana_val;

// Replace with your network credentials
const char* ssid = "MikroTik";
const char* password = "";

// Set web server port number to 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Helvetica; font-size: 24px; display: inline-block; text-align: center;}
    body {max-width: 600px; margin:0px auto;}
    span {color: red;}
    div  {margin-bottom: 10px;}
    input[type=text] {height: 40px; font-size: 20px; width: 150px;}
    input[type=button] {background-color: #4CAF50; color: white; border: none; cursor: pointer; font-size: 24px; padding: 10px 30px; }
  </style>
</head>
<body>
  <h2>Analog to PWM</h2>
  <p>Frequency range <span id="s_min_freq">%FREQ_MIN_STYLE%</span> ~ <span id="s_max_freq">%FREQ_MAX_STYLE%</span></p>
  <div>
    <label>Min Frequency(Hz)</label>
    <input type="text" id="min_freq" value="%MINFREQVALUE%"/>
    <input type="button" value="Set" onclick="set_min()"/>
  </div>
  <div>
    <label>Max Frequency(Hz)</label>
    <input type="text" id="max_freq" value="%MAXFREQVALUE%"/>
    <input type="button" value="Set" onclick="set_max()"/>
  </div>
  
<script>
function set_min(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if ( this.responseText == "FAILED" ) {
        alert("Failed! Min Frequency is invalid!");
      } else {
        document.getElementById("s_min_freq").innerHTML = this.responseText;
      }
    }
  };
  var value = document.getElementById("min_freq").value;
  xhr.open("GET", "/setmin?val=" + value, true);
  xhr.send();
  
}
function set_max(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if ( this.responseText == "FAILED" ) {
        alert("Failed! Max Frequency is invalid!");
      } else {
        document.getElementById("s_max_freq").innerHTML = this.responseText;
      }
    }
  };
  var value = document.getElementById("max_freq").value;
  xhr.open("GET", "/setmax?val=" + value, true);
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

String make_style(int freq) {
  if ( freq >= 1000 * 1000 ) { //MHz
    return String((float)freq / 1000 / 1000) + "MHz";
  } else if ( freq >= 1000 ) { //KHz
    return String((float)freq / 1000) + "KHz";
  } else {
    return String(freq) + "Hz";
  }
  return String();
}

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "FREQ_MIN_STYLE"){
    return make_style(min_freq);
  }
  if(var == "FREQ_MAX_STYLE"){
    return make_style(max_freq);
  }
  if(var == "MINFREQVALUE"){
    return String(min_freq);
  }
  if(var == "MAXFREQVALUE"){
    return String(max_freq);
  }
  return String();
}


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

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/setmin", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputMessage;
    
    inputMessage = request->getParam("val")->value();
    int new_val = inputMessage.toInt();

    if ( new_val < 0 || new_val >= max_freq ) {
      request->send(200, "text/plain", "FAILED");
      return;
    }

    min_freq = new_val;
    Serial.print("set min freq: ");
    Serial.println(min_freq);
    request->send(200, "text/plain", make_style(min_freq));
  });

  server.on("/setmax", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputMessage;
    
    inputMessage = request->getParam("val")->value();
    int new_val = inputMessage.toInt();

    if ( new_val > allowed_freq || new_val <= min_freq ) {
      request->send(200, "text/plain", "FAILED");
      return;
    }
    
    max_freq = new_val;
    Serial.print("set max freq: ");
    Serial.println(max_freq);
    request->send(200, "text/plain", make_style(max_freq));

    ledcSetup(pwm_ch, max_freq, resolution);
  });
  
  server.begin();
}

void loop() {
  ana_val = analogRead(anapin);

  int led_val = mapping_ana(ana_val);
  ledcWrite(pwm_ch, led_val);
  delay(15);
}
