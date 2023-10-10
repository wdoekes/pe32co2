#include <assert.h>

#include <Arduino.h>
#include <Wire.h>

//#include <WiFi.h>
//#include <ArduinoMqttClient.h>

#include "arduinoutil.h"
#include "component.h"
#include "sensorvalue.h"

#include "sens_bme680.h"
#include "sens_ccs811.h"
#include "sens_scd30.h"
#include "sens_scd40.h"

#include "NetworkComponent.h"

#if BOARD_IS_MCH2022_BADGE
# define GPIO_I2C_SDA 22
# define GPIO_I2C_SCL 21
# define I2C_FREQUENCY 400000UL /* 10,000/100,000/400,000 */
#elif defined(ARDUINO_ARCH_ESP8266)
# undef BOARD_IS_MCH2022_BADGE
# define GPIO_I2C_SDA 4 // D2/SDA
# define GPIO_I2C_SCL 5 // D1/SCL
#elif defined(ARDUINO_ARCH_ESP32)
# undef BOARD_IS_MCH2022_BADGE
#endif

#ifdef BOARD_IS_MCH2022_BADGE
/* Workaround to skip esp_ota_get_running_partition()
 * See: https://wjd.nu/notes/2023#mch2022-badge-arduino-ide-part-1-serial */
extern "C" bool verifyRollbackLater() {
  return true;
}
#endif


/**
 * Container for all the components of this project.
 *
 * Usage:
 *
 *   Component components;
 *   components.add(new SomeComponent());
 *   components.add(new OtherComponent());
 *   components.each([](Component& component) { component.setup(); });
 *   components.each([](Component& component) { component.tick(); });
 */
class Components {
  private:
    typedef void (*ComponentCb)(Component&);
    static constexpr unsigned MAX_COMPONENTS = 10;

  public:
    Components() : m_n(0) {};
    void add(Component* component_ptr) {
      assert(component_ptr);
      assert(m_n < MAX_COMPONENTS);
      m_components[m_n] = component_ptr;
      ++m_n;
    };
    void each(const ComponentCb& cb) {
      for (unsigned i = 0; i < m_n; ++i) {
        cb(*m_components[i]);
      }
    };

  private:
    unsigned int m_n;
    Component* m_components[MAX_COMPONENTS];
};


/**
 * TODO: document me
 */
class Values {
  private:
    static constexpr unsigned MAX_VALUES = 20;
    static const SensorFloat STALE_VALUE;

  public:
    Values() : m_n(0) {};

    SensorFloat& make(char const* title, float initial) {
      assert(title);
      assert(m_n < MAX_VALUES);
      m_titles[m_n] = title; // BUG: Assumes these are from .rodata
      SensorFloat& ret = m_values[m_n];
      ++m_n;
      ret = initial;
      return ret;
    }

    // BUG: This is not very efficient
    const SensorFloat& get(const char* title) {
      for (unsigned i = 0; i < MAX_VALUES && m_titles[i]; ++i) {
        if (strcmp(m_titles[i], title) == 0) {
          return m_values[i];
        }
      }
      return STALE_VALUE; // is never fresh
    }

  private:
    unsigned int m_n;
    char const* m_titles[MAX_VALUES];
    SensorFloat m_values[MAX_VALUES];
};

const SensorFloat Values::STALE_VALUE;


/**
 * Globals.
 */
Components components;
Values vals;
NetworkComponent* net;
Sensor_CCS811* ccs811;

void update_averages();
float update_average(const char **sensor_titles);
float averages_updated = 0;
float average_co2 = 500;
float average_ctemp = 22;
float average_eco2 = 500;
float average_humid = 50;
float average_tvoc = 0;


/**
 * SoC setup routine.
 * We initialize our components here.
 */
