#include <Arduino.h>

#include <Adafruit_CCS811.h>

#include "arduinoutil.h"
#include "sens_ccs811.h"

Sensor_CCS811::Sensor_CCS811(TwoWire* i2c_wire, floatlike& eco2, floatlike& tvoc) :
    m_ccs811(new Adafruit_CCS811()), m_wire(i2c_wire), m_eco2(eco2), m_tvoc(tvoc)
{
}

void Sensor_CCS811::setup()
{
    // BEWARE: Don't forget to put the RST pin of the CCS811 in GND
    if (!m_ccs811->begin(CCS811_ADDRESS, m_wire)) {
        Serial << "CCS811: Failed to start sensor! Please check your wiring\r\n";
        delete m_ccs811;
        m_ccs811 = NULL;
        return;
    }

    // Sample often
    //m_ccs811->setDriveMode(CCS811_DRIVE_MODE_250MS);
    m_ccs811->setDriveMode(CCS811_DRIVE_MODE_10SEC);

    Serial << "CCS811: Up and running\r\n";
}

void Sensor_CCS811::tick()
{
    if (m_ccs811 && (millis() - m_lastact) >= 2000 && m_ccs811->available()) {
        uint8_t ret = m_ccs811->readData(); /* ignore error status */
        if (ret != 0) {
            Serial << "CCS811: Got non-0 readData() status: " << ret << "\r\n";
            return;
        }
        float eco2 = m_ccs811->geteCO2();
        float tvoc = m_ccs811->getTVOC();

        m_eco2 = eco2;
        m_tvoc = tvoc;
        m_lastact = millis();

#ifdef DEBUG_SENSORS
        Serial << "CCS811: " <<
            (floattype)m_eco2 << " [" << (floattype)eco2 << "] (CO2-ppm)\t" <<
            (floattype)m_tvoc << " [" << (floattype)tvoc << "] (TVOC)\t" <<
            (hextype<uint16_t>)m_ccs811->getBaseline() << " (baseline)\r\n";
#endif
    }
}

void Sensor_CCS811::set_environment(float ctemp, float humid)
{
    if (m_ccs811) {
        m_ccs811->setEnvironmentalData(ctemp, humid);
    }
}

// vim: set ts=8 sw=4 sts=4 et ai:
