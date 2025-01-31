
#include <Arduino.h>
#include "ruuvitag.h"

bool TRuuviTag::ParseData(const NimBLEAdvertisedDevice* advertisedDevice) {
    std::string data = advertisedDevice->getManufacturerData();
    if (data.empty()) {
        DEBUG_PRINTLN("[RuuviTag] ERR: Empty manufacturer data");
        return false;
    }
    if (data[0] != RUUVI_COMPANY_ID_1 && data[1] != RUUVI_COMPANY_ID_2) {
        DEBUG_PRINTLN("[RuuviTag] ERR: Not a RuuviTag");
        return false;
    }
    if (data[2] != RUUVI_DATA_FORMAT_5) {
        DEBUG_PRINTF("[RuuviTag] Invalid RuuviTag data format, format: %d\n", data[2]);
        return false;
    }
    if (data.length() < RUUVI_DATA_LENGTH) {
        DEBUG_PRINTLN("[RuuviTag] Invalid RuuviTag data length");
        return false;
    }

    char macBuffer[20];
    snprintf(macBuffer, sizeof(macBuffer), "%02X:%02X:%02X:%02X:%02X:%02X", 
             data[20], data[21], data[22], data[23], data[24], data[25]);
    std::string mac(macBuffer);
    DEBUG_PRINTF("[RuuviTag] MAC: %s\n", mac.c_str());

    for (uint8_t i = 0; i < tags_count; i++) {
        if (mac == tags_data[i].mac) {
            DEBUG_PRINTF("[RuuviTag] %s found in the table, index: %d\n", mac.c_str(), i);
            UpdateTagData(i, data, advertisedDevice->getRSSI());
            return true;
        }
    }
    
    if (tags_count < RUUVI_MAX_NUM_OF_TAGS) {
        DEBUG_PRINTF("[RuuviTag] %s not found in the table. Add to table -> index: %d\n", mac.c_str(), tags_count);
        tags_data[tags_count].mac = mac;
        UpdateTagData(tags_count, data, advertisedDevice->getRSSI());
        tags_count++;
        return true;
    }
    return false;
}

void TRuuviTag::UpdateTagData(uint8_t index, const std::string& data, int8_t rssi) {
    if (index >= tags_count) {
        return;
    }

    tags_data[index].temp = ((int16_t)(data[1+2] << 8 | data[2+2])) * 0.005;
    tags_data[index].humidity = ((uint16_t)(data[3+2] << 8 | data[4+2])) * 0.0025;
    tags_data[index].battery = ((data[13+2] << 8 | data[14+2]) >> 5) + 1600;
    tags_data[index].measurementSequence = (data[16+2] << 8) | data[17+2];
    tags_data[index].rssi = rssi;

    DEBUG_PRINTF("[RuuviTag] Temperature: %.2f\n", tags_data[index].temp);
    DEBUG_PRINTF("[RuuviTag] Humidity: %.2f\n", tags_data[index].humidity);
    DEBUG_PRINTF("[RuuviTag] Battery: %d\n", tags_data[index].battery);
    DEBUG_PRINTF("[RuuviTag] Measurement Sequence: %d\n", tags_data[index].measurementSequence);
    DEBUG_PRINTF("[RuuviTag] RSSI: %d\n", tags_data[index].rssi);
    DEBUG_PRINTLN("=====================================");
}

bool TRuuviTag::GetDataFromMac(const std::string& mac, TRuuviTagData& data) {
    std::string cleanMac = mac;
    cleanMac.erase(std::remove(cleanMac.begin(), cleanMac.end(), ':'), cleanMac.end());
    std::transform(cleanMac.begin(), cleanMac.end(), cleanMac.begin(), ::toupper);
    for (uint8_t i = 0; i < tags_count; i++) {
        std::string storedMac = tags_data[i].mac;
        storedMac.erase(std::remove(storedMac.begin(), storedMac.end(), ':'), storedMac.end());
        std::transform(storedMac.begin(), storedMac.end(), storedMac.begin(), ::toupper);
        
        if (cleanMac == storedMac) {
            data = tags_data[i];
            return true;
        }
    }
    return false;
}

bool TRuuviTag::GetMacFromIndex(uint8_t index, std::string& mac) const {
    if (index >= tags_count) {
        return false;
    }
    mac = tags_data[index].mac;
    return true;
}

TRuuviTag ruuvitag;
