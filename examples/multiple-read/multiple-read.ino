/*

Attention! Since Serial (aka UART0) is used for communication with the Viessmann boiler,
serial logging is disabled by default.
Hence, this sketch generates no output.
You can specify another printer like Serial1 or use a telnet server to see the debug messages.

*/

#include <ESP8266WiFi.h>
#include <VitoWifi.h>


const char* ssid     = "xxxx";
const char* password = "xxxx";
const int interval = 60;


VitoWifi myVitoWifi;


uint32_t lastMillis = 0;
bool doLoop = false;
bool sendNewDP = true;
uint8_t DPindex = 0;
Datapoint currentDP;
char value[8] = {0};


//Use struct to hold Viessmann datapoints
//name - RW - address - length - returntype
//char[15+1] - READ/WRITE - uint16_t - uint8_t - type
//const Datapoint DP[] = {
const PROGMEM Datapoint DP[] = {
  "toutside", READ, 0x5525, 2, TEMP,
  "tdhw", READ, 0x0812, 2, TEMP,
  "tdhwsolar", READ, 0x6566, 2, TEMP,
  "tdhwsolarcoll", READ, 0x6564, 2, TEMP,
  "statsolarpump", READ, 0x0846, 1, TEMP,
  "tboiler", READ, 0x0810, 2, TEMP,
  "tfluegas", READ, 0x5642, 2, TEMP
};
//const uint8_t numberOfDPs = sizeof(DP) / sizeof(DP[0]);
const uint8_t numberOfDPs = ArraySize(DP);


void setup(){

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
  }

  //Start Viessmann communication on Serial (aka UART0)
  myVitoWifi.begin(&Serial);

  //myVitoWifi.setLoggingPrinter(&XXXX); //replace XXXX by your printer
  //myVitoWifi.enableLogger(true);

}


void loop(){

  //put loop() function inside main loop
  myVitoWifi.loop();

  if(millis() - lastMillis >= interval * 1000UL){
    //loop through DPs every interval
    lastMillis = millis();
    doLoop = true;
    DPindex = 0;
  }

  if(doLoop){
    //Loop when flag is raised
    if(sendNewDP){
      //handle when previous has been handled
      PROGMEM_readAnything(&DP[DPindex], currentDP);
      myVitoWifi.sendDP(currentDP);
      sendNewDP = false;
    }
    //when value is available, display
    if(myVitoWifi.available()){
        //Display value and move to next DP
        myVitoWifi.read(value, sizeof(value));
        myVitoWifi.getLogger().print("Name: ");
        myVitoWifi.getLogger().println(currentDP.name);
        myVitoWifi.getLogger().print("Value: ");
        myVitoWifi.getLogger().println(value);
        myVitoWifi.getLogger().println();
        DPindex++;
        sendNewDP = true;
    }
    if(DPindex > numberOfDPs){
      //reset when array has completely been handled
      doLoop = false;
    }
  }
}//end loop()
