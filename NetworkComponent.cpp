#include "NetworkComponent.h"

#if defined(ARDUINO_ARCH_ESP32)
# include <WiFi.h>
# define HAVE_ESPWIFI
#elif defined(ARDUINO_ARCH_ESP8266)
# include <ESP8266WiFi.h>
# define HAVE_ESPWIFI
#endif

#include <ArduinoMqttClient.h>

#include "arduino_secrets.h" // XXX

NetworkComponent::NetworkComponent()
{
#ifdef HAVE_ESPWIFI
    m_wifistatus = WL_DISCONNECTED;
    m_mqttbackend = new WiFiClient();
    if (m_mqttbackend) {
        m_mqttclient = new MqttClient(*m_mqttbackend);
    }
#endif
}

void NetworkComponent::setup()
{
//    Device.set_guid(String("EUI48:") + WiFi.macAddress());
//    Device.set_alert(Device::INACTIVE_WIFI);
    m_guid = String("EUI48:") + WiFi.macAddress();
    m_wifidowntime = millis();
#ifdef HAVE_ESPWIFI
    WiFi.persistent(false);         // false is default, we don't need to save to flash
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);   // we don't need this, we do it manually?
    handle_wifi_state_change(WL_IDLE_STATUS);
    m_wifistatus = WL_IDLE_STATUS;
    m_wifidowntime = m_lastact = millis();
    // Do not forget setId(). Some MQTT daemons will reject id-less connections.
    m_mqttclient->setId(m_guid.substring(0, 23));
#endif
}

void NetworkComponent::tick()
{
#ifdef HAVE_ESPWIFI
    wl_status_t wifistatus;
    if ((millis() - m_lastact) >= 3000 && (
                (wifistatus = WiFi.status()) != WL_CONNECTED || wifistatus != m_wifistatus)) {
        if (m_wifistatus != wifistatus) {
            handle_wifi_state_change(wifistatus);
            m_wifistatus = wifistatus;
            // Don't set m_lastact. We'll want to run WL_CONNECTED code below.
        } else if (m_wifistatus != WL_CONNECTED && (millis() - m_wifidowntime) > 5000) {
            wifistatus = WL_IDLE_STATUS;
            handle_wifi_state_change(wifistatus);
            m_wifistatus = wifistatus;
            m_lastact = millis();
        }
    }
#endif
    if (m_wifistatus == WL_CONNECTED && (millis() - m_lastact) >= m_interval) {
        const unsigned char *bssid = WiFi.BSSID();
        Serial << F("NetworkComponent: RSSI: ") << WiFi.RSSI() << F(", BSSID: 0x");
        Serial.print(bssid[0], HEX);
        Serial.print(bssid[1], HEX);
        Serial.print(bssid[2], HEX);
        Serial.print(bssid[3], HEX);
        Serial.print(bssid[4], HEX);
        Serial.print(bssid[5], HEX);
        Serial << F("\r\n");
        ensure_mqtt();
        m_lastact = millis();  // after poll, so we don't hammer on failure
    }
}

bool NetworkComponent::push_remote(String topic, String formdata)
{
    if (m_mqttclient->connected()) {
        Serial << F("NetworkComponent: push: ") << topic << F(" :: ") <<  // (idefix)
            F("device_id=") << m_guid << F("&") << formdata << F("\r\n");
        m_mqttclient->beginMessage(topic);
        m_mqttclient->print(F("device_id="));
        m_mqttclient->print(m_guid);
        m_mqttclient->print(F("&"));
        m_mqttclient->print(formdata);
        m_mqttclient->endMessage();
        return true;
    } else {
      return false;
    }
}

#ifdef HAVE_ESPWIFI
void NetworkComponent::handle_wifi_state_change(uint16_t wifistatus)
{
    // FIXME: translate wifistatus from number to something readable
    Serial << F("NetworkComponent: Wifi state ") << m_wifistatus << F(" -> ") << wifistatus << F("\r\n");

    if (m_wifistatus == WL_CONNECTED) {
        m_wifidowntime = millis();
    }
    String downtime(String("") + ((millis() - m_wifidowntime) / 1000) + " downtime");

    switch (wifistatus) {
        case WL_IDLE_STATUS:
//            Device.set_alert(Device::INACTIVE_WIFI);
//            Device.set_error(F("Wifi connecting"), downtime);
            WiFi.disconnect(true, true);
#ifdef SECRET_WIFI_BSSID
            // Speed up wifi connect, especially for poor (<= -70 RSSI) connections.
            if ((millis() - m_wifidowntime) < 30000) {
                const uint8_t bssid[6] = SECRET_WIFI_BSSID;
                WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASS, 0, bssid, true);
                Serial << F("NetworkComponent: Wifi connecting (with preset BSSID)...\r\n");
            } else {
                WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASS);
                Serial << F("NetworkComponent: Wifi connecting...\r\n");
            }
#else
            WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASS);
            Serial << F("NetworkComponent: Wifi connecting...\r\n");
#endif
            break;
        case WL_CONNECTED:
//            Device.clear_alert(Device::INACTIVE_WIFI);
            break;
        case WL_NO_SSID_AVAIL:
        case WL_CONNECT_FAILED:
        case WL_DISCONNECTED:
//            Device.set_alert(Device::INACTIVE_WIFI);
//            Device.set_error(String(F("Wifi state ")) + wifistatus, downtime);
            break;
#ifdef WL_CONNECT_WRONG_PASSWORD
        case WL_CONNECT_WRONG_PASSWORD:
//            Device.set_alert(Device::INACTIVE_WIFI);
//            Device.set_error(F("Wifi wrong creds."), downtime);
            break;
#endif
        default:
//            Device.set_alert(Device::INACTIVE_WIFI);
//            Device.set_error(String(F("Wifi unknown ")) + wifistatus, downtime);
            break;
    }
#ifdef DEBUG
    Serial << F("  --NetworkComponent: Wifi values BEGIN\r\n");
    WiFi.printDiag(Serial);  // FIXME/XXX: beware, shows password on serial output
    Serial << F("  --NetworkComponent: Wifi values END\r\n");
#endif
}
#endif

void NetworkComponent::ensure_mqtt()
{
    m_mqttclient->poll();
    if (!m_mqttclient->connected()) {
        if (m_mqttclient->connect(SECRET_MQTT_BROKER, SECRET_MQTT_PORT)) {
            Serial << F("NetworkComponent: MQTT connected to " SECRET_MQTT_BROKER "\r\n");
        } else {
            Serial << F("NetworkComponent: MQTT connection to "
                SECRET_MQTT_BROKER " failed: ") <<  // (idefix)
                m_mqttclient->connectError() << F("\r\n");
        }
    }
}
