#ifndef INCLUDED_PE32_SENS_SCD30_H
#define INCLUDED_PE32_SENS_SCD30_H

#include <Wire.h>

#include "component.h"
#include "sensorvalue.h"

class SensirionI2cScd30;

class Sensor_SCD30 : public Component {
public:
    Sensor_SCD30(TwoWire* i2c_wire, floatlike& co2, floatlike& ctemp, floatlike& humid);
    void setup();
    void tick();

private:
    SensirionI2cScd30* m_scd30;
    TwoWire* m_wire;
    floatlike& m_co2;
    floatlike& m_ctemp;
    floatlike& m_humid;
    unsigned long m_lastact;
};

// vim: set ts=8 sw=4 sts=4 et ai:
#endif //INCLUDED_PE32_SENS_SCD30_H
