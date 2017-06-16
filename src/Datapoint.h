#pragma once
#include <Arduino.h>
#include "Datatypes.h"


class Datapoint {
  public:
    Datapoint(const char* name, const char* group, const uint16_t address, bool isWriteable);
    ~Datapoint();
    const char* getName() const;
    const char* getGroup() const;
    const uint16_t getAddress() const;
    const bool isWriteable() const;
    Datapoint& setWriteable();
    void setGlobalCallback(GlobalCallbackFunction globalCallback);

    //virtual methods, see inherited classes for implementation
    virtual const uint8_t getLength() const { return 0; };
    virtual void callback(uint8_t value[]);
    virtual void setCallback(GlobalCallbackFunction globalCallback) {};
    virtual Datapoint& setCallback(TempCallbackFunction callback) {};
    virtual Datapoint& setCallback(StatCallbackFunction callback) {};
    virtual void transformValue(uint8_t transformedValue[], float value) {};
    virtual void transformValue(uint8_t transformedValue[], bool value) {};

  protected:  //all properties are protected for ease of use in inherited classes
    static GlobalCallbackFunction _globalCallback;
    const char* _name;
    const char* _group;
    const uint16_t _address;
    bool _writeable;
};

class TempDP : public Datapoint {
  public:
    TempDP(const char* name, const char* group, const uint16_t address, bool isWriteable);
    Datapoint& setCallback(TempCallbackFunction callback);
    Datapoint& setCallback(StatCallbackFunction callback) {};
    const uint8_t getLength() const { return 2; }
    void callback(uint8_t value[]);
    void transformValue(uint8_t transformedValue[], float value);
    void transformValue(uint8_t transformedValue[], bool value) { /* you shouldn't be here */ };
  private:
    TempCallbackFunction _callback;
};

class StatDP : public Datapoint {
  public:
    StatDP(const char* name, const char* group, const uint16_t address, bool isWriteable);
    Datapoint& setCallback(StatCallbackFunction callback);
    Datapoint& setCallback(TempCallbackFunction callback) {};
    const uint8_t getLength() const { return 1; }
    void callback(uint8_t value[]);
    void transformValue(uint8_t transformedValue[], float value) { /* you shouldn't be here */ };
    void transformValue(uint8_t transformedValue[], bool value);
  private:
    StatCallbackFunction _callback;
};

/*
class ModeDP : public Datapoint {
  public:
    ModeDP(const char* name, const char* group, const uint16_t address, bool isWriteable);
    Datapoint& setCallback(ModeCallbackFunction callback);
    virtual void callback(uint8_t value[]);
    virtual const uint8_t getLength() const { return 1; }
  private:
    ModeCallbackFunction _callback;
};


class HoursDP : public Datapoint {
  public:
    HoursDP(const char* name, const char* group, const uint16_t address, bool isWriteable);
    Datapoint& setCallback(HoursCallbackFunction callback);
    virtual void callback(uint8_t value[]);
    virtual const uint8_t getLength() const { return 2; }
  private:
    HoursCallbackFunction _callback;
};


class HoursLDP : public Datapoint {
  public:
    HoursLDP(const char* name, const char* group, const uint16_t address, bool isWriteable);
    Datapoint& setCallback(HoursLCallbackFunction callback);
    virtual void callback(uint8_t value[]);
    virtual const uint8_t getLength() const { return 4; }
  private:
    HoursLCallbackFunction _callback;
};


class CountDP : public Datapoint {
  public:
    CountDP(const char* name, const char* group, const uint16_t address, bool isWriteable);
    Datapoint& setCallback(CountCallbackFunction callback);
    virtual void callback(uint8_t value[]);
    virtual const uint8_t getLength() const { return 4; }
  private:
    CountCallbackFunction _callback;
};
*/
