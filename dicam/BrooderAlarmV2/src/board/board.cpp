
#include "board.h"

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

extern const char* captivePortalPage;

// WebServer and DNS Server
WebServer server(80);
DNSServer dnsServer;

// Define static member variables
String TBoardMgr::apn = "";
String TBoardMgr::apn_username = "";
String TBoardMgr::apn_password = "";
bool TBoardMgr::apn_set_flag = false;


//================================================
// Hotspot handlers

void TBoardMgr::HotspotHandleRoot() {
  server.send(200, "text/html", captivePortalPage);
}

void TBoardMgr::HotspotHandelSave() {
  if (server.hasArg("apn") && server.hasArg("username") && server.hasArg("password")) {
    apn = server.arg("apn");
    apn_username = server.arg("username");
    apn_password = server.arg("password");

    server.send(200, "text/html", "<h2>Configuration Saved!</h2><p>APN: " + apn + "<br>Username: " + apn_username + "<br>Password: [hidden]</p>");
    apn_set_flag = true;

    DEBUG_PRINTLN("Configuration Saved:");
    DEBUG_PRINTLN("APN: " + apn);
    DEBUG_PRINTLN("Username: " + apn_username);
    DEBUG_PRINTLN("Password: " + apn_password);
  } else {
    server.send(400, "text/html", "<h2>Error: Missing parameters!</h2>");
  }
}

void TBoardMgr::EnableHotspot(){
    // Start WiFi in Access Point mode
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    DEBUG_PRINTLN("Access Point started");
    DEBUG_PRINT("IP Address: ");
    DEBUG_PRINTLN(WiFi.softAPIP());

    // Start DNS Server for captive portal
    dnsServer.start(53, "*", WiFi.softAPIP());

    // Setup WebServer routes
    server.on("/", TBoardMgr::HotspotHandleRoot);
    server.on("/save", HTTP_POST, TBoardMgr::HotspotHandelSave);

    // Start WebServer
    server.begin();
    DEBUG_PRINTLN("WebServer started");
}

void TBoardMgr::DisableHotspot(){
    // Stop DNS Server
    dnsServer.stop();
    DEBUG_PRINTLN("DNS Server stopped");

    // Stop WebServer
    server.stop();
    DEBUG_PRINTLN("WebServer stopped");

    // Disable WiFi Access Point
    WiFi.softAPdisconnect(true);
    DEBUG_PRINTLN("Access Point disabled");
}

void TBoardMgr::RunHotspot(){
    dnsServer.processNextRequest();
    server.handleClient();
}

//================================================
// Modem functions

#define TOGGLE_BOARD_PWRKEY()   {   \
        digitalWrite(BOARD_PWRKEY_PIN, LOW); \
        delay(100);                          \
        digitalWrite(BOARD_PWRKEY_PIN, HIGH);\
        delay(1000);                         \
        digitalWrite(BOARD_PWRKEY_PIN, LOW); \
    }

void TBoardMgr::EnableModemPower(){
    DEBUG_PRINTLN("Powering on the modem...");
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
    DEBUG_PRINTLN("Start modem...");

    int retry = 0;
    while (!modem.testAT(MODEM_TIMEOUT_MS)) {
        DEBUG_PRINTLN(".");
        if (retry++ > 10) {
            TOGGLE_BOARD_PWRKEY();
            retry = 0;
        }
    }
    DEBUG_PRINTLN();
}

void TBoardMgr::BeginModemSerial(){
    DEBUG_PRINTLN("Initializing modem serial communication...");
    SerialAT.begin(MODEM_BAUDRATE, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);
}

void TBoardMgr::CheckSimStatus() {
    DEBUG_PRINTLN("Checking SIM card status...");
    sim_status = SIM_ERROR;

    if (!CheckSimCardStatus()) {
        return;
    }

    if (!WaitForNetworkConnection()) {
        return;
    }

    if (!ConnectToGprs()) {
        return;
    }

    DEBUG_PRINTLN("System is fully connected.");
}

bool TBoardMgr::CheckSimCardStatus() {
    for (uint8_t attempt = 0; attempt < SMS_MAX_RETRY; attempt++) {
        sim_status = modem.getSimStatus();

        if (sim_status == SIM_READY) {
            DEBUG_PRINTLN("SIM card online");
            return true;
        }

        if (sim_status == SIM_LOCKED) {
            DEBUG_PRINTLN("The SIM card is locked. Please unlock the SIM card first.");
            return false;
        }

        DEBUG_PRINTF("Unable to check SIM status, retry #%d\n", attempt);
        delay(1000);
    }

    DEBUG_PRINTLN("SIM card status check failed after maximum retries.");
    return false;
}

