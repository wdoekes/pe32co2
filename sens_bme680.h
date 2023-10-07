#ifndef INCLUDED_PE32_SENS_BME680_H
#define INCLUDED_PE32_SENS_BME680_H

#include <Wire.h>

#include "component.h"
#include "sensorvalue.h"

class Adafruit_BME680;

class Sensor_BME680 : public Component {
public:
    Sensor_BME680(TwoWire* i2c_wire, floatlike& ctemp, floatlike& humid);
    void setup();
    void tick();

private:
    Adafruit_BME680* m_bme680;
    TwoWire* m_wire;
    floatlike& m_ctemp;
    floatlike& m_humid;
    unsigned long m_lastact;
};

// vim: set ts=8 sw=4 sts=4 et ai:
#endif //INCLUDED_PE32_SENS_BME680_H
