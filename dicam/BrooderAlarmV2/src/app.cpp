/**
 * @file      main.ino
 * @author    Ibrahim Hroob (ibrahim.hroub7@gmail.com)
 * @copyright Copyright (c) 2024  Dicam Technology Ltd
 * @date      2024-12-25
 *
 */

#include "app.h"

#ifdef DUMP_AT_COMMANDS  // if enabled it requires the streamDebugger lib
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

// Global params
TinyGsmClient client(modem);
HttpClient http(client, SERVER, PORT);
BLEScan *pBLEScan;

// WiFi connection credentials, if unable to connect to gprs
const char wifiSSID[] = "VM4825723";
const char wifiPass[] = "4rcqwxyHpbyg";


#define TOGGLE_BOARD_PWRKEY()   {   \
        digitalWrite(BOARD_PWRKEY_PIN, LOW); \
        delay(100);                          \
        digitalWrite(BOARD_PWRKEY_PIN, HIGH);\
        delay(1000);                         \
        digitalWrite(BOARD_PWRKEY_PIN, LOW); \
    }


BrooderAlarmGateway::BrooderAlarmGateway(){}

void BrooderAlarmGateway::InitParams(){
    // preferences.begin("system-params", false); // Set to "true" for read-only mode.
    // // Save a setting
    // preferences.putInt("my-int", 42);
    // preferences.putFloat("my-float", 3.14);
    // preferences.putString("my-string", "Hello, ESP!");

    // // Retrieve settings
    // int myInt = preferences.getInt("my-int", 0);         // Default is 0 if not set
    // float myFloat = preferences.getFloat("my-float", 0); // Default is 0
    // String myString = preferences.getString("my-string", "Default");

    // Serial.println("Saved Settings:");
    // Serial.println(myInt);
    // Serial.println(myFloat);
    // Serial.println(myString);

    // // Close preferences to free resources
    // preferences.end();
}

void BrooderAlarmGateway::EnableModemPower(){
    Serial.println("Powering on the modem...");
#ifdef BOARD_POWERON_PIN
    pinMode(BOARD_POWERON_PIN, OUTPUT);
    digitalWrite(BOARD_POWERON_PIN, HIGH);
#endif

    // Set modem reset pin ,reset modem
    pinMode(MODEM_RESET_PIN, OUTPUT);
    digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL); delay(100);
    digitalWrite(MODEM_RESET_PIN, MODEM_RESET_LEVEL); delay(2600);
    digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL);

    pinMode(BOARD_PWRKEY_PIN, OUTPUT);
    TOGGLE_BOARD_PWRKEY();

    // Check if the modem is online
    Serial.println("Start modem...");

    int retry = 0;
    while (!modem.testAT(MODEM_TIMEOUT_MS)) {
        Serial.println(".");
        if (retry++ > 10) {
            TOGGLE_BOARD_PWRKEY();
            retry = 0;
        }
    }
    Serial.println();
}

void BrooderAlarmGateway::BeginModemSerial(){
    Serial.println("Initializing modem serial communication...");
    SerialAT.begin(MODEM_BAUDRATE, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);
}

void BrooderAlarmGateway::CheckSimStatus() {
    Serial.println("Checking SIM card status...");
    system_status.sim_status = SIM_ERROR;

    if (!CheckSimCardStatus()) {
        return;
    }

    if (!WaitForNetworkConnection()) {
        return;
    }

    if (!ConnectToGprs()) {
        return;
    }

    Serial.println("System is fully connected.");
}

bool BrooderAlarmGateway::CheckSimCardStatus() {
    for (uint8_t attempt = 0; attempt < SMS_MAX_RETRY; attempt++) {
        system_status.sim_status = modem.getSimStatus();

        if (system_status.sim_status == SIM_READY) {
            Serial.println("SIM card online");
            return true;
        }

        if (system_status.sim_status == SIM_LOCKED) {
            Serial.println("The SIM card is locked. Please unlock the SIM card first.");
            return false;
        }

        Serial.printf("Unable to check SIM status, retry #%d\n", attempt);
        delay(1000);
    }

    Serial.println("SIM card status check failed after maximum retries.");
    return false;
}

bool BrooderAlarmGateway::WaitForNetworkConnection() {
    Serial.print("Waiting for network...");

    for (uint8_t attempt = 0; attempt < SMS_MAX_RETRY; attempt++) {
        if (modem.waitForNetwork()) {
            Serial.println(" success");

            if (modem.isNetworkConnected()) {
                Serial.println("Network connected");
            }

            return true;
        }

        Serial.printf("Network wait failed, retry #%d\n", attempt);
        delay(1000);
    }

    Serial.println("Network wait failed after maximum retries.");
    delay(1000);
    return false;
}

