/*

This example displays all possible datapoints and shows how the conversion is done.
You don't need an actual optolink to use this example.

*/

#include <Arduino.h>
#include <Datapoint.hpp>

DPRaw raw_dp("Raw", "DPs", 0x00);
DPTemp temp_dp("TempL", "DPs", 0x00);
DPTempS temps_dp("TempS", "DPs", 0x00);
DPStat stat_dp("Stat", "DPs", 0x00);
DPCount count_dp("CountL", "DPs", 0x00);
DPCountS counts_dp("CountS", "DPs", 0x00);
DPMode mode_dp("Mode", "DPs", 0x00);
DPHours hours_dp("Hours", "DPs", 0x00);
DPCoP cop_dp("CoP", "DPs", 0x00);

void setup() {
#if defined ARDUINO_ARCH_ESP8266
  Serial.begin(74880);
#elif defined ARDUINO_ARCH_ESP32
  Serial.begin(115200);
#endif

  delay(10000);
  
  // note: all byte arrays are stored little endian, so printing high byte first
  Serial.print("Testing Datapoints:\n");

  char out_str[10] = {0};

  {
    // test: 26.3°C is encoded to 0x0107
    DPValue value(26.3f);
    uint8_t raw[4] = {0};
    temp_dp.encode(raw, value);
    Serial.printf("Temp %#1.1f°C is encoded to 0x%02x%02x\n", value.getFloat(), raw[0], raw[1]);
    value = temp_dp.decode(raw);
    Serial.printf("Temp 0x%02x%02x is decoded to %#1.1f°C\n", raw[0], raw[1], value.getFloat());
    Serial.print("Or printed directly: ");
    value.getString(out_str, sizeof(out_str));
    Serial.println(out_str);
  }

  {
    // test: 16°C is encoded to 0x16
    uint8_t temp = 16;
    DPValue value(temp);
    uint8_t raw[1] = {0};
    temps_dp.encode(raw, value);
    Serial.printf("Temp %u°C is encoded to 0x%02x\n", value.getU8(), raw[0]);
    value = temps_dp.decode(raw);
    Serial.printf("Temp 0x%02x is decoded to %u°C\n", raw[0], value.getU8());
    Serial.print("Or printed directly: ");
    value.getString(out_str, sizeof(out_str));
    Serial.println(out_str);
  }

  {
    // test: True is encoded to 0x01
    bool tmp = true;
    DPValue value(tmp);
    uint8_t raw[1] = {0};
    stat_dp.encode(raw, value);
    Serial.printf("Stat %s is encoded to 0x%02x\n", value.getBool() ? "true" : "false", raw[0]);
    value = stat_dp.decode(raw);
    Serial.printf("Stat 0x%02x is decoded to %s\n", raw[0], value.getBool() ? "true" : "false");
    Serial.print("Or printed directly: ");
    value.getString(out_str, sizeof(out_str));
    Serial.println(out_str);
  }

  {
    // test: 458620 is encoded to 0x7CFF06
    uint32_t temp = 458620;
    DPValue value(temp);
    uint8_t raw[4] = {0};
    count_dp.encode(raw, value);
    Serial.printf("Long Counter %u is encoded to 0x%02x%02x%02x%02x\n", value.getU32(), raw[0], raw[1], raw[2], raw[3]);
    value = count_dp.decode(raw);
    Serial.printf("Long Counter 0x%02x%02x%02x%02x is decoded to %u\n", raw[0], raw[1], raw[2], raw[3], value.getU32());
    Serial.print("Or printed directly: ");
    value.getString(out_str, sizeof(out_str));
    Serial.println(out_str);
  }

  {
    // test: 45862 is encoded to 0x26B3
    uint16_t temp = 45862;
    DPValue value(temp);
    uint8_t raw[2] = {0};
    counts_dp.encode(raw, value);
    Serial.printf("Short Counter %u is encoded to 0x%02x%02x\n", value.getU16(), raw[1], raw[0]);
    value = counts_dp.decode(raw);
    Serial.printf("Short Counter 0x%02x%02xx is decoded to %u\n", raw[1], raw[0], value.getU16());
    Serial.print("Or printed directly: ");
    value.getString(out_str, sizeof(out_str));
    Serial.println(out_str);
  }

  {
    // test: 458.62 is encoded to ...
    DPValue value(458.62f);
    uint8_t raw[4] = {0};
    hours_dp.encode(raw, value);
    Serial.printf("Hours %#1.1f is encoded to 0x%02x%02x%02x%02x\n", value.getFloat(), raw[0], raw[1], raw[2], raw[3]);
    value = hours_dp.decode(raw);
    Serial.printf("Hours 0x%02x%02x%02x%02x is decoded to %#1.1f\n", raw[0], raw[1], raw[2], raw[3], value.getFloat());
    Serial.print("Or printed directly: ");
    value.getString(out_str, sizeof(out_str));
    Serial.println(out_str);
  }

  {
    // test: 2.6 is encoded to ...
    DPValue value(2.6f);
    uint8_t raw[1] = {0};
    cop_dp.encode(raw, value);
    Serial.printf("CoP %#1.1f is encoded to 0x%02x\n", value.getFloat(), raw[0]);
    value = cop_dp.decode(raw);
    Serial.printf("CoP 0x%02x is decoded to %#1.1f\n", raw[0], value.getFloat());
    Serial.print("Or printed directly: ");
    value.getString(out_str, sizeof(out_str));
    Serial.println(out_str);
  }

  {
    // test: just copy bytes from in to out
    uint8_t tmp[4] = { 0x01, 0x02, 0x03, 0x04};
    DPValue value(tmp, sizeof(tmp));
    uint8_t raw[4] = {0};
    raw_dp.setLength(4);
    raw_dp.encode(raw, value);
    value.getString(out_str, sizeof(out_str));
    Serial.printf("Raw %s is encoded to 0x%02x%02x%02x%02x\n", out_str, raw[0], raw[1], raw[2], raw[3]);
  }

  // test: print all DPs and length
  const std::vector<IDatapoint*>& v = raw_dp.getCollection();
  Serial.print("\nThese are all the registered datapoints:\n");
  for (uint8_t i = 0; i < v.size(); ++i)
    Serial.printf("DP: %s - length %d\n", v[i]->getName(), v[i]->getLength());
}

void loop() {
}
