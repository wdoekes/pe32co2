#ifndef INCLUDED_PE32_SENS_CCS811_H
#define INCLUDED_PE32_SENS_CCS811_H

#include <Wire.h>

#include "component.h"
#include "sensorvalue.h"

class Adafruit_CCS811;

class Sensor_CCS811 : public Component {
public:
    Sensor_CCS811(TwoWire* i2c_wire, floatlike& eco2, floatlike& tvoc);
    void setup();
    void tick();

    void set_environment(float ctemp, float humid);

private:
    Adafruit_CCS811* m_ccs811;
    TwoWire* m_wire;
    floatlike& m_eco2;
    floatlike& m_tvoc;
    unsigned long m_lastact;
};

// vim: set ts=8 sw=4 sts=4 et ai:
#endif //INCLUDED_PE32_SENS_CCS811_H
