// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _water_valve_gcloud_H_
#define _water_valve_gcloud_H_
#include "Arduino.h"
//add your includes for the project water_sensor_gcloud here
#include "ArduinoJson.h"
#if defined(ARDUINO_SAMD_MKR1000) or defined(ESP8266)
#define __SKIP_ESP32__
#endif

#if defined(ESP32)
#define __ESP32_MQTT_H__
#endif

//end of add your includes here


//add your function definitions for the project water_sensor_gcloud here


unsigned long last_report_time=0;
uint8_t VALVE_PIN =BUILTIN_LED;

void commandReceived(String &topic, String &payload);

void commandClb(String &payload);
void configClb(String &payload);

#ifdef __ESP32_MQTT_H__
#include "esp32-mqtt.h"
#endif
//Do not add code below this line
#endif /* _water_valve_gcloud_H_ */