bool BrooderAlarmGateway::ConnectToGprs() {
    Serial.print(F("Connecting to "));
    Serial.print(NETWORK_APN);

    for (uint8_t attempt = 0; attempt < SMS_MAX_RETRY; attempt++) {
        if (modem.gprsConnect(NETWORK_APN, "", "")) {
            Serial.println(" success");

            if (modem.isGprsConnected()) {
                Serial.println("GPRS connected");
            }

            return true;
        }

        Serial.printf("GPRS connection failed, retry #%d\n", attempt);
        delay(1000);
    }

    Serial.println("GPRS connection failed after maximum retries.");
    delay(1000);
    return false;
}


void BrooderAlarmGateway::SetModemNetworkMode(){
    Serial.println("Setting the modem's network mode...");
    //SIM7672G Can't set network mode
#ifndef TINY_GSM_MODEM_SIM7672
    if (!modem.setNetworkMode(MODEM_NETWORK_AUTO)) {
        Serial.println("Set network mode failed!");
    }
    system_status.network_mode = modem.getNetworkModes();
    Serial.print("Current network mode : ");
    Serial.println(system_status.network_mode);
#endif
}

void BrooderAlarmGateway::GetModemName(){
    Serial.println("Retrieving modem name...");
    system_status.modem_Name = "UNKOWN";
    while (1) {
        system_status.modem_Name = modem.getModemName();
        if (system_status.modem_Name == "UNKOWN") {
            Serial.println("Unable to obtain module information normally, try again");
            delay(1000);
        } else if (system_status.modem_Name.startsWith("SIM7670")) {
            while (1) {
                Serial.println("SIM7670 does not support SMS Function");
                delay(1000);
            }
        } else {
            Serial.print("Modem Name:");
            Serial.println(system_status.modem_Name);
            break;
        }
    }

    // Get model info
    modem.sendAT("+SIMCOMATI");
    modem.waitResponse();
}

void BrooderAlarmGateway::SetSMSToTextMode(){
    // Wait PB DONE
    Serial.println("Configuring SMS to text mode...");
    if (!modem.waitResponse(10000UL, "SMS DONE")) {
        Serial.println("Can't wait from sms register ....");
        return;
    }
    // Set SMS system into text mode
    modem.sendAT("+CMGF=1");
    modem.waitResponse(10000);
}

void BrooderAlarmGateway::SetNetworkAPN(){
#ifdef NETWORK_APN
    Serial.printf("Set network apn : %s\n", NETWORK_APN);
    modem.sendAT(GF("+CGDCONT=1,\"IP\",\""), NETWORK_APN, "\"");
    if (modem.waitResponse() != 1) {
        Serial.println("Set network apn error !");
    }
#endif
}

void BrooderAlarmGateway::EnableModemGps(){
    Serial.println("Enabling GPS/GNSS/GLONASS");
    while (!modem.enableGPS(MODEM_GPS_ENABLE_GPIO, MODEM_GPS_ENABLE_LEVEL)) {
        Serial.print(".");
    }
    Serial.println();
    Serial.println("GPS Enabled");

    // Set GPS Baud
    modem.setGPSBaud(MODEM_BAUDRATE);
}

void BrooderAlarmGateway::CheckNetworkStatus(){
    // Check network registration status and network signal status    
    Serial.println("Wait for the modem to register with the network.");
    RegStatus status = REG_NO_RESULT;
    while (status == REG_NO_RESULT || status == REG_SEARCHING || status == REG_UNREGISTERED) {
        status = modem.getRegistrationStatus();
        switch (status) {
        case REG_UNREGISTERED:
        case REG_SEARCHING:
            system_status.sq = modem.getSignalQuality();
            Serial.printf("[%lu] Signal Quality:%d\n", millis() / 1000, system_status.sq);
            // delay(1000);
            break;
        case REG_DENIED:
            Serial.println("Network registration was rejected, please check if the APN is correct");
            return ;
        case REG_OK_HOME:
            Serial.println("Online registration successful");
            break;
        case REG_OK_ROAMING:
            Serial.println("Network registration successful, currently in roaming mode");
            break;
        default:
            Serial.printf("Registration Status:%d\n", status);
            // delay(1000);
            break;
        }
    }
    Serial.println();
    system_status.sq = modem.getSignalQuality();

    if (modem.getSystemInformation(system_status.ue_info)) {
        Serial.print("Inquiring UE system information:");
        Serial.println(system_status.ue_info);
    }

    if (!modem.setNetworkActive()) {
        Serial.println("Enable network failed!");
    }

    system_status.ip_address = modem.getLocalIP();
    Serial.print("Network IP:"); Serial.println(system_status.ip_address);
}