void setup()
{
#if defined(ARDUINO_ARCH_ESP32)
  Wire.begin(GPIO_I2C_SDA, GPIO_I2C_SCL, I2C_FREQUENCY);
#elif defined(ARDUINO_ARCH_ESP8266)
  Wire.begin(GPIO_I2C_SDA, GPIO_I2C_SCL);
#else
# error Unknown board
#endif

  Serial.begin(115200);
  delay(500);
  Serial << "\r\nPE32CO2: Starting...\r\n";

  components.add((net = new NetworkComponent()));

  components.add((ccs811 = new Sensor_CCS811(
          &Wire,
          vals.make("ccs811.eco2", average_eco2),
          vals.make("ccs811.tvoc", average_tvoc))));
  components.add(new Sensor_BME680(
        &Wire,
        vals.make("bme680.ctemp", average_ctemp),
        vals.make("bme680.humid", average_humid)));
  components.add(new Sensor_SCD30(
        &Wire,
        vals.make("scd30.co2", average_co2),
        vals.make("scd30.ctemp", average_ctemp),
        vals.make("scd30.humid", average_humid)));
  components.add(new Sensor_SCD40(
        &Wire,
        vals.make("scd40.co2", average_co2),
        vals.make("scd40.ctemp", average_ctemp),
        vals.make("scd40.humid", average_humid)));

  Serial << "PE32CO2: Setting up components/sensors...\r\n";
  components.each([](Component& component) { component.setup(); });

  Serial << "PE32CO2: Starting mainloop...\r\n";
}


/**
 * SoC continuous loop.
 * Calls tick for each component.
 */
void loop()
{
  components.each([](Component& component) { component.tick(); });

  if ((millis() - averages_updated) > 20000) {
    update_averages();
    averages_updated = millis();

    if (ccs811) {
      // This doesn't help a damn thing. This eCO2 device is totally useless.
      ccs811->set_environment(average_ctemp, average_humid);
    }

    Serial << "== Averages\t" <<
      (floattype)average_co2 << " (CO2-ppm)\t" <<
      (floattype)average_eco2 << " (eCO2-ppm)\t" <<
      (floattype)average_ctemp << " (temp-C)\t" <<
      (floattype)average_humid << " (humid%)\t" <<
      (floattype)average_tvoc << " (TVOC) ==\r\n";

    // TODO: move this into a function
    net->push_remote("pe32/hud/temp/xwwwform", (
        String("temperature=") + average_ctemp +
        String("&humidity=") + average_humid));

    const SensorFloat& scd30_co2 = vals.get("scd30.co2");
    const SensorFloat& scd40_co2 = vals.get("scd40.co2");

    String co2form = (
        String("uptime=") + millis() +
        String("&co2=") + average_co2 +
        String("&eco2=") + average_eco2 +
        String("&tvoc=") + average_tvoc);
    if (scd30_co2.is_fresh(60)) {
      co2form += String("&co2.scd30=") + scd30_co2;
    }
    if (scd40_co2.is_fresh(60)) {
      co2form += String("&co2.scd40=") + scd40_co2;
    }
    if (!net->push_remote("pe32/hud/co2/xwwwform", co2form)) {
      while(1); // crash! + reboot!
    }
  }
}


/**
 * Update all averages based as a combination of other values.
 */
void update_averages()
{
  static const char *co2_sensors[] = {"scd30.co2", "scd40.co2", NULL};
  static const char *ctemp_sensors[] = {"bme680.ctemp", "scd30.ctemp", "scd40.ctemp", NULL};
  static const char *eco2_sensors[] = {"ccs811.eco2", NULL};
  static const char *humid_sensors[] = {"bme680.humid", "scd30.humid", "scd40.humid", NULL};
  static const char *tvoc_sensors[] = {"ccs811.tvoc", NULL};

  average_co2 = update_average(co2_sensors);
  average_ctemp = update_average(ctemp_sensors);
  average_eco2 = update_average(eco2_sensors);
  average_humid = update_average(humid_sensors);
  average_tvoc = update_average(tvoc_sensors);
}


/**
 * Utility function for averages.
 */
float update_average(const char **sensor_titles)
{
  float sum = 0.0f;
  unsigned used_values = 0;

  for (const char **p = &sensor_titles[0]; *p; ++p) {
    const SensorFloat& f = vals.get(*p);
    if (f.is_fresh(60)) {
      //Serial << "[" << *p << "] is fresh: " << (float)f << " ...\r\n";
      sum += f;
      used_values += 1;

      // WORKAROUND: BME680 is near hot components! Subtract (at least) 2 degrees..
      // BUG: The SCD30 component uses a heat source too and might need some correction..
      if (strcmp(*p, "bme680.ctemp") == 0) {
        sum -= 2;
      }
    }
  }
  if (used_values) {
    return sum / (float)used_values;
  }
  // Return 0 == broken == disabled == bad
  return 0.0; //previous_average;
}

// vim: set ts=8 sw=2 sts=2 et ai:
