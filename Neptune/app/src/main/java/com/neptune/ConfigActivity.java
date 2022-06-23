package com.neptune;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import com.google.api.client.http.javanet.NetHttpTransport;
import com.google.api.client.json.gson.GsonFactory;
import com.google.api.services.cloudiot.v1.CloudIot;
import com.google.api.services.cloudiot.v1.model.DeviceConfig;
import com.google.api.services.cloudiot.v1.model.ModifyCloudToDeviceConfigRequest;
import com.google.auth.http.HttpCredentialsAdapter;
import com.google.auth.oauth2.GoogleCredentials;

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Base64;
import java.util.List;
import java.util.Objects;

public class ConfigActivity extends AppCompatActivity {

    private static final String SENSOR_ID = "sensor";
    private static final String PROJECT_ID = "iotproject-347817";
    private static final String CLOUD_REGION = "europe-west1";
    private static final String REGISTRY_ID = "neptune";

    private static CloudIot service;

    Integer inputReportTimeoutMS;

    Integer inputStayAwakeMS;

    Integer inputWakeUpPeriodMS;

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

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_conf);
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    public void setConfig(View view) {
        try {
            inputReportTimeoutMS = Integer.parseInt(((EditText)
                    findViewById(R.id.inputReportTimeoutMS))
                    .getText().toString());

            inputStayAwakeMS = Integer.parseInt(((EditText)
                    findViewById(R.id.inputStayAwakeMS))
                    .getText().toString());

            inputWakeUpPeriodMS = Integer.parseInt(((EditText)
                    findViewById(R.id.inputWakeUpPeriodMS))
                    .getText().toString());


            final String devicePath =
                    String.format(
                            "projects/%s/locations/%s/registries/%s/devices/%s",
                            PROJECT_ID, CLOUD_REGION, REGISTRY_ID, SENSOR_ID);

            ModifyCloudToDeviceConfigRequest req = new ModifyCloudToDeviceConfigRequest();
            (new Thread(() -> {
                List<DeviceConfig> deviceConfigs =
                        null;
                try {
                    deviceConfigs = service
                            .projects()
                            .locations()
                            .registries()
                            .devices()
                            .configVersions()
                            .list(devicePath)
                            .execute()
                            .getDeviceConfigs();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                assert deviceConfigs != null;
                req.setVersionToUpdate((long) deviceConfigs.get(0).getVersion());
            })).start();

            String data = "{ 'command_typ': 'CONFIG', " +
                    "'stayAwakeMS': " + inputReportTimeoutMS + ", " +
                    "'wakeUpPeriodMS': " + inputStayAwakeMS + ", " +
                    "'reportTimeoutMS': " + inputWakeUpPeriodMS + "}";

            String encPayload = Base64.getEncoder().encodeToString(data
                    .getBytes(StandardCharsets.UTF_8.name()));
            req.setBinaryData(encPayload);

            (new Thread(() -> {
                DeviceConfig config =
                        null;
                try {
                    config = service
                            .projects()
                            .locations()
                            .registries()
                            .devices()
                            .modifyCloudToDeviceConfig(devicePath, req)
                            .execute();
                } catch (IOException e) {
                    e.printStackTrace();
                }

                assert config != null;
                startActivity(new Intent(this, MainActivity.class));
            })).start();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