void BrooderAlarmGateway::InitModem(){
    BeginModemSerial();
    EnableModemPower();
    SetModemNetworkMode();
    GetModemName();
    EnableModemGps();
    CheckSimStatus();
    SetSMSToTextMode();
    SetNetworkAPN();
    CheckNetworkStatus();
}

void BrooderAlarmGateway::InitBle(){
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);
}

void BrooderAlarmGateway::ScanBle(){
    BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME, false);
    Serial.printf("Found %d devices.\n", foundDevices.getCount());
    for (auto i = 0; i < foundDevices.getCount(); i++){
        BLEAdvertisedDevice advertisedDevice = foundDevices.getDevice(i);
        std::string manufacturerData = advertisedDevice.getManufacturerData();
        if (!manufacturerData.empty() && manufacturerData[0] == 0x99 && manufacturerData[1] == 0x04) { // RuuviTag company ID
            Serial.printf("RuuviTag found: %s\n", advertisedDevice.toString().c_str());
            // Send the device data to the server
            char payload[256];
            snprintf(payload, sizeof(payload), "{ \"RuuviTag\": \"%s\" }", advertisedDevice.toString().c_str());
            SendDataToServer(payload);
        }
    }
    pBLEScan->clearResults();
    pBLEScan->stop();
}

void BrooderAlarmGateway::SendDataToServer(const char* payload) {
    Serial.println("Sending data to server...");
    Serial.print("Payload: ");
    Serial.println(payload);

    http.beginRequest();
    http.post(PATH);
    http.sendHeader("Content-Type", "application/json");
    http.sendHeader("Content-Length", strlen(payload));
    http.beginBody();
    http.print(payload);
    http.endRequest();

    int statusCode = http.responseStatusCode();
    String response = http.responseBody();

    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);
}

void BrooderAlarmGateway::ReadDataFromServer() {
    Serial.println("Reading data from server...");
    const char resource[] = "/server_data";

    int err = http.get(resource);
    if (err != 0) {
        Serial.println(F("failed to connect"));
        return;
    }
    int statusCode = http.responseStatusCode();
    String response = http.responseBody();

    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);

    if (statusCode == 200) {
        // Process the response if needed
        Serial.println("Data received successfully.");
    } else {
        Serial.println("Failed to read data from server.");
    }
}

#define GD system_status.gps_data
void BrooderAlarmGateway::UpdateGpsData(){
    for(uint8_t i = 0; i < GPS_MAX_RETRY; i++){
        Serial.println("Requesting current GPS/GNSS/GLONASS location");
        if (modem.getGPS(&GD.fixMode, &GD.lat2, &GD.lon2, &GD.speed2, &GD.alt2, &GD.vsat2, &GD.usat2, &GD.accuracy2,
                        &GD.year2, &GD.month2, &GD.day2, &GD.hour2, &GD.min2, &GD.sec2)) {

            Serial.print("FixMode:"); Serial.println(GD.fixMode);
            Serial.print("Latitude:"); Serial.print(GD.lat2, 6); Serial.print("\tLongitude:"); Serial.println(GD.lon2, 6);
            Serial.print("Speed:"); Serial.print(GD.speed2); Serial.print("\tAltitude:"); Serial.println(GD.alt2);
            Serial.print("Visible Satellites:"); Serial.print(GD.vsat2); Serial.print("\tUsed Satellites:"); Serial.println(GD.usat2);
            Serial.print("Accuracy:"); Serial.println(GD.accuracy2);

            Serial.print("Year:"); Serial.print(GD.year2);
            Serial.print("\tMonth:"); Serial.print(GD.month2);
            Serial.print("\tDay:"); Serial.println(GD.day2);

            Serial.print("Hour:"); Serial.print(GD.hour2);
            Serial.print("\tMinute:"); Serial.print(GD.min2);
            Serial.print("\tSecond:"); Serial.println(GD.sec2);

            return;
        } else if (i < (GPS_MAX_RETRY - 1)) {
            Serial.println("Couldn't get GPS/GNSS/GLONASS location, retrying in 15s.");
            for(uint8_t sec = 0; sec < 15; sec++){
                delay(1000);
                Serial.print(".");
            }
            Serial.println();
        } else {
            Serial.println("Failed to get GPS/GNSS/GLONASS location. Exit.");
        }
    } 
}

