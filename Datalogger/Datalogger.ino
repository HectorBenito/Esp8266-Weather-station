/* 
 *  ESP8266 client with a DHT and a bmp180 sensor as inputs.
 *  
 *    Copyright (c) [2016] [Hector Benito]
 *    
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 *   
 *     Must use ESP8266 Arduino core from:
 *      https://github.com/esp8266/Arduino
 *     Arduino EEPROM library from:
 *      https://www.arduino.cc/en/Reference/EEPROM
 *     DHT sensors Adafruit library from:
 *      https://github.com/adafruit/DHT-sensor-library
 *     Arduino Wire library from: 
 *      https://www.arduino.cc/en/Reference/Wire
 *     Bmp180 Sparkfun library from:
 *      https://github.com/sparkfun/BMP180_Breakout/tree/master/Libraries/Arduino
 *      
 */
 
#include <EEPROM.h> //for reading the wifi credentials

#include <ESP8266WiFi.h> //for using wifi and acting as a client
#include <WiFiClient.h>

#include <ESP8266WiFiMulti.h> //for connect to one of the saved wifis
ESP8266WiFiMulti WiFiMulti;

ADC_MODE(ADC_VCC); //for use the analog input for reading the input voltage

#include <SFE_BMP180.h> //for use the bmp180 temperature and presure sensor
#include <Wire.h>
SFE_BMP180 bmp180;
double T,P,A; // Values read from sensor
double SeaLevel=1013.25; //pressure at sea level in mbar

#include "DHT.h" //for use the temperature and humidity sensor
      #define DHTPIN 12   // what digital pin we're connected to
    // Uncomment whatever sensor type you're using!
      //#define DHTTYPE DHT11   // DHT 11
      #define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
      //#define DHTTYPE DHT21   // DHT 21 (AM2301)
    // Initialize DHT sensor - adafruit note
    // NOTE: For working with a faster than ATmega328p 16 MHz Arduino chip, like an ESP8266,
    // you need to increase the threshold for cycle counts considered a 1 or 0.
    // You can do this by passing a 3rd parameter for this threshold.  It's a bit
    // of fiddling to find the right value, but in general the faster the CPU the
    // higher the value.  The default for a 16mhz AVR is a value of 6.  For an
    // Arduino Due that runs at 84mhz a value of 30 works.
    // This is for the ESP8266 processor on ESP8266
    DHT dht(DHTPIN, DHTTYPE, 15); // 15 works fine for ESP8266 running at 80mhz
    float h, t, hic;  // Values read from sensor

const char* host = "example.com"; //what server to connect
const int port = 80; // http server is running on default port 80


//Reading stored credentials from the eeprom memory
String credentials[2];
void read_credentials(byte n) {
  credentials[0]=""; credentials[1]="";
  int addr;
  addr+=1; 
  for(int i = 0; i<n; i++){
     addr+=60;
  }
  int value;
  int string_length = EEPROM.read(addr);
  for (int i = 0; i < string_length; i++) {
    addr++;
    value = EEPROM.read(addr);
    credentials[0] += (char)value;
  }
  addr+=30;
  string_length = EEPROM.read(addr);
  for (int i = 0; i < string_length; i++) {
    addr++;
    value = EEPROM.read(addr);
    credentials[1] += (char)value;
  }
}

 
void setup(void)
{  
  Serial.begin(9600); // Serial connection from ESP via 3.3V
  Serial.println(""); Serial.println("Weather Reading Client");
  EEPROM.begin(4096); //initialize the eeprom memory
  Wire.pins(2, 0); //initialize i2c interface - sda gpio2, scl gpio0
  Wire.begin(2, 0);
  if (!bmp180.begin()){ //wait until the bmp180 is initialized
    yield();
    return;
  }
  dht.begin(); //initialize DHT sensor

  WiFi.mode(WIFI_STA); //initialize wifi in station mode 
  //read stored wifi credentials and put them into multiwifi
    char ssid[29]; char pass[29];
    int wifis=(int)EEPROM.read(0);
    for(int i=0; i<wifis; i++){
        read_credentials(i);
        credentials[0].toCharArray(ssid, credentials[0].length());
        credentials[1].toCharArray(pass, credentials[1].length());
        WiFiMulti.addAP(ssid, pass);
    };
  delay(10);
  
  float vcc = (float)ESP.getVcc(); //get the input voltage value
  Serial.print("current voltage: "); Serial.print(vcc); Serial.println("V"); //print it to serial console
  if (vcc<2.7){ //if battery is low, input voltage lower than 2.7V, deep sleep for maximum time
    Serial.print("Low battery, powering off");
    ESP.deepSleep(0xFFFFFFF);
    delay(100);
  }
  
   delay(10);
}

void loop(void)
{
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  t = dht.readTemperature();
  // Check if any reads failed and try again
  if (isnan(h) || isnan(t)) {
    yield();
    return;
   }
  // Compute heat index in Celsius (isFahreheit = false)
  hic = dht.computeHeatIndex(t, h, false);

  char status = bmp180.startTemperature(); //starts the bmp180 temperature reading
  if (status != 0)
  {   
    delay(status); //wait until temperature reading is obtained
    status = bmp180.getTemperature(T); //save the temperature reading
    if (status != 0)
    {
      status = bmp180.startPressure(3); //starts the bmp180 pressure reading, it will take 3 readings to obtain an accurate result
      if (status != 0)
      { 
        delay(status); //wait until pressure reading is obtained       
        status = bmp180.getPressure(P,T); //save the pressure reading
        if (status != 0)
        { 
          A = bmp180.altitude(P,SeaLevel); //compute the altitude with the pressure data
        }      
      }      
    }   
  }
  Serial.println("Sensors done");
    
  // Connect to WiFi network with multiwifi
  Serial.print("Waiting for WiFi ");
  while(WiFiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(200);
  }
   
  Serial.print("Connected, ");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  WiFiClient client; //start wifi client
  Serial.println("Client started");
  if (client.connect(host, port)) { //try to the connect to remote server
    Serial.println("connected");

    String p = String(ESP.getChipId()); //the esp unique id will be the password for the server
    
    String url = "/dato.php?pass=" + p + "&temp=" + T + "&pres=" + P + "&alt=" + A + "&hum=" + h + "&temp2=" + t + "&hic=" + hic; //formulate the url with GET requests
    Serial.print("Requesting URL: "); //print it to serial console
    Serial.println(url);
  
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: keep-alive\r\n\r\n"); //make the request to the server
    client.println(); //print empty line for apache servers

    //Wait up to 10 seconds for server to respond then read response
    int i = 0;
    while ((!client.available()) && (i < 1000)) {
      delay(10);
      i++;
    }
    //print the server response to serial console
    while (client.available())
    {
      String Line = client.readStringUntil('\r');
      Serial.print(Line);
    }
    client.stop();
  } 
  else {
    Serial.println("connection failed");
  }
  Serial.println();

  pinMode(DHTPIN, OUTPUT); //set the DHT pin as low output to prevent bad readings after waking from deep sleep
  digitalWrite(DHTPIN, LOW);
  
  Serial.println("Sleeping..."); //deep sleep for less power consumption, up to 1.22mAh in my tests, remember to connect gpio16 and reset pins together
  ESP.deepSleep(10 * 60 * 1000000); // loop every 10 minutes
  delay(100);
}
