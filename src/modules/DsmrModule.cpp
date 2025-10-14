#include "DsmrModule.h"
#include <dsmr.h>
#include "DebugConfiguration.h"
#include "Router.h"
#include "MeshService.h"
#include <StreamString.h>

#define RX_BUFFER 1700
#define TIMEOUT 250
#define BAUD 115200
#define ACK 1

struct Printer
{
  StreamString *p;

  Printer(StreamString *p) : p(p)
  {
  }

  template <typename Item>
  void apply(Item &i)
  {
    if (i.present())
    {
      p->print(Item::get_name());
      p->print(F(": "));
      p->print(i.val());
      p->println(Item::unit());
    }
  }
};

DsmrModule *dsmrModule;

DsmrModule::DsmrModule() : concurrency::OSThread("DsmrModule") {}

int32_t DsmrModule::cleanup()
{
  LOG_WARN("cleanup");
  p1Reader->clear();
  p1Reader->disable();
  streamString->flush();
  return 30000;
}

int32_t DsmrModule::runOnce()
{

  if (firstTime == 1)
  {

    LOG_WARN("DsmrModule firstTime = 1");

    Serial2.setRxBufferSize(RX_BUFFER);
    // Serial2.setRxInvert(true);
    Serial2.begin(115200, SERIAL_8N1, P1_RX, -1, true, 1000);
    Serial2.flush();

    delay(10);

    p1Reader = new P1Reader(&Serial2, P1_DTR);

    LOG_WARN("setup done");
    firstTime = 0;
  }

  else
  {
    LOG_WARN("RUN!");
    Serial2.flush();

    delay(10);

    LOG_WARN("firstTime = 0");
    LOG_WARN("ENABLE");

    auto enableTime = millis();

    p1Reader->enable(true);

    LOG_WARN("LOOP IN");

    while (!p1Reader->loop())
    {
      delay(10);

      if ((millis() - enableTime) > 1000)
      {
        LOG_WARN("TIMEOUT");
        return cleanup();
      }
    }

    if (p1Reader->available())
    {
      LOG_WARN("AVAILABLE"); // \n%s",.c_str());
    }
    else
    {
      LOG_WARN("NOT AVAILABLE");
      return cleanup();
    }

    MyData data;
    err.clear();
    if (p1Reader->parse(&data, &err))
    {
      LOG_WARN("PARSE SUCCEEDED");
      data.applyEach(Printer(streamString));

      char i = 0;
      while (streamString->available() && i < 10)
      {
        LOG_WARN("SEND PACKET");
        meshtastic_MeshPacket *p = router->allocForSending();

        const meshtastic_Channel *ch = &channels.getByName(Channels::serialChannel);
        if (ch != NULL)
        {
          p->channel = ch->index;
        }
        p->to = NODENUM_BROADCAST;
        p->decoded.want_response = false;
        p->want_ack = ACK;
        p->decoded.portnum = meshtastic_PortNum_SERIAL_APP;

        char buf[meshtastic_Constants_DATA_PAYLOAD_LEN];
        buf[0] = i;
        buf[1] = ';';
        size_t leng = streamString->readBytes(buf + 2, meshtastic_Constants_DATA_PAYLOAD_LEN - 2);
        p->decoded.payload.size = leng + 2;
        memcpy(p->decoded.payload.bytes, buf, p->decoded.payload.size);
        service->sendToMesh(p);
        i++;
      }
    }
    else
    {
      LOG_WARN("ERROR: %s", err.c_str());
    }

    LOG_WARN("DsmrModule LOOP DONE");
    return cleanup();
  }

  return 30000;
}
