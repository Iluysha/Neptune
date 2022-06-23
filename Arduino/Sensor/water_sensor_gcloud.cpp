// Do not remove the include below
#include "water_sensor_gcloud.h"

void setup() {

	Serial.begin(115200);

	setupCloudIoT();
	mqtt->loop();
		delay(10);  // <- fixes some issues with WiFi stability

		if (!mqttClient->connected()) {
			connect();
		}
	//delay(2000);
	report_wakeup_info(print_wakeup_info(get_wakeup_info()));
	setup_sleep(devConfig);
}

void loop() {
	mqtt->loop();
	delay(10);  // <- fixes some issues with WiFi stability

	if (!mqttClient->connected()) {
		connect();
	}
	for (int i = 0;
			i < SENSORS_AMOUNT
					&& (millis() - last_report_time) > devConfig.reportTimeoutMS;
			i++) {
		if (digitalRead(SENSORS[i])) {
			StaticJsonDocument<256> doc;
			doc["command_type"] = "TRIGGER";
			doc["reason"] = "Leakage detected while awake";
			JsonArray data = doc.createNestedArray("sensors_data");
			for (int i = 0; i < SENSORS_AMOUNT; i++) {
				data.add(digitalRead(SENSORS[i]));
			}
			String output;
			serializeJson(doc, output);
			publishTelemetry(output);
			last_report_time = millis();
			//log_i("%s",output.c_str());
			log_i("sending telemetry: %d", SENSORS[i]);
			return;

		}
	}
	if (ready_to_sleep()) {
		esp_deep_sleep_start();
	}
}

WakeUpInfo get_wakeup_info() {
	WakeUpInfo wakeupinfo;
	wakeupinfo.reason = esp_sleep_get_wakeup_cause();
	wakeupinfo.gpio = (log(esp_sleep_get_ext1_wakeup_status())) / log(2);
	return wakeupinfo;
}

WakeUpInfo print_wakeup_info(WakeUpInfo wakeupinfo) {
	switch (wakeupinfo.reason) {
	case ESP_SLEEP_WAKEUP_EXT0:
		log_d("Wakeup caused by external signal using RTC_IO");
		break;
	case ESP_SLEEP_WAKEUP_EXT1:
		log_d("Wakeup caused by external signal using RTC_CNTL");
		break;
	case ESP_SLEEP_WAKEUP_TIMER:
		log_d("Wakeup caused by timer");
		break;
	case ESP_SLEEP_WAKEUP_TOUCHPAD:
		log_d("Wakeup caused by touchpad");
		break;
	case ESP_SLEEP_WAKEUP_ULP:
		log_d("Wakeup caused by ULP program");
		break;
	default:
		log_d("Wakeup was not caused by deep sleep: %d\n", wakeupinfo.reason);
		break;
	}
	//log_i("sending telemetry");
	return wakeupinfo;

}

WakeUpInfo report_wakeup_info(WakeUpInfo wakeupinfo) {
	String reason;
	switch (wakeupinfo.reason) {
	case ESP_SLEEP_WAKEUP_EXT0:
		reason = "Wakeup caused by external signal using RTC_IO";
		break;
	case ESP_SLEEP_WAKEUP_EXT1:
		reason = "Wakeup caused by external signal using RTC_CNTL";
		break;
	case ESP_SLEEP_WAKEUP_TIMER:
		reason = "Wakeup caused by timer";
		break;
	case ESP_SLEEP_WAKEUP_TOUCHPAD:
		reason = "Wakeup caused by touchpad";
		break;
	case ESP_SLEEP_WAKEUP_ULP:
		reason = "Wakeup caused by ULP program";
		break;
	default:
		reason = "Wakeup was not caused by deep sleep";
		break;
	}
	StaticJsonDocument<256> doc;

	doc["command_type"] = "WAKEUP";
	doc["reason"] = reason;

	JsonArray data = doc.createNestedArray("sensors_data");
	for (int i = 0; i < SENSORS_AMOUNT; i++) {
		data.add(digitalRead(SENSORS[i]));
	}
	String output;
	serializeJson(doc, output);
	publishTelemetry(output);
	log_i("sending telemetry");
	return wakeupinfo;

}

void commandReceived(String &topic, String &payload) {
	Serial.print("\ntopic:");
	Serial.println(topic);
	Serial.print("\npayload:");
	Serial.println(payload);
	if (topic.equals("/devices/sensor/commands")) {
		commandClb(payload);

	}
	if (topic.equals("/devices/sensor/config")) {
		configClb(payload);
	}
}

void setup_sleep(DevConfig devconfig) {
	uint64_t bitmask = 0;
	for (int i = 0; i < SENSORS_AMOUNT; i++) {
		bitmask += SENSORS_BIT_MASKS[i];
	}
	esp_sleep_enable_ext1_wakeup(bitmask, ESP_EXT1_WAKEUP_ANY_HIGH);
	esp_sleep_enable_timer_wakeup(devconfig.wakeUpPeriodMS * 1000);
}
bool ready_to_sleep() {
	if (millis() - last_report_time > devConfig.stayAwakeMS) {
		return 1;
	}
	return 0;
}

void commandClb(String &payload) {
	StaticJsonDocument<256> doc;

	DeserializationError error = deserializeJson(doc, payload);

	if (error) {
		log_i("deserializeJson() failed: %s", error.c_str());
		return;
	}

//	const char *command_type = doc["command_type"]; // "STATUS" or "ACTION"
//	if (String(command_type).equals("STATUS")) {
////sending status
//		StaticJsonDocument<256> reply;
//		String output;
//		reply["command_type"] = "STATUS";
//		reply["status"] = digitalRead(VALVE_PIN);
//		serializeJson(reply, output);
//		publishTelemetry(output);
//		log_i("Sent status");
//	} else if (String(command_type).equals("ACTION")) {
//		int set = doc["set"]; // 1
//		digitalWrite(VALVE_PIN, set);
////sending status anyway
//		StaticJsonDocument<256> reply;
//		String output;
//		reply["command_type"] = "STATUS";
//		serializeJson(reply, output);
//		publishTelemetry(output);
//	log_i("Sent status");
//	}

}

void configClb(String &payload) {
	StaticJsonDocument<256> doc;

	DeserializationError error = deserializeJson(doc, payload);

	if (error) {
		log_i("deserializeJson() failed: %s", error.c_str());
		return;
	}
	const char *command_type = doc["command_type"]; // "CONFIG"
	if (String(command_type).equals("CONFIG")) {
		devConfig.stayAwakeMS = doc["stayAwakeMS"]; // 1111111111111
		devConfig.wakeUpPeriodMS = doc["wakeUpPeriodMS"]; // 1.111111111111111e+25
		devConfig.reportTimeoutMS = doc["reportTimeoutMS"]; // 1111111111
		publishTelemetry(payload);
	}
	//	const char *command_type = doc["command_type"]; // "STATUS" or "ACTION"

}
