#include <Arduino.h>

#include <SensirionI2CScd4x.h>

#include "pe32co2.h"
#include "sens_scd40.h"

Sensor_SCD40::Sensor_SCD40(TwoWire* i2c_wire, floatlike& co2, floatlike& ctemp, floatlike& humid) :
    m_scd40(new SensirionI2CScd4x()), m_wire(i2c_wire), m_co2(co2), m_ctemp(ctemp), m_humid(humid)
{
}

void Sensor_SCD40::setup()
{
    uint16_t error;

    m_scd40->begin(*m_wire);
    error = m_scd40->stopPeriodicMeasurement();
    if (error) {
        char error_msg[256];
        // errorToString is in Sensirion_Core
        errorToString(error, error_msg, sizeof error_msg);
        Serial << "SCD40:  Error trying to execute stopPeriodicMeasurement(): " <<
            error_msg << "\r\n";
        delete m_scd40;
        m_scd40 = NULL;
        return;
    }

    uint16_t serial0;
    uint16_t serial1;
    uint16_t serial2;
    error = m_scd40->getSerialNumber(serial0, serial1, serial2);
    if (error) {
        char error_msg[256];
        // errorToString is in Sensirion_Core
        errorToString(error, error_msg, sizeof error_msg);
        Serial << "SCD40:  Error trying to execute getSerialNumber(): " <<
            error_msg << "\r\n";
        delete m_scd40;
        m_scd40 = NULL;
        return;
    }

    Serial << "SCD40:  Serial# " << (hextype<uint16_t>)serial0 <<
        (hextype<uint16_t>)serial1 << (hextype<uint16_t>)serial2 << "\r\n";

    error = m_scd40->startPeriodicMeasurement();
    if (error) {
        char error_msg[256];
        // errorToString is in Sensirion_Core
        errorToString(error, error_msg, sizeof error_msg);
        Serial << "SCD40:  Error trying to execute startPeriodicMeasurement(): " <<
            error_msg << "\r\n";
        delete m_scd40;
        m_scd40 = NULL;
        return;
    }
}

void Sensor_SCD40::tick()
{
    if (m_scd40 && (millis() - m_lastact) >= 250) {
        bool data_ready = false;
        uint16_t error = m_scd40->getDataReadyFlag(data_ready);
        if (error) {
            char error_msg[256];
            // errorToString is in Sensirion_Core
            errorToString(error, error_msg, sizeof error_msg);
            Serial << "SCD40:  Error trying to execute getDataReady(): " <<
                error_msg << "\r\n";
        }
        if (!data_ready) {
            return;
        }

        uint16_t co2;
        float ctemp, humid;
        error = m_scd40->readMeasurement(co2, ctemp, humid);
        if (error) {
            char error_msg[256];
            // errorToString is in Sensirion_Core
            errorToString(error, error_msg, sizeof error_msg);
            Serial << "SCD40:  Error trying to execute readMeasurementData(): " <<
                error_msg << "\r\n";
            return;
        }

        m_co2 = (float)co2;
        m_ctemp = ctemp;
        m_humid = humid;
        m_lastact = millis();

#ifdef DEBUG_SENSORS
        Serial << "SCD40:  " <<
            (floattype)m_co2 << " [" << (floattype)co2 << "] (CO2-ppm)\t" <<
            (floattype)m_ctemp << " [" << (floattype)ctemp << "] (C)\t" <<
            (floattype)m_humid << " [" << (floattype)humid << "] (humid%)\r\n";
#endif
    }
}

// vim: set ts=8 sw=4 sts=4 et ai:
