#ifndef UTILITY_H
#define UTILITY_H

void EnterIdleState(void);
void ScreenSetup(void);
void InitWiFi(void);
void SendToAzure(void);
void SetupDeviceId(void);
void GetTime(char *);

void EmergencyMessageLED(void);
void WiFiConnectLED(void);
void AzureConfirmationLED(void);

void parseTwinMessage(DEVICE_TWIN_UPDATE_STATE, const char *);
void MessageToAzure(int, char *, size_t);
void SensorInit(void);
int GetInterval(void);
#endif /* UTILITY_H */