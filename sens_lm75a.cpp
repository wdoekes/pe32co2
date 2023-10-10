#include "sens_lm75a.h"

#include <Arduino.h>

#include <Temperature_LM75_Derived.h>

#include "pe32co2.h"

Sensor_LM75A::Sensor_LM75A(TwoWire* i2c_wire, floatlike& ctemp) :
    m_lm75a(new Generic_LM75_11Bit(i2c_wire)), m_wire(i2c_wire), m_ctemp(ctemp)
{
}

void Sensor_LM75A::setup()
{
#if 0
    if (!m_lm75a->begin(*m_wire)) {
        Serial << "LM75A:  Error initializing NXP LM75A (CJMCU-75)\r\n";
        delete m_lm75a;
        m_lm75a = NULL;
        return;
    }
#endif

    //Serial << "LM75A:  Config: " << m_lm75a->readConfigurationRegister() << " (?)\r\n";
    Serial << "LM75A:  Up and running\r\n";
}

void Sensor_LM75A::tick()
{
    if (m_lm75a && (millis() - m_lastact) >= 2000) {
        float ctemp = m_lm75a->readTemperatureC();

        if (-0.14f < ctemp and ctemp < -0.12f) {
            Serial << "LM75A:  Ignoring " << ctemp << " value; looks like no sensor\r\n";
            m_lastact = millis();
            return;
        }

        m_ctemp = ctemp;
        m_lastact = millis();

#ifdef DEBUG_SENSORS
        Serial << "LM75A:  " <<
            (floattype)m_ctemp <<  " [" << (floattype)ctemp << "] (temp-C)\r\n";
#endif
    }
}

// vim: set ts=8 sw=4 sts=4 et ai:
