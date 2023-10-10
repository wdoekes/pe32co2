#ifndef INCLUDED_PE32_SENS_LM75A_H
#define INCLUDED_PE32_SENS_LM75A_H

#include <Wire.h>

#include "component.h"
#include "sensorvalue.h"

class Generic_LM75_11Bit; // NXP_LM75A

class Sensor_LM75A : public Component {
public:
    Sensor_LM75A(TwoWire* i2c_wire, floatlike& ctemp);
    void setup();
    void tick();

private:
    Generic_LM75_11Bit* m_lm75a;
    TwoWire* m_wire;
    floatlike& m_ctemp;
    unsigned long m_lastact;
};

// vim: set ts=8 sw=4 sts=4 et ai:
#endif //INCLUDED_PE32_SENS_LM75A_H
