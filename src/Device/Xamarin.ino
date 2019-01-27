#include "AZ3166WiFi.h"
#include "OLEDDisplay.h"
#include "AzureIotHub.h"
#include "DevKitMQTTClient.h"
#include "DevkitDPSClient.h"
#include "SystemTickCounter.h"
#include "utility.h"
#include "config.h"

enum STATUS
{
  Idle,
  SendingToAzure
};

int wifiStatus = WL_DISCONNECTED;
static STATUS status = Idle;
static bool has_main_WiFi = false;
int message_count = 1;
static bool message_sending = true;
static uint64_t send_interval_ms;
char packetBuffer[MESSAGE_MAX_LEN];

const char *Global_Device_Endpoint = "";
const char *ID_Scope = "";
const char *registrationId = "";
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities

void EnterIdleState()
{
  ScreenSetup();
  status = Idle;
}

void ScreenSetup()
{
  Screen.print(0, "DevKit");
  Screen.print(1, " ");
  Screen.print(2, " ");
  has_main_WiFi ? Screen.print(3, "> WiFi on") : Screen.print(3, "> WiFi off");
}

void InitWiFi()
{
  has_main_WiFi = false;
  Screen.print(1, "Connecting...");
  wifiStatus = WiFi.begin();
  while (wifiStatus != WL_CONNECTED)
  {
    wifiStatus = WiFi.begin();
    return;
  }
  WiFiConnectLED();
  has_main_WiFi = true;
  EnterIdleState();
}

void SendToAzure()
{
  if (message_sending && (int)(SystemTickCounterRead() - send_interval_ms) >= GetInterval())
  {
    Screen.print(1, "Sending...");
    MessageToAzure(message_count++, packetBuffer, MESSAGE_MAX_LEN);
    EVENT_INSTANCE *message = DevKitMQTTClient_Event_Generate(packetBuffer, MESSAGE);
    DevKitMQTTClient_SendEventInstance(message);
    send_interval_ms = SystemTickCounterRead();
  }
  else
  {
    DevKitMQTTClient_Check();
  }
  ScreenSetup();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Callbacks
static void MessageCallback(const char *payLoad, int size)
{
  char buff[MESSAGE_MAX_LEN];
  snprintf(buff, MESSAGE_MAX_LEN, "%s", payLoad);
  Screen.print(1, "Msg arrived:");
  Screen.print(2, buff);
  EmergencyMessageLED();
  delay(5000);
}

int DeviceMethodCallback(const char *methodName, const unsigned char *payload, int size, unsigned char **response, int *response_size)
{
  LogInfo("Try to invoke method %s", methodName);
  const char *responseMessage = "\"Successfully invoke device method\"";
  int result = 200;

  if (strcmp(methodName, "start") == 0)
  {
    message_sending = true;
  }
  else if (strcmp(methodName, "stop") == 0)
  {
    message_sending = false;
  }
  else
  {
    LogInfo("No method %s found", methodName);
    responseMessage = "\"No method found\"";
    result = 404;
  }

  *response_size = strlen(responseMessage);
  *response = (unsigned char *)malloc(*response_size);
  strncpy((char *)(*response), responseMessage, *response_size);

  return result;
}

void DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, int size)
{
  char *temp = (char *)malloc(size + 1);
  if (temp == NULL)
  {
    return;
  }
  memcpy(temp, payLoad, size);
  temp[size] = '\0';
  parseTwinMessage(updateState, temp);
  EmergencyMessageLED();
  free(temp);
}

static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result)
{
  if (result == IOTHUB_CLIENT_CONFIRMATION_OK)
  {
    AzureConfirmationLED();
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino sketch
void setup()
{
  Screen.init();
  ScreenSetup();
  SetupDeviceId();
  pinMode(USER_BUTTON_B, INPUT);
  pinMode(USER_BUTTON_A, INPUT);
  Serial.begin(115200);
  InitWiFi();
  SensorInit();

  DevkitDPSClientStart(Global_Device_Endpoint, ID_Scope, registrationId);
  DevKitMQTTClient_Init(true);
  DevKitMQTTClient_SetMessageCallback(MessageCallback);
  DevKitMQTTClient_SetDeviceMethodCallback(DeviceMethodCallback);
  DevKitMQTTClient_SetDeviceTwinCallback(DeviceTwinCallback);
  DevKitMQTTClient_SetSendConfirmationCallback(SendConfirmationCallback);
}

void DoIdle()
{
  if (digitalRead(USER_BUTTON_B) == HIGH && digitalRead(USER_BUTTON_A) == HIGH)
  {
    status = SendingToAzure;
    return;
  }
}

void loop()
{
  switch (status)
  {
  case Idle:
    DoIdle();
    break;

  case SendingToAzure:
    SendToAzure();
    break;
  }
  delay(10);
}