bool TBoardMgr::WaitForNetworkConnection() {
    DEBUG_PRINT("Waiting for network...");

    for (uint8_t attempt = 0; attempt < SMS_MAX_RETRY; attempt++) {
        if (modem.waitForNetwork()) {
            DEBUG_PRINTLN(" success");

            if (modem.isNetworkConnected()) {
                DEBUG_PRINTLN("Network connected");
            }

            return true;
        }

        DEBUG_PRINTF("Network wait failed, retry #%d\n", attempt);
        delay(1000);
    }

    DEBUG_PRINTLN("Network wait failed after maximum retries.");
    delay(1000);
    return false;
}

bool TBoardMgr::ConnectToGprs() {
    DEBUG_PRINT(F("Connecting to "));
    DEBUG_PRINT(apn);

    for (uint8_t attempt = 0; attempt < SMS_MAX_RETRY; attempt++) {
        if (modem.gprsConnect(apn.c_str(), apn_username.c_str(), apn_password.c_str())) {
            DEBUG_PRINTLN(" success");

            if (modem.isGprsConnected()) {
                DEBUG_PRINTLN("GPRS connected");
            }

            return true;
        }

        DEBUG_PRINTF("GPRS connection failed, retry #%d\n", attempt);
        delay(1000);
    }

    DEBUG_PRINTLN("GPRS connection failed after maximum retries.");
    delay(1000);
    return false;
}


void TBoardMgr::SetModemNetworkMode(){
    DEBUG_PRINTLN("Setting the modem's network mode...");
    //SIM7672G Can't set network mode
#ifndef TINY_GSM_MODEM_SIM7672
    if (!modem.setNetworkMode(MODEM_NETWORK_AUTO)) {
        DEBUG_PRINTLN("Set network mode failed!");
    }
    network_mode = modem.getNetworkModes();
    DEBUG_PRINT("Current network mode : ");
    DEBUG_PRINTLN(network_mode);
#endif
}

void TBoardMgr::GetModemName(){
    DEBUG_PRINTLN("Retrieving modem name...");
    modem_Name = "UNKOWN";
    while (1) {
        modem_Name = modem.getModemName();
        if (modem_Name == "UNKOWN") {
            DEBUG_PRINTLN("Unable to obtain module information normally, try again");
            delay(1000);
        } else if (modem_Name.startsWith("SIM7670")) {
            while (1) {
                DEBUG_PRINTLN("SIM7670 does not support SMS Function");
                delay(1000);
            }
        } else {
            DEBUG_PRINT("Modem Name:");
            DEBUG_PRINTLN(modem_Name);
            break;
        }
    }

    // Get model info
    modem.sendAT("+SIMCOMATI");
    modem.waitResponse();
}

void TBoardMgr::SetSMSToTextMode(){
    // Wait PB DONE
    DEBUG_PRINTLN("Configuring SMS to text mode...");
    if (!modem.waitResponse(10000UL, "SMS DONE")) {
        DEBUG_PRINTLN("Can't wait from sms register ....");
        return;
    }
    // Set SMS system into text mode
    modem.sendAT("+CMGF=1");
    modem.waitResponse(10000);
}

void TBoardMgr::SetNetworkAPN(){
    DEBUG_PRINTF("Set network apn : %s\n", apn.c_str());
    modem.sendAT(GF("+CGDCONT=1,\"IP\",\""), apn.c_str(), "\"");
    if (modem.waitResponse() != 1) {
        DEBUG_PRINTLN("Set network apn error !");
    }
}

void TBoardMgr::EnableModemGps(){
    DEBUG_PRINTLN("Enabling GPS/GNSS/GLONASS");
    while (!modem.enableGPS(MODEM_GPS_ENABLE_GPIO, MODEM_GPS_ENABLE_LEVEL)) {
        DEBUG_PRINT(".");
    }
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("GPS Enabled");

    // Set GPS Baud
    modem.setGPSBaud(MODEM_BAUDRATE);
}

void TBoardMgr::CheckNetworkStatus(){
    // Check network registration status and network signal status    
    DEBUG_PRINTLN("Wait for the modem to register with the network.");
    RegStatus status = REG_NO_RESULT;
    while (status == REG_NO_RESULT || status == REG_SEARCHING || status == REG_UNREGISTERED) {
        status = modem.getRegistrationStatus();
        switch (status) {
        case REG_UNREGISTERED:
        case REG_SEARCHING:
            signal_quality = modem.getSignalQuality();
            DEBUG_PRINTF("[%lu] Signal Quality:%d\n", millis() / 1000, signal_quality);
            delay(1000);
            break;
        case REG_DENIED:
            DEBUG_PRINTLN("Network registration was rejected, please check if the APN is correct");
            return ;
        case REG_OK_HOME:
            DEBUG_PRINTLN("Online registration successful");
            break;
        case REG_OK_ROAMING:
            DEBUG_PRINTLN("Network registration successful, currently in roaming mode");
            break;
        default:
            DEBUG_PRINTF("Registration Status:%d\n", status);
            delay(1000);
            break;
        }
    }
    DEBUG_PRINTLN();
    signal_quality = modem.getSignalQuality();

    if (!modem.setNetworkActive()) {
        DEBUG_PRINTLN("Enable network failed!");
    }

    ip_address = modem.getLocalIP();
    DEBUG_PRINT("Network IP:"); DEBUG_PRINTLN(ip_address);
}

