#include <Arduino.h>
#include <Adafruit_BME680.h>

#include "pe32co2.h"
#include "sens_bme680.h"

Sensor_BME680::Sensor_BME680(TwoWire* i2c_wire, floatlike& ctemp, floatlike& humid) :
    m_bme680(new Adafruit_BME680()), m_wire(i2c_wire), m_ctemp(ctemp), m_humid(humid)
{
}

void Sensor_BME680::setup()
{
    if (!m_bme680->begin()) {
        Serial << "BME680: Error initializing BME680\r\n";
        delete m_bme680;
        m_bme680 = NULL;
        return;
    }

    // Set up oversampling and filter initialization
    m_bme680->setTemperatureOversampling(BME68X_OS_4X);
    m_bme680->setHumidityOversampling(BME68X_OS_4X);
    m_bme680->setPressureOversampling(BME68X_OS_NONE);
    m_bme680->setIIRFilterSize(BME680_FILTER_SIZE_0);
    //m_bme680->setGasHeater(320, 150); // 320*C for 150 ms
    m_bme680->setGasHeater(0, 0);  // 320*C for 150 ms
    m_bme680->beginReading();

    Serial << "BME680: Up and running\r\n";
}

void Sensor_BME680::tick()
{
    if (m_bme680 && (millis() - m_lastact) >= 2000 &&
            m_bme680->remainingReadingMillis() <= 0 && m_bme680->endReading()) {
        // BEWARE: Temp is too high on the MCH2022 badge because the
        // sensor is close to backlight (and other components)
        float ctemp = m_bme680->temperature;
        float humid = m_bme680->humidity;
        //canvas->print(bme.pressure / 100.0);
        //canvas->print("hPa, ");
        //canvas->print(bme.gas_resistance / 1000.0);
        //canvas->print("gas-KOhms, ");
        //#define SEALEVELPRESSURE_HPA (1013.25)
        //canvas->print(bme.readAltitude(SEALEVELPRESSURE_HPA));
        //canvas->print("alt-m");

        m_ctemp = ctemp;
        m_humid = humid;
        m_lastact = millis();

        m_bme680->beginReading();

#ifdef DEBUG_SENSORS
        Serial << "BME680: " <<
            (floattype)m_ctemp <<  " [" << (floattype)ctemp << "] (temp-C)\t" <<
            (floattype)m_humid << " [" << (floattype)humid << "] (humid%)\r\n";
#endif
    }
}

// vim: set ts=8 sw=4 sts=4 et ai:
