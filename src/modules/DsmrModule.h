#pragma once
#ifdef HELTEC_V3_P1
#include "concurrency/OSThread.h"
#include <dsmr.h>

#define P1_RX 48
#define P1_TX 47
#define P1_DTR 26

class DsmrModule : private concurrency::OSThread
{
  HardwareSerial *p1Serial = &Serial1;
  P1Reader *p1Reader;

    bool firstTime = 1;
    char outbuf[90] = "";

  public:
    DsmrModule();

  protected:
    virtual int32_t runOnce() override;

  private:
    void processWXSerial();
};

extern DsmrModule *dsmrModule;

#endif
