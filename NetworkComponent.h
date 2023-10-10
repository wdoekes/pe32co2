#ifndef INCLUDED_PE32HUD_NETWORKCOMPONENT_H
#define INCLUDED_PE32HUD_NETWORKCOMPONENT_H

#include "pe32co2.h"

#include "component.h"

class MqttClient;
class WiFiClient;

class NetworkComponent : public Component {
public:
    NetworkComponent();

    void setup();
    void tick();

    bool push_remote(String topic, String formdata);

private:
    void handle_wifi_state_change(uint16_t wifistatus);
    void ensure_mqtt();

private:
    static constexpr unsigned long m_interval = 5000;
    unsigned long m_lastact;
    unsigned long m_wifidowntime;
    String m_guid;
    uint16_t m_wifistatus;
    // NOTE: We need a WiFiClient for _each_ component that does network
    // connections (httpclient and mqttclient), otherwise using one will
    // disconnect the TCP session of the other.
    // See also: https://forum.arduino.cc/t/simultaneous-mqtt-and-http-post/430361/7
    WiFiClient *m_mqttbackend;
    MqttClient *m_mqttclient;

};

#endif //INCLUDED_PE32HUD_NETWORKCOMPONENT_H
