#pragma once
#include <Arduino.h>

typedef void (*GlobalCallbackFunction)(const char*, const char*, const char*);
typedef void (*TempCallbackFunction)(const char*, const char*, float);
typedef void (*StatCallbackFunction)(const char*, const char*, bool);
typedef void (*CountCallbackFunction)(const char*, const char*, uint32_t);  //long counter: 4 bytes
typedef void (*CountSCallbackFunction)(const char*, const char*, uint16_t);  //short counter: 2 bytes
typedef void (*ModeCallbackFunction)(const char*, const char*, uint8_t);

enum DPType {
  TEMP,
  STAT,
  COUNT,
  COUNTS,
  MODE
};

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
    virtual const uint8_t getLength() const = 0;
    virtual void callback(uint8_t value[]) = 0;
    virtual void setCallback(GlobalCallbackFunction globalCallback) {};
    virtual Datapoint& setCallback(TempCallbackFunction callback) {};
    virtual Datapoint& setCallback(StatCallbackFunction callback) {};
    virtual Datapoint& setCallback(CountCallbackFunction callback) {};
    virtual Datapoint& setCallback(CountSCallbackFunction callback) {};
    virtual Datapoint& setCallback(ModeCallbackFunction callback) {};
    virtual void transform(uint8_t transformedValue[], float value) {};

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
    virtual Datapoint& setCallback(TempCallbackFunction callback);
    virtual const uint8_t getLength() const { return 2; }
    virtual void callback(uint8_t value[]);
    virtual void transform(uint8_t transformedValue[], float value);
  private:
    TempCallbackFunction _callback;
};

class StatDP : public Datapoint {
  public:
    StatDP(const char* name, const char* group, const uint16_t address, bool isWriteable);
    virtual Datapoint& setCallback(StatCallbackFunction callback);
    virtual const uint8_t getLength() const { return 1; }
    virtual void callback(uint8_t value[]);
    virtual void transform(uint8_t transformedValue[], float value);
  private:
    StatCallbackFunction _callback;
};

class CountDP : public Datapoint {
  public:
    CountDP(const char* name, const char* group, const uint16_t address, bool isWriteable);
    virtual Datapoint& setCallback(CountCallbackFunction callback);
    virtual const uint8_t getLength() const { return 4; }
    virtual void callback(uint8_t value[]);
    virtual void transform(uint8_t transformedValue[], float value);
  private:
    CountCallbackFunction _callback;
};

class CountSDP : public Datapoint {
  public:
    CountSDP(const char* name, const char* group, const uint16_t address, bool isWriteable);
    virtual Datapoint& setCallback(CountSCallbackFunction callback);
    virtual const uint8_t getLength() const { return 2; }
    virtual void callback(uint8_t value[]);
    virtual void transform(uint8_t transformedValue[], float value);
  private:
    CountSCallbackFunction _callback;
};

class ModeDP : public Datapoint {
  public:
    ModeDP(const char* name, const char* group, const uint16_t address, bool isWriteable);
    Datapoint& setCallback(ModeCallbackFunction callback);
    virtual const uint8_t getLength() const { return 1; }
    virtual void callback(uint8_t value[]);
    virtual void transform(uint8_t transformedValue[], float value);
  private:
    ModeCallbackFunction _callback;
};
