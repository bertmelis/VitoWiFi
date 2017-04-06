/*

Attention! Since Serial (aka UART0) is used for communication with the Viessmann boiler,
serial logging is disabled by default.
You can specify another printer like Serial1 or use a telnet server to see the debug messages.

*/


#include <ESP8266WiFi.h>

#include <VitoWifi.h>

const char* ssid     = "xxxx";
const char* password = "xxxx";


VitoWifi myVitoWifi;

uint32_t lastMillis = 0;
bool getValues = false;

//Use struct to hold Viessmann datapoints
//name - RW - address - length - type
//char[15+1] - READ/WRITE - uint16_t - uint8_t - type
const Datapoint DP = { "OutsideTemp", READ, 0x5525, 16, TEMP};


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
    lastMillis = millis();
    getValues = true;
  }

  //if flag is set, get values
  if(getValues){
    myVitoWifi.sendDP(DP);
    getValues = false;
  }

  //when value is available, display
  if( myVitoWifi.getStatus() == RETURN ){
      myVitoWifi.getLogger().print("Name: ");
      myVitoWifi.getLogger().println(DP.name);
      myVitoWifi.getLogger().print("Value: ");
      myVitoWifi.getLogger().println(myVitoWifi.getValue());
      myVitoWifi.getLogger().println();
      getValues = true;
  }
}
