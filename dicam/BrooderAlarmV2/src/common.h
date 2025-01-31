#ifndef _COMMON_H_
#define _COMMON_H_


// System parameters
#define NONE_PID 0  // None param pid
#define MAX_NUM_OF_PARAMS 200  // Maximum number of parameters that can be stored
#define NVS_NAMESPACE "system-params"  // Namespace used for Non-Volatile Storage (NVS) to store system parameters
#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor to convert microseconds to seconds
#define WDT_TIMEOUT 70  // Watchdog timer (WDT) timeout period in seconds

// Server params
#define SERVER "86.17.107.206"  // Server IP address for the application
#define PORT 8181  // Server port number

// BEL and ruuvi-tag params
#define BLE_SCAN_TIME_MS 10000  // Duration of Bluetooth Low Energy (BLE) scan in ms
#define RUUVI_COMPANY_ID_1 0x99
#define RUUVI_COMPANY_ID_2 0x04
#define RUUVI_MAX_NUM_OF_TAGS 20  // Maximum number of Ruuvi tags that can be scanned
#define RUUVI_MAC_LENGTH 7  // Length of the MAC address of Ruuvi tags, 6+1 for null terminator
#define RUUVI_DATA_LENGTH 26 // Length of the Ruuvi tag data
#define RUUVI_DATA_FORMAT_5 5  // Ruuvi tag data format

// Modem params
#define MODEM_TIMEOUT_MS 1000 // Modem timeout in ms
#define SMS_MAX_RETRY 10
#define TINY_GSM_RX_BUFFER 2048  // Set the RX buffer size for TinyGSM library to 2048 bytes (2 KB)

// ESP hotspot config 
#define AP_SSID "ESP32_Hotspot"
#define AP_PASSWORD "12345678" // Minimum 8 characters

//==============================================================================
// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

// Define DEBUG to enable debug logging
#define DEBUG 1 // Set to 0 to disable debugging

// Debug macros for different Serial methods
#if DEBUG
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(x)    // Do nothing
    #define DEBUG_PRINTLN(x) // Do nothing
    #define DEBUG_PRINTF(...) // Do nothing
#endif


#endif  // _COMMON_H_