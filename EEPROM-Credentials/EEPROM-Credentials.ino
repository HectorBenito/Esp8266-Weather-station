/* 
 *  Storing wifi credentials to ESP8266 EEPROM memory.
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
 *      
 */

#include "credentials.h" //file to store the credentials to be saved
#include <EEPROM.h>


//*******  Save in EEPROM  ***********/
void store_credentials(byte n) {
  int addr;
  addr+=1; 
  for(int i = 0; i<n; i++){
     addr+=60;
    }
  String a =  wifi_credentials[n][0];
  int string_length = (a.length() + 1);
  char inchar[29];    //'29' max lenght of the string containing each credential
  a.toCharArray(inchar, string_length);
  EEPROM.write(addr, string_length);
  for (int i = 0; i < string_length; i++) {
    addr++;
    EEPROM.write(addr, inchar[i]);
  }
  addr+=30;
  a =  wifi_credentials[n][1];
  string_length = (a.length() + 1);
  a.toCharArray(inchar, string_length);
  EEPROM.write(addr, string_length);
  for (int i = 0; i < string_length; i++) {
    addr++;
    EEPROM.write(addr, inchar[i]);
  }
  EEPROM.commit();
}


//*******  Read from EEPROM  ***********/
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

void setup() {
  EEPROM.begin(4096);
  Serial.begin(9600);

  EEPROM.write(0, wifis);
  EEPROM.commit();
  for(int i=0; i<wifis; i++){
      store_credentials(i);
  }; 
  Serial.print("Saved");
}

void loop() {
  for(int i=0; i<wifis; i++){
      read_credentials(i);
      Serial.print("SSID: ");
      Serial.println(credentials[0]);
      Serial.print("PASS: ");
      Serial.println(credentials[1]);
  }
  Serial.println();
}