bool BrooderAlarmGateway::SendSMS(const String& number, const String& msg_str){
    Serial.printf("Sending SMS message to %s \n", number);
    bool res = modem.sendSMS(number, msg_str);
    Serial.println(res ? "OK" : "fail");
    return res;
}

String BrooderAlarmGateway::ReadSMS(){
    String data;
    int8_t res;

    modem.sendAT("+CMGL=\"REC UNREAD\""); // Fetch only unread messages
    res = modem.waitResponse(1000, data); // Wait for the response
    if (res == 1) {
        Serial.print("Unread Messages:");
        data.replace("\r\nOK\r\n", "");
        data.replace("\rOK\r", "");
        data.trim();
        Serial.println(data); // Print the data of unread messages
    } else {
        Serial.print("Failed to fetch unread messages");
    }
    return data;
}

void BrooderAlarmGateway::EnterModemSleepMode() {
    Serial.println("Entering modem sleep mode...");

    // Pull up DTR to put the modem into sleep
    pinMode(MODEM_DTR_PIN, OUTPUT);
    digitalWrite(MODEM_DTR_PIN, HIGH);
    // Set DTR to keep at high level, if not set, DTR will be invalid after ESP32 goes to sleep.
    gpio_hold_en(static_cast<gpio_num_t>(MODEM_DTR_PIN));

#ifdef BOARD_POWERON_PIN
    // Hold BOARD_POWERON_PIN during deep sleep, if defined
    gpio_hold_en(static_cast<gpio_num_t>(BOARD_POWERON_PIN));
#endif

    // Enable deep sleep hold for all configured pins
    gpio_deep_sleep_hold_en();

    // // Enter sleep mode
    // // This command is used to enable UART Sleep or always work. If set to 0, UART always work. If set to 1,
    // // ensure that DTR is pulled high and the module can go to DTR sleep. If set to 2, the module will enter RX
    // // sleep. RX wakeup directly sends data through the serial port (for example: AT) to wake up
    // modem.sendAT("+CSCLK=1");

    // Attempt to enable modem sleep mode
    if (modem.sleepEnable(true)) {
        Serial.println("Modem entered sleep mode.");
    } else {
        Serial.println("Failed to enter modem sleep mode.");
    }

#ifdef MODEM_RESET_PIN
    // Keep it low during the sleep period. If the module uses GPIO5 as reset, 
    // there will be a pulse when waking up from sleep that will cause the module to start directly.
    // https://github.com/Xinyuan-LilyGO/LilyGO-T-A76XX/issues/85
    // digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL);
    // gpio_hold_en(static_cast<gpio_num_t>(MODEM_RESET_PIN));
    // gpio_deep_sleep_hold_en();
#endif
}

bool BrooderAlarmGateway::TestModemConnection() {
    Serial.println("Checking modem connection...");
    for (int i = 0; i < 10; i++) {
        if (modem.testAT()) {
            Serial.println("Modem is online!");
            return true;
        }
        Serial.print(".");
        delay(500);
    }
    return false;
}

void BrooderAlarmGateway::WakeModemFromSleep(){
    Serial.println("Waking up modem...");
    gpio_hold_dis((gpio_num_t)MODEM_DTR_PIN);

    pinMode(MODEM_DTR_PIN, OUTPUT);
    digitalWrite(MODEM_DTR_PIN, LOW);
    delay(2000);
    modem.sleepEnable(false);

    delay(10000);
}

void BrooderAlarmGateway::EnterEspDeepSleepMode(uint8_t time_to_sleep_sec){
    Serial.println("ESP32 going to deep sleep...");
    esp_sleep_enable_timer_wakeup(time_to_sleep_sec * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
}

/*========================================================================*/
void BrooderAlarmGateway::Init(){
    Serial.begin(115200); // Set console baud rate
    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER) {
        InitModem();
    } else {
        WakeModemFromSleep();
        TestModemConnection();
        CheckSimStatus();
    }

    InitBle();

    UpdateGpsData();

    String msg = "BrooderAlarmGateway System initialization complete. The system is now running and operational.";
    SendSMS(SMS_TARGET, msg);
}

void BrooderAlarmGateway::Run(){
    ScanBle();
    ReadDataFromServer();
    auto msg = ReadSMS();
    delay(60000);
}


#ifndef TINY_GSM_FORK_LIBRARY
#error "No correct definition detected, Please copy all the [lib directories](https://github.com/Xinyuan-LilyGO/LilyGO-T-A76XX/tree/main/lib) to the arduino libraries directory , See README"
#endif