bool TBoardMgr::SendSMS(const String& number, const String& msg_str){
    DEBUG_PRINTF("Sending SMS message to %s \n", number);
    bool res = modem.sendSMS(number, msg_str);
    DEBUG_PRINTLN(res ? "OK" : "fail");
    return res;
}

void TBoardMgr::ReadSMS(String& data){
    int8_t res;

    modem.sendAT("+CMGL=\"REC UNREAD\""); // Fetch only unread messages
    res = modem.waitResponse(1000, data); // Wait for the response
    if (res == 1) {
        DEBUG_PRINT("Unread Messages:");
        data.replace("\r\nOK\r\n", "");
        data.replace("\rOK\r", "");
        data.trim();
        DEBUG_PRINTLN(data); // Print the data of unread messages
    } else {
        DEBUG_PRINT("Failed to fetch unread messages");
    }
}

bool TBoardMgr::TestModemConnection() {
    DEBUG_PRINTLN("Checking modem connection...");
    for (int i = 0; i < 10; i++) {
        if (modem.testAT()) {
            DEBUG_PRINTLN("Modem is online!");
            return true;
        }
        DEBUG_PRINT(".");
        delay(500);
    }
    return false;
}

void TBoardMgr::SwitchModemOff(){
    DEBUG_PRINTLN("Enter modem power off!");
    if (modem.poweroff()) { 
        DEBUG_PRINTLN("Modem enter power off modem!"); 
    } else { 
        DEBUG_PRINTLN("modem power off failed!"); 
    }
    delay(500);
#ifdef BOARD_POWERON_PIN 
    // Turn on DC boost to power off the modem 
    digitalWrite(BOARD_POWERON_PIN, LOW);
#endif
#ifdef MODEM_RESET_PIN 
    // Keep it low during the sleep period. If the module uses GPIO5 as reset, 
    // there will be a pulse when waking up from sleep that will cause the module to start directly. 
    // https://github.com/Xinyuan-LilyGO/LilyGO-T-A76XX/issues/85 
    digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL); 
    gpio_hold_en((gpio_num_t)MODEM_RESET_PIN); 
    gpio_deep_sleep_hold_en();
#endif
}

void TBoardMgr::EnterEspDeepSleepMode(uint8_t time_to_sleep_sec){
    DEBUG_PRINTLN("ESP32 going to deep sleep...");
    esp_sleep_enable_timer_wakeup(time_to_sleep_sec * uS_TO_S_FACTOR);
    delay(500);
    esp_deep_sleep_start();
    DEBUG_PRINTLN("This will never be printed");
}

void TBoardMgr::InitModem(){
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

//================================================
// BLE functions

void TBoardMgr::InitBle(){
    NimBLEDevice::init("Beacon-scanner");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(false);
}

void TBoardMgr::RunBle(){
    //before starting new scan, we first parse the data from the previous scan,
    //then we clear the resutls and start a new scan
    this->ParseBleData();
    this->ScanBle(); 
}

void TBoardMgr::ScanBle(){
    if(!pBLEScan->isScanning()) {
        DEBUG_PRINTLN("BLE Scanning is running...");
        pBLEScan->start(BLE_SCAN_TIME_MS, false);        
    }   
}

void TBoardMgr::ParseBleData(){
    //Only parse the data if the scanning is not active and if there are devices found
    if(pBLEScan->isScanning()){
        return;
    }

    NimBLEScanResults foundDevices = pBLEScan->getResults();
    DEBUG_PRINT("Devices found: ");
    DEBUG_PRINTLN(foundDevices.getCount());
    // find the ruuvi tag data and copy it to the ruuviTagData array
    for (auto i = 0; i < foundDevices.getCount(); i++){
        const NimBLEAdvertisedDevice *advertisedDevice = foundDevices.getDevice(i);
        std::string manufacturerData = advertisedDevice->getManufacturerData();
        // if we found the ruuvi tag data, parse it
        if (!manufacturerData.empty() && manufacturerData[0] == RUUVI_COMPANY_ID_1 &&  manufacturerData[1] == RUUVI_COMPANY_ID_2) { // RuuviTag company ID
            ruuvitag.ParseData(advertisedDevice);
        }
    }
    pBLEScan->clearResults(); // delete results scan buffer to release memory

}

//================================================