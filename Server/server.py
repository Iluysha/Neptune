import json
from google.cloud import iot_v1
from google.cloud import pubsub_v1

SENSOR_ID = 'sensor'
VALVE_ID = 'valve'
PROJECT_ID = 'iotproject-347817'
CLOUD_REGION = 'europe-west1'
REGISTRY_ID = 'neptune'
SUBSCRIPTION_ID = 'neptune'
ALGORITHM = 'RS256'
MQTT_BRIDGE_HOSTNAME = 'mqtt.googleapis.com'
MQTT_BRIDGE_PORT = 8883


def send_command(client, device_path, command):
    try:
        client.send_command_to_device(request={"name": device_path,
                                               "binary_data": command.encode("utf-8")})
        print("Command sent")
    except:
        print("disconnected")


def sensor_callback(message: pubsub_v1.subscriber.message.Message) -> None:
    try:
        message_json = json.loads(message.data.decode('utf8').replace("'", '"'))
        if "command_type" in message_json:
            if message_json['command_type'] == "TRIGGER" or message_json['command_type'] == "WAKEUP":
                if 1 in message_json['sensors_data'] or \
                        message_json['reason'] == "Wakeup caused by external signal using RTC_CNTL":
                    client = iot_v1.DeviceManagerClient()
                    valve_path = iot_v1.DeviceManagerClient()\
                        .device_path(PROJECT_ID, CLOUD_REGION, REGISTRY_ID, VALVE_ID)
                    send_command(client, valve_path, '{"command_type": "ACTION", "set": 0}')
                    message.ack()
    except:
        print("Command is not in json")


if __name__ == '__main__':
    subscriber = pubsub_v1.SubscriberClient()
    subscription_sensor_path = subscriber.subscription_path(PROJECT_ID, SUBSCRIPTION_ID)
    streaming_pull_future_sensor = subscriber.subscribe(subscription_sensor_path, callback=sensor_callback)
    print(f"Listening for messages on {subscription_sensor_path}..\n")

    with subscriber:
        streaming_pull_future_sensor.result()
