/*

Attention! Since Serial (aka UART0) is used for communication with the Viessmann boiler,
serial logging is disabled by default.
You can specify another printer like Serial1 or use a telnet server to see the debug messages.

*/


#include <ESP8266WiFi.h>

#include <VitoWifi.h>

const char* ssid     = "xxxx";
const char* password = "xxxx";

//new Optilink instance using Serial
VitoWifi myVitoWifi;

uint32_t lastMillis = 0;
bool getValues = false;

//Use struct to hold Viessmann datapoints
//name - RW - address - length - type
//char[15+1] - READ/WRITE - uint16_t - uint8_t - type
Datapoint DP[] = {
  "toutside", READ, 0x5525, 2, TEMP,
  "tdhw", READ, 0x0812, 2, TEMP,
  "tdhwsolar", READ, 0x6566, 2, TEMP,
  "tdhwsolarcoll", READ, 0x6564, 2, TEMP,
  "statsolarpump", READ, 0x0846, 1, TEMP,
  "tboiler", READ, 0x0810, 2, TEMP,
  "tfluegas", READ, 0x5642, 2, TEMP
};
uint8_t numberOfDPs = sizeof(DP) / sizeof(DP[0]);
uint8_t currentDP = 0;

void setup(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  //Start Viessmann communication on Serial (aka UART0)
  myVitoWifi.begin(&Serial);
}


void loop(){
  //put loop() function inside main loop
  myVitoWifi.loop();

  if((millis() - lastMillis) > 30000){
    //flag to get new values every 30 seconds
    //start with DP zero.
    lastMillis = millis();
    getValues = true;
    currentDP = 0;
  }

  //if flag is set, get values
  if(getValues){
    //pass Datapoint by reference
    myVitoWifi.sendDP(&DP[currentDP]);
    //stop sending new DPs when all DPs are sent.
    if(currentDP >= numberOfDPs) getValues = false;
  }

  //when value is available, display
  if( myVitoWifi.getStatus() == RETURN ){
      //Display value and move to next DP
      myVitoWifi.getLogger().print("Name: ");
      myVitoWifi.getLogger().println(DP[currentDP].name);
      myVitoWifi.getLogger().print("Value: ");
      myVitoWifi.getLogger().println(myVitoWifi.getValue());
      myVitoWifi.getLogger().println();
      currentDP++;
  }
}
