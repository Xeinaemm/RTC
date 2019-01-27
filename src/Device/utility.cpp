#include "AZ3166WiFi.h"
#include "RGB_LED.h"
#include "Arduino.h"
#include "AzureIotHub.h"
#include "HTS221Sensor.h"
#include "LSM6DSLSensor.h"
#include "RGB_LED.h"
#include "parson.h"
#include "utility.h"
#include <assert.h>

DevI2C *i2c;
HTS221Sensor *sensor;
LSM6DSLSensor *acc_gyro;
static RGB_LED rgb_led;
static int interval = 5000;
float xMovement;
float yMovement;
float zMovement;
char ap_name[24];

uint8_t color[][3] =
    {
        {255, 0, 0},    // red
        {0, 255, 0},    // green
        {255, 165, 0},  // orange
        {51, 102, 153}, // Azure
};

void SetLED(int red, int green, int blue)
{
    for (int i = 0; i < 10; i++)
    {
        rgb_led.turnOff();
        delay(10);
        rgb_led.setColor(red, green, blue);
        delay(10);
    }
    rgb_led.turnOff();
}

void EmergencyMessageLED()
{
    SetLED(color[0][0], color[0][1], color[0][2]);
}

void WiFiConnectLED()
{
    SetLED(color[1][0], color[1][1], color[1][2]);
}

void AzureConfirmationLED()
{
    SetLED(color[3][0], color[3][1], color[3][2]);
}

void SetupDeviceId()
{
    byte mac[6];
    WiFi.macAddress(mac);
    sprintf(ap_name, "XAMARIN-%02x%02x", mac[4], mac[5]);
}

int GetInterval()
{
    return interval;
}

void parseTwinMessage(DEVICE_TWIN_UPDATE_STATE updateState, const char *message)
{
    JSON_Value *root_value;
    root_value = json_parse_string(message);
    if (json_value_get_type(root_value) != JSONObject)
    {
        if (root_value != NULL)
        {
            json_value_free(root_value);
        }
        LogError("parse %s failed", message);
        return;
    }
    JSON_Object *root_object = json_value_get_object(root_value);

    double val = 0;
    if (updateState == DEVICE_TWIN_UPDATE_COMPLETE)
    {
        JSON_Object *desired_object = json_object_get_object(root_object, "desired");
        if (desired_object != NULL)
        {
            val = json_object_get_number(desired_object, "interval");
        }
    }
    else
    {
        val = json_object_get_number(root_object, "interval");
    }
    if (val > 1)
    {
        interval = (int)val;
        LogInfo(">>>Device twin updated: set interval to %d", interval);
    }
    json_value_free(root_value);
}

void SensorInit()
{
    i2c = new DevI2C(D14, D15);

    acc_gyro = new LSM6DSLSensor(*i2c, D4, D5);
    acc_gyro->init(NULL);
    acc_gyro->enableAccelerator();
    acc_gyro->enableGyroscope();

    sensor = new HTS221Sensor(*i2c);
    sensor->init(NULL);
}

void SetMotionGyroSensor()
{
    int axes[3];
    acc_gyro->getGAxes(axes);
    xMovement = axes[0];
    yMovement = axes[1];
    zMovement = axes[2];
}

float ReadTemperature()
{
    sensor->reset();

    float temperature = 0;
    sensor->getTemperature(&temperature);

    return temperature;
}

float ReadHumidity()
{
    sensor->reset();

    float humidity = 0;
    sensor->getHumidity(&humidity);

    return humidity;
}

void GetTime(char *buffer)
{
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%d/%m/%y %H:%M:%S", timeinfo);
}

void MessageToAzure(int messageId, char *payload, size_t len)
{
    float temperature = ReadTemperature();
    float humidity = ReadHumidity();
    SetMotionGyroSensor();

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "DeviceId", ap_name);
    json_object_set_number(root_object, "MessageId", messageId);

    char buffer[80];
    GetTime(buffer);

    json_object_set_string(root_object, "Timestamp", buffer);
    json_object_set_number(root_object, "Temperature", temperature);
    json_object_set_number(root_object, "Humidity", humidity);
    json_object_set_number(root_object, "X", xMovement);
    json_object_set_number(root_object, "Y", yMovement);
    json_object_set_number(root_object, "Z", zMovement);

    char *serialized_string = json_serialize_to_string(root_value);

    strncpy(payload, serialized_string, len);
    payload[len - 1] = 0;

    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
}