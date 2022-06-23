// Do not remove the include below
#include "water_valve_gcloud.h"

void setup() {

	Serial.begin(115200);
pinMode(VALVE_PIN,OUTPUT);
digitalWrite(VALVE_PIN,LOW);
	setupCloudIoT();

}

// The loop function is called in an endless loop
void loop() {
	mqtt->loop();
	delay(10);  // <- fixes some issues with WiFi stability

	if (!mqttClient->connected()) {
		connect();
	}
}

void commandReceived(String &topic, String &payload) {
	log_i("\ntopic: %s", topic.c_str());
	log_i("\npayload:");
	Serial.println(payload);
	if (topic.equals("/devices/valve/commands")) {
		commandClb(payload);

	}
	if (topic.equals("/devices/valve/config")) {
		configClb(payload);
	}

}

void commandClb(String &payload) {
	StaticJsonDocument<256> doc;

	DeserializationError error = deserializeJson(doc, payload);

	if (error) {
		log_i("deserializeJson() failed: %s", error.c_str());
		return;
	}

	const char *command_type = doc["command_type"]; // "STATUS" or "ACTION"
	if (String(command_type).equals("STATUS")) {
//sending status
		StaticJsonDocument<256> reply;
		reply["command_type"] = "STATUS";
		reply["status"] = 1-digitalRead(VALVE_PIN);

		String output;
		serializeJson(reply, output);

		publishTelemetry(output);
		log_i("Sent status %s", output.c_str());


	} else if (String(command_type).equals("ACTION")) {
		int set = doc["set"]; // 1
		digitalWrite(VALVE_PIN, !set);
//sending status anyway
		StaticJsonDocument<256> reply;
		reply["command_type"] = "STATUS";
		reply["status"] = 1-digitalRead(VALVE_PIN);

		String output;
		serializeJson(reply, output);

		publishTelemetry(output);

		log_i("Sent status and applied value to valve %s", output.c_str());
	}

}

void configClb(String &payload) {
	StaticJsonDocument<256> doc;

	DeserializationError error = deserializeJson(doc, payload);

	if (error) {
		log_i("deserializeJson() failed: %s", error.c_str());
		return;
	}

//	const char *command_type = doc["command_type"]; // "STATUS" or "ACTION"

}
