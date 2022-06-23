package com.neptune;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import com.google.api.client.http.javanet.NetHttpTransport;
import com.google.api.client.json.gson.GsonFactory;
import com.google.api.gax.core.FixedCredentialsProvider;
import com.google.api.services.cloudiot.v1.CloudIot;
import com.google.api.services.cloudiot.v1.model.SendCommandToDeviceRequest;
import com.google.auth.http.HttpCredentialsAdapter;
import com.google.auth.oauth2.GoogleCredentials;
import com.google.auth.oauth2.ServiceAccountCredentials;
import com.google.cloud.pubsub.v1.AckReplyConsumer;
import com.google.cloud.pubsub.v1.MessageReceiver;
import com.google.cloud.pubsub.v1.Subscriber;
import com.google.pubsub.v1.ProjectSubscriptionName;
import com.google.pubsub.v1.PubsubMessage;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class MainActivity extends AppCompatActivity {

    boolean valve_status;

    private static final String VALVE_ID = "valve";
    private static final String PROJECT_ID = "iotproject-347817";
    private static final String CLOUD_REGION = "europe-west1";
    private static final String REGISTRY_ID = "neptune";
    private static final String SUBSCRIPTION_ID = "app";

    private static CloudIot service;

    static {
        try {
            service = new CloudIot.Builder(new NetHttpTransport(),
                    GsonFactory.getDefaultInstance(),
                    new HttpCredentialsAdapter(GoogleCredentials
                            .fromStream(Objects
                                    .requireNonNull(MainActivity.class.getClassLoader())
                                    .getResourceAsStream("res/raw/google.json"))))
                    .setApplicationName("Neptune")
                    .build();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    protected Map<String, String> stringToMap(String message) {
        try {
            Map<String, String> messageMap = new HashMap<>();
            String[] parts = message
                    .replace("\"", "")
                    .replace("{", "")
                    .replace("}", "")
                    .replace("[", "[@")
                    .split("@");
            String[] pairs = (parts[0] + (parts.length > 1 ?
                    parts[1].replace(",", ";") : ""))
                    .replace("\"", "")
                    .replace("{", "")
                    .replace("}", "")
                    .replace(" ", "")
                    .split(",");
            for (String pair : pairs) {
                String[] keyValue = pair.split(":");
                messageMap.put(keyValue[0], keyValue[1]);
            }
            return messageMap;
        } catch (Exception e) {
            return new HashMap<>();
        }
    }

    @SuppressLint("SetTextI18n")
    protected void waterDetected() {
        runOnUiThread(() -> ((TextView) findViewById(R.id.sensor_status))
                .setText("Water detected"));
    }

    @SuppressLint("SetTextI18n")
    protected void valveStatus() {
        runOnUiThread(() -> {
            ((TextView) findViewById(R.id.valve_status))
                    .setText("Valve status: " + (valve_status ? "open" : "closed"));
            if(!valve_status) {
                ((TextView) findViewById(R.id.sensor_status))
                        .setText("");
            }
        });
    }

    protected void listen() {
        Subscriber subscriber = null;
        ProjectSubscriptionName subscriptionName = ProjectSubscriptionName
                .of(PROJECT_ID, SUBSCRIPTION_ID);
        try {
            subscriber = Subscriber.newBuilder(subscriptionName, receiver)
                    .setCredentialsProvider(FixedCredentialsProvider
                            .create(ServiceAccountCredentials
                                    .fromStream(this.getClassLoader()
                                            .getResourceAsStream(
                                                    "res/raw/google.json"))))
                    .build();
        } catch (IOException e) {
            e.printStackTrace();
        }

        assert subscriber != null;
        subscriber.startAsync().awaitRunning();
        System.out.printf("Listening for messages on %s:\n", subscriptionName);
    }

    @SuppressLint("SetTextI18n")
    MessageReceiver receiver =
            (PubsubMessage message, AckReplyConsumer consumer) -> {
                System.out.println("MESSAGE RECEIVED: " + message.getData().toStringUtf8());

                Map<String, String> messageMap = stringToMap(message.getData().toStringUtf8());
                if (!messageMap.isEmpty()) {
                    switch (Objects.requireNonNull(messageMap.get("command_type"))) {
                        case "WAKEUP":
                            if(Objects.equals(messageMap.get("reason"),
                                    "WakeupcausedbyexternalsignalusingRTC_CNTL") ||
                                    Objects.requireNonNull(messageMap.get("sensors_data"))
                                            .contains("1")) {
                                waterDetected();
                            }
                        case "TRIGGER":
                            waterDetected();
                            break;
                        case "STATUS":
                            valve_status = Integer.parseInt(
                                    Objects.requireNonNull(messageMap.get("status"))) == 1;
                            valveStatus();
                        default:
                    }
                    consumer.ack();
                }
            };

    @SuppressLint("SetTextI18n")
    @RequiresApi(api = Build.VERSION_CODES.O)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        sendCommand("{'command_type': 'STATUS'}");
        valveStatus();

        listen();
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    protected static void sendCommand(String data) {
            try  {
                final String devicePath = String.format(
                                "projects/%s/locations/%s/registries/%s/devices/%s",
                                PROJECT_ID, CLOUD_REGION, REGISTRY_ID, VALVE_ID);
                SendCommandToDeviceRequest req = new SendCommandToDeviceRequest();
                req.setBinaryData(Base64.getEncoder().encodeToString(data.getBytes(StandardCharsets.UTF_8.name())));
                System.out.printf("Sending command " + data + " to %s%n",
                        devicePath.split("registries/")[1]);

                (new Thread(() -> {
                    try {
                        service
                                .projects()
                                .locations()
                                .registries()
                                .devices()
                                .sendCommandToDevice(devicePath, req)
                                .execute();
                        System.out.println("Command response: sent");
                    } catch (Exception e) {
                        System.out.println("Device doesn't have connection!");
                    }
                })).start();
            } catch (Exception e) {
                e.printStackTrace();
            }
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    public void setConfig(View view) {
        startActivity(new Intent(this, ConfigActivity.class));
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    @SuppressLint("SetTextI18n")
    public void switchValve(View view) {
        sendCommand("{'command_type': 'ACTION', 'set': " + (valve_status ? "0" : "1") + "}");
    }
}