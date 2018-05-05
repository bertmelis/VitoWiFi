/* Copyright 2017 Bert Melis */

#include <Arduino.h>
#include <Datapoint.h>

DPManager dpManager;

// find datapoint
IDatapoint* getDP(DPManager dpManager, const char* name) {
  for (auto it = dpManager.begin(); it != dpManager.end(); ++it) {
    if (strcmp(name, (*it).get()->getName()) == 0) {
      return (*it).get();
    }
  }
  return nullptr;
}

void setup() {
#if defined ARDUINO_ARCH_ESP8266
  Serial.begin(74880);
#elif defined ARDUINO_ARCH_ESP32
  Serial.begin(115200);
#endif
  dpManager.addDP("Raw", "DPs", 0x00, RAW).setLength(4);
  dpManager.addDP("TempL", "DPs", 0x00, TEMPL);
  dpManager.addDP("TempS", "DPs", 0x00, TEMPS);
  dpManager.addDP("Stat", "DPs", 0x00, STAT);
  dpManager.addDP("CountL", "DPs", 0x00, COUNTL);
  dpManager.addDP("CountS", "DPs", 0x00, COUNTS);
  dpManager.addDP("Mode", "DPs", 0x00, MODE);
  dpManager.addDP("Hours", "DPs", 0x00, HOURS);
  dpManager.addDP("CoP", "DPs", 0x00, COP);
}

void loop() {
  static uint32_t lastMillis;
  if (millis() - lastMillis > 10000UL) {
    lastMillis = millis();

    // note: all byte arrays are stored little endian, so printing high byte first
    Serial.print("Testing Datapoints:\n");

    char out_str[10] = {0};

    {
      // test: 26.3°C is encoded to 0x0107
      DPValue value(26.3f);
      uint8_t raw[4] = {0};
      getDP(dpManager, "TempL")->encode(raw, value);
      Serial.printf("Temp %#1.1f°C is encoded to 0x%02x%02x\n", value.getFloat(), raw[0], raw[1]);
      value = getDP(dpManager, "TempL")->decode(raw);
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
      getDP(dpManager, "TempS")->encode(raw, value);
      Serial.printf("Temp %u°C is encoded to 0x%02x\n", value.getU8(), raw[0]);
      value = getDP(dpManager, "TempS")->decode(raw);
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
      getDP(dpManager, "Stat")->encode(raw, value);
      Serial.printf("Stat %s is encoded to 0x%02x\n", value.getBool() ? "true" : "false", raw[0]);
      value = getDP(dpManager, "Stat")->decode(raw);
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
      getDP(dpManager, "CountL")->encode(raw, value);
      Serial.printf("Long Counter %u is encoded to 0x%02x%02x%02x%02x\n", value.getU32(), raw[0], raw[1], raw[2], raw[3]);
      value = getDP(dpManager, "CountL")->decode(raw);
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
      getDP(dpManager, "CountS")->encode(raw, value);
      Serial.printf("Short Counter %u is encoded to 0x%02x%02x\n", value.getU16(), raw[1], raw[0]);
      value = getDP(dpManager, "CountS")->decode(raw);
      Serial.printf("Short Counter 0x%02x%02xx is decoded to %u\n", raw[1], raw[0], value.getU16());
      Serial.print("Or printed directly: ");
      value.getString(out_str, sizeof(out_str));
      Serial.println(out_str);
    }

    {
      // test: 458.62 is encoded to ...
      DPValue value(458.62f);
      uint8_t raw[4] = {0};
      getDP(dpManager, "Hours")->encode(raw, value);
      Serial.printf("Hours %#1.1f is encoded to 0x%02x%02x%02x%02x\n", value.getFloat(), raw[0], raw[1], raw[2], raw[3]);
      value = getDP(dpManager, "Hours")->decode(raw);
      Serial.printf("Hours 0x%02x%02x%02x%02x is decoded to %#1.1f\n", raw[0], raw[1], raw[2], raw[3], value.getFloat());
      Serial.print("Or printed directly: ");
      value.getString(out_str, sizeof(out_str));
      Serial.println(out_str);
    }

    {
      // test: 2.6 is encoded to ...
      DPValue value(2.6f);
      uint8_t raw[1] = {0};
      getDP(dpManager, "CoP")->encode(raw, value);
      Serial.printf("CoP %#1.1f is encoded to 0x%02x\n", value.getFloat(), raw[0]);
      value = getDP(dpManager, "CoP")->decode(raw);
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
      getDP(dpManager, "Raw")->encode(raw, value);
      value.getString(out_str, sizeof(out_str));
      Serial.printf("Raw %s is encoded to 0x%02x%02x%02x%02x\n", out_str, raw[0], raw[1], raw[2], raw[3]);
    }

    // test: print all DPs and length
    Serial.print("\nThese are all the registered datapoints:\n");
    for (auto i = dpManager.begin(); i != dpManager.end(); ++i) {
      Serial.printf("DP: %s - length %d\n", (*i)->getName(), (*i)->getLength());
    }

    // test: what happens if DP is not found?
    delay(1000);
    if (getDP(dpManager, "unknown")) {
      Serial.printf("\nUnknown DP length: %d\n", getDP(dpManager, "unknown")->getLength());
    } else {
      Serial.println("Unknown datapoint is not found.");
    }
  }
}
