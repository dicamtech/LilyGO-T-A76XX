#ifndef APP_H
#define APP_H

/**
 * @file      main.ino
 * @author    Ibrahim Hroob (ibrahim.hroub7@gmail.com)
 * @copyright Copyright (c) 2024  Dicam Technology Ltd
 * @date      2024-12-25
 *
 */
#define TINY_GSM_RX_BUFFER          1024 // Set RX buffer to 1Kb

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

#include "utilities.h"
#include <TinyGsmClient.h>
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <ArduinoHttpClient.h> // Include HTTP client library
#include <Preferences.h>

// Hard coded params
#define MAX_SMS_TARGETS 10u
#define SCAN_TIME 5 // Duration of BLE scan in seconds
#define SERVER "86.17.107.206"
#define PORT 8181
#define PATH "/gateway_data"
#define PATH_SERVER_DATA "/server_data"
#define NETWORK_APN "id" 
#define MODEM_TIMEOUT_MS 1000 // Modem timeout in ms
#define SMS_TARGET "+447803669557" //Change the number you want to send sms message
#define GPS_MAX_RETRY 5
#define SMS_MAX_RETRY 10

#define uS_TO_S_FACTOR      1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP       20          /* Time ESP32 will go to sleep (in seconds) */

// Struct to hold all gnss data 
typedef struct{
    float lat2;
    float lon2;
    float speed2;
    float alt2;
    int vsat2;
    int usat2;
    float accuracy2;
    int year2;
    int month2;
    int day2;
    int hour2;
    int min2;
    int sec2;
    uint8_t fixMode;
}GnssData;

// System status params, those params are intended as read only to report the status of the system
typedef struct{
    bool gnss_enabled;
    GnssData gps_data;
    SimStatus sim_status;
    RegStatus reg_status;
    String network_mode;
    String modem_Name;
    String ue_info;
    String ip_address;
    int16_t sq; // signal quality
    int server_status_code;
    bool sms_send_res[MAX_SMS_TARGETS];  
    bool sms_read_res;
    float modem_cpu_temp;
    bool is_gprs_connected;
    String netwrok_operator;
}SystemStatus;

typedef struct{
    int ble_scan_time;
    // Server struct 
    struct{
        String address;
        String path;
        int port;
    }server;
    String network_apn;
    String sms_targets[MAX_SMS_TARGETS];
    String sms_msg;
}SystemParams;


// TODO: watchdog & pins status at startup 

class BrooderAlarmGateway{
public:
    BrooderAlarmGateway();
    void Init();
    void Run();

private:
    void InitParams();

    void InitModem();
    void BeginModemSerial();
    void EnableModemPower();
    void CheckSimStatus();
    void CheckNetworkStatus();
    void SetModemNetworkMode();
    void GetModemName();
    void SetSMSToTextMode();
    void SetNetworkAPN();
    void EnableModemGps();

    bool CheckSimCardStatus();
    bool WaitForNetworkConnection();
    bool ConnectToGprs();


    void EnterModemSleepMode();
    bool TestModemConnection();
    void WakeModemFromSleep();
    void SwitchModemOff();

    void EnterEspDeepSleepMode(uint8_t time_to_sleep_sec);

    void InitBle();
    void ScanBle();

    void UpdateGpsData();

    void SendDataToServer(const char* payload);
    void ReadDataFromServer();
    
    bool SendSMS(const String& number, const String& msg_str);
    String ReadSMS();

    SystemParams system_params;
    SystemStatus system_status;
    Preferences preferences;

};



#ifndef TINY_GSM_FORK_LIBRARY
#error "No correct definition detected, Please copy all the [lib directories](https://github.com/Xinyuan-LilyGO/LilyGO-T-A76XX/tree/main/lib) to the arduino libraries directory , See README"
#endif


#endif // APP_H