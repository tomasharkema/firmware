#include "DsmrModule.h"
#include <dsmr.h>
#include "configuration.h"

#define RX_BUFFER 1700
#define TIMEOUT 250
#define BAUD 115200
#define ACK 1


using MyData = ParsedData<
  /* String */ identification,
  /* String */ p1_version,
  /* String */ timestamp,
  /* String */ equipment_id,
  /* FixedValue */ energy_delivered_tariff1,
  /* FixedValue */ energy_delivered_tariff2,
  /* FixedValue */ energy_returned_tariff1,
  /* FixedValue */ energy_returned_tariff2,
  /* String */ electricity_tariff,
  /* FixedValue */ power_delivered,
  /* FixedValue */ power_returned,
  /* FixedValue */ electricity_threshold,
  /* uint8_t */ electricity_switch_position,
  /* uint32_t */ electricity_failures,
  /* uint32_t */ electricity_long_failures,
  /* String */ electricity_failure_log,
  /* uint32_t */ electricity_sags_l1,
  /* uint32_t */ electricity_sags_l2,
  /* uint32_t */ electricity_sags_l3,
  /* uint32_t */ electricity_swells_l1,
  /* uint32_t */ electricity_swells_l2,
  /* uint32_t */ electricity_swells_l3,
  /* String */ message_short,
  /* String */ message_long,
  /* FixedValue */ voltage_l1,
  /* FixedValue */ voltage_l2,
  /* FixedValue */ voltage_l3,
  /* FixedValue */ current_l1,
  /* FixedValue */ current_l2,
  /* FixedValue */ current_l3,
  /* FixedValue */ power_delivered_l1,
  /* FixedValue */ power_delivered_l2,
  /* FixedValue */ power_delivered_l3,
  /* FixedValue */ power_returned_l1,
  /* FixedValue */ power_returned_l2,
  /* FixedValue */ power_returned_l3,
  /* uint16_t */ gas_device_type,
  /* String */ gas_equipment_id,
  /* uint8_t */ gas_valve_position,
  /* TimestampedFixedValue */ gas_delivered,
  /* uint16_t */ thermal_device_type,
  /* String */ thermal_equipment_id,
  /* uint8_t */ thermal_valve_position,
  /* TimestampedFixedValue */ thermal_delivered,
  /* uint16_t */ water_device_type,
  /* String */ water_equipment_id,
  /* uint8_t */ water_valve_position,
  /* TimestampedFixedValue */ water_delivered,
  /* uint16_t */ slave_device_type,
  /* String */ slave_equipment_id,
  /* uint8_t */ slave_valve_position,
  /* TimestampedFixedValue */ slave_delivered
>;


DsmrModule *dsmrModule;

DsmrModule::DsmrModule() : concurrency::OSThread("DsmrModule") {}


int32_t DsmrModule::runOnce() {
  
if (firstTime == 1) {

  LOG_WARN("DsmrModule firstTime = 1");

  p1Serial->setRxBufferSize(RX_BUFFER);
  p1Serial->setRxInvert(true);
  p1Serial->begin(115200, SERIAL_8N1, P1_RX);
  p1Serial->flush();

  delay(10);

  p1Reader = new P1Reader(p1Serial, P1_DTR);

  LOG_WARN("DsmrModule setup done");
  firstTime=0;
}

else {  
  LOG_WARN("DsmrModule RUN!");
  p1Serial->flush();

  delay(10);

  LOG_WARN("DsmrModule firstTime = 0");
  LOG_WARN("DsmrModule ENABLE");
    
  p1Reader->enable(true);

      LOG_WARN("DsmrModule LOOP IN");
  
      // while (!p1Reader->loop()) {
  
      while (!p1Reader->available()) {
        p1Reader->loop();
        delay(10);  
      }
  


      LOG_WARN("DsmrModule LOOP OUT");
  
      if (p1Reader->available()) {

      LOG_WARN("AVAILABLE");
  
      }else {
       LOG_WARN("NOT AVAILABLE");
  
      }

          MyData data;
    String err;
    if (p1Reader->parse(&data, &err)) {
      // Parse succesful, print result
      LOG_WARN("power_delivered:");
      LOG_WARN("power_delivered: %i", data.power_delivered.int_val());
      LOG_WARN("power_delivered_l1:");
      LOG_WARN("power_delivered_l1: %i", data.power_delivered_l1.int_val());
    } else {
        LOG_WARN("ERROR:");
      // Parser error, print error
      LOG_WARN(err.c_str());
    }

  LOG_WARN(p1Reader->raw().c_str());
  LOG_WARN("DsmrModule LOOP DONE");

  p1Reader->clear();
  p1Reader->disable();
}

return 30000;

  
}
