// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _water_sensor_gcloud_H_
#define _water_sensor_gcloud_H_
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
struct DevConfig{
	unsigned long stayAwakeMS;
	unsigned long wakeUpPeriodMS;
	unsigned long reportTimeoutMS;
};

RTC_DATA_ATTR DevConfig devConfig={
		30000,
		3600000,
		10000
};
unsigned long last_report_time=0;
const uint8_t SENSORS_AMOUNT = 4;
const uint8_t SENSORS[SENSORS_AMOUNT] = { 13,12, 27, 4 };
const uint64_t SENSORS_BIT_MASKS[SENSORS_AMOUNT]={
		0b10000000000000,
		//0b1000000000000000,
		0b1000000000000,
		0b1000000000000000000000000000,
		0b10000
};
struct WakeUpInfo {
	esp_sleep_wakeup_cause_t reason;
	uint64_t gpio;
};
WakeUpInfo get_wakeup_info();
WakeUpInfo print_wakeup_info(
		WakeUpInfo wakeupinfo);
WakeUpInfo report_wakeup_info(
		WakeUpInfo wakeupinfo);
void commandReceived(String &topic, String &payload);
void commandClb(String &payload);
void configClb(String &payload);
void setup_sleep(DevConfig devconfig);
bool ready_to_sleep();
#ifdef __ESP32_MQTT_H__
#include "esp32-mqtt.h"
#endif
//Do not add code below this line
#endif /* _water_sensor_gcloud_H_ */
