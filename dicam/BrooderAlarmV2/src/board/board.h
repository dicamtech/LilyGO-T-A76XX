#ifndef _BOARD_H_
#define _BOARD_H_

#include "utilities.h"
#include "ruuvitag.h"
#include <TinyGsmClient.h>
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <NimBLEAdvertisedDevice.h>
#include <ArduinoHttpClient.h> // Include HTTP client library
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

#include "../common.h"

// Class responsible for managing the board
class TBoardMgr {
public:

    void InitModem();
    void SwitchModemOff();

    void InitBle();
    void RunBle();

    bool SendSMS(const String& number, const String& msg_str);
    void ReadSMS(String& msg_str);

    void EnableHotspot();
    void DisableHotspot();
    void RunHotspot();
    bool IsApnSet() const { return apn_set_flag; }

    static void HotspotHandleRoot();
    static void HotspotHandelSave();

    void SetApn(const String& apn) { this->apn = apn; }
    void SetApnUsername(const String& apn_username) { this->apn_username = apn_username; }
    void SetApnPassword(const String& apn_password) { this->apn_password = apn_password; }

    void GetApn(String& apn) const { apn = this->apn; }
    void GetApnUsername(String& apn_username) const { apn_username = this->apn_username; }
    void GetApnPassword(String& apn_password) const { apn_password = this->apn_password; }

    int GetSignalQuality() const { return signal_quality; }
    void GetIpAddress(String& ip_address) const { ip_address = this->ip_address; }
    void GetNetworkMode(String& network_mode) const { network_mode = this->network_mode; }
    void GetModemName(String& modem_Name) const { modem_Name = this->modem_Name; }

private:
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
    bool TestModemConnection();
    
    void EnterEspDeepSleepMode(uint8_t time_to_sleep_sec);

    void ScanBle();
    void ParseBleData();

private:
    NimBLEScan* pBLEScan;

    // Variables to hold APN settings
    static String apn;
    static String apn_username;
    static String apn_password;
    static bool apn_set_flag;

    SimStatus sim_status;
    int signal_quality;
    String ip_address;
    String network_mode;
    String modem_Name;
};

#ifndef TINY_GSM_FORK_LIBRARY
#error "No correct definition detected. Please copy all the [lib directories](https://github.com/Xinyuan-LilyGO/LilyGO-T-A76XX/tree/main/lib) to the Arduino libraries directory. See README."
#endif

#endif  // _BOARD_H_
