#include <Arduino.h>

#include <SensirionI2cScd30.h>

#include "arduinoutil.h"
#include "sens_scd30.h"

Sensor_SCD30::Sensor_SCD30(TwoWire* i2c_wire, floatlike& co2, floatlike& ctemp, floatlike& humid) :
    m_scd30(new SensirionI2cScd30()), m_wire(i2c_wire), m_co2(co2), m_ctemp(ctemp), m_humid(humid)
{
}

void Sensor_SCD30::setup()
{
    m_scd30->begin(Wire, SCD30_I2C_ADDR_61);
    m_scd30->stopPeriodicMeasurement();
    m_scd30->softReset();
    //delay(1500); // XXX??

    uint16_t error;
    uint8_t major = 0;
    uint8_t minor = 0;
    error = m_scd30->readFirmwareVersion(major, minor);
    if (error != NO_ERROR) {
        char error_msg[256];
        // errorToString is in Sensirion_Core
        errorToString(error, error_msg, sizeof error_msg);
        Serial << "SCD30:  Error trying to execute readFirmwareVersion(): " <<
            error_msg;
        delete m_scd30;
        m_scd30 = NULL;
        return;
    }

    Serial << "SCD30:  Firmware version " << major << "." << minor << "\r\n";
    error = m_scd30->startPeriodicMeasurement(2); // measure every 2s..
    if (error != NO_ERROR) {
        char error_msg[256];
        // errorToString is in Sensirion_Core
        errorToString(error, error_msg, sizeof error_msg);
        Serial << "SCD30:  Error trying to execute startPeriodicMeasurement(): " <<
            error_msg << "\r\n";
        delete m_scd30;
        m_scd30 = NULL;
    }
}

void Sensor_SCD30::tick()
{
    if (m_scd30 && (millis() - m_lastact) >= 2000) {
        uint16_t data_ready = 0;
        uint16_t error = m_scd30->getDataReady(data_ready);

        // error is OR'ed with data_ready :-/
        if (!data_ready) {
            if (error) {
                char error_msg[256];
                // errorToString is in Sensirion_Core
                errorToString(error, error_msg, sizeof error_msg);
                Serial << "SCD30:  Error trying to execute getDataReady(): " <<
                    error_msg << "\r\n";
            }
            return;
        }

        float co2, ctemp, humid;
        error = m_scd30->readMeasurementData(co2, ctemp, humid);
        if (error != NO_ERROR) {
            char error_msg[256];
            // errorToString is in Sensirion_Core
            errorToString(error, error_msg, sizeof error_msg);
            Serial << "SCD30:  Error trying to execute readMeasurementData(): " <<
                error_msg << "\r\n";
            return;
        }

        m_co2 = co2;
        m_ctemp = ctemp;
        m_humid = humid;
        m_lastact = millis();

#ifdef DEBUG_SENSORS
        Serial << "SCD30:  " <<
            (floattype)m_co2 << " [" << (floattype)co2 << "] (CO2-ppm)\t" <<
            (floattype)m_ctemp << " [" << (floattype)ctemp << "] (C)\t" <<
            (floattype)m_humid << " [" << (floattype)humid << "] (humid%)\r\n";
#endif
    }
}

// vim: set ts=8 sw=4 sts=4 et ai:
