#ifndef _PARAMMGR_H_
#define _PARAMMGR_H_

#include <array>
#include <bitset>
#include <stdint.h>
#include <Preferences.h>

#include "../common.h"
#include "../d2_params_generator/api/paramdescr.h"

//---------------------------------------------------------
#define _PD(reg, owner, pac, ptype, nvs_storage) \
    ParamsMgr.AddParam(TParamData{reg, owner, pac, ptype, nvs_storage})

//---------------------------------------------------------
enum EParamType {PT_NONE = 0, PT_INT32, PT_UINT32, PT_STRING};

struct TParamData {

    uint16_t classID = 0;   // Param class ID
    uint16_t ownerPID = 0;  // Param owner process ID
    bool nvs = false;       // Save param to Non-Volatile Storage (NVS). Set to 4 bits for ESP register alignment.
    EParamType type = PT_NONE;    // casting type
    const void* pAddr = nullptr;  // Param address
    const TParamDescr* descr = nullptr; // Parameter descriptor pointer

    // Default constructor
    TParamData() = default;

    // Custom constructor
    TParamData(const void* addr, uint16_t owner, uint16_t clsID, EParamType ptype, bool storage)
        : classID(clsID), ownerPID(owner), nvs(storage), type(ptype), pAddr(addr), descr(nullptr) {}    
};

//---------------------------------------------------------
class TParamsMgr {
public:
    int16_t AddParam(TParamData pd);

    void SaveToNVS(int16_t pid); // from pid we can get the value of param and save it to nvs 

private: // Methods


private:
    int16_t params_idx = 0; // Current parameter index
    std::array<TParamData, MAX_NUM_OF_PARAMS> params_data; // Array to store parameter data
    std::bitset<MAX_NUM_OF_PARAMS> params_changes; // Tracks changes to parameters

    // void SetPID(TParamData& pd, uint16_t pid);
};


extern TParamsMgr ParamsMgr;
extern Preferences preferences;

//---------------------------------------------------------
#endif //_PARAMMGR_H_