#ifndef _RUUVITAG_H_
#define _RUUVITAG_H_

#include <stdint.h>
#include <string>
#include <NimBLEAdvertisedDevice.h>
#include "../common.h"

struct TRuuviTagData {
    float temp{};
    float humidity{};
    uint16_t battery{};
    uint16_t measurementSequence{};
    int8_t rssi{};
    std::string mac;

    TRuuviTagData() 
        : temp(0.0f), humidity(0.0f), battery(0), measurementSequence(0), rssi(0), mac("") {}

    void ResetSenData() {
        temp = humidity = 0.0f;
        measurementSequence = battery = 0;
        rssi = 0;
    }
};

class TRuuviTag {
public:
    TRuuviTag() : tags_count(0) {};

    bool ParseData(const NimBLEAdvertisedDevice* advertisedDevice);
    bool GetMacFromIndex(uint8_t index, std::string& mac) const;
    bool GetDataFromMac(const std::string& mac, TRuuviTagData& data);
    
    uint8_t GetTagsCount() const { return tags_count; }
    

private:
    void UpdateTagData(uint8_t index, const std::string& data, int8_t rssi);

private:
    uint8_t tags_count;
    TRuuviTagData tags_data[RUUVI_MAX_NUM_OF_TAGS]{};
};

extern TRuuviTag ruuvitag;

#endif  // _RUUVITAG_H_
