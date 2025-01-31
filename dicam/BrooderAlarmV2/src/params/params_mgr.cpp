#include "params_mgr.h"
#include <type_traits>

#define TO_CHAR(x) std::to_string(x).c_str()

Preferences preferences;

int16_t TParamsMgr::AddParam(TParamData pd) {
    // Ensure the index is within bounds
    if (params_idx >= MAX_NUM_OF_PARAMS) {
        return -1; //Exceeded maximum number of parameters
    }

    // Add parameter data and associate its descriptor
    params_data[params_idx] = pd;
    params_data[params_idx].descr = &paramdescr[pd.classID];

    // Mark the parameter as changed
    params_changes.set(params_idx);

    // increment for the next parameter
    return params_idx++;
}

void TParamsMgr::SaveToNVS(int16_t pid) {
    // Ensure pid is valid
    if (pid >= params_idx) {
        // pid is out of system params
        return;
    }

    TParamData* pd = &params_data[pid];

    // Check if the param can be saved into NVS
    if (!pd->nvs) {
        // Param is not meant to be in ESP NVS
        return;
    }

    preferences.begin(NVS_NAMESPACE, false);

    // Convert PID to a string and ensure proper memory management
    std::string pid_key_str = std::to_string(pid);
    const char* pid_key = pid_key_str.c_str(); // Safe pointer to C-string

    switch(pd->type) {
        case PT_UINT32: {
            uint32_t val = *(const uint32_t*)pd->pAddr;
            preferences.putUInt(pid_key, val);
        } break;

        case PT_INT32: {
            int32_t val = *(const int32_t*)pd->pAddr;
            preferences.putInt(pid_key, val);
        } break;

        case PT_STRING: {
            std::string val = *(const std::string*)pd->pAddr;
            preferences.putString(pid_key, val.c_str());
        } break;

        default:
            // Log error or handle unknown param type
            break;
    }

    // Close preferences to free resources
    preferences.end();
}



    // preferences.begin("system-params", false); // Set to "true" for read-only mode.
    // // Save a setting
    // preferences.putInt("my-int", 42);
    // preferences.putFloat("my-float", 3.14);
    // preferences.pustd::string("my-string", "Hello, ESP!");

    // // Retrieve settings
    // int myInt = preferences.getInt("my-int", 0);         // Default is 0 if not set
    // float myFloat = preferences.getFloat("my-float", 0); // Default is 0
    // String myString = preferences.gestd::string("my-string", "Default");

    // Serial.println("Saved Settings:");
    // Serial.println(myInt);
    // Serial.println(myFloat);
    // Serial.println(myString);

    // // Close preferences to free resources
    // preferences.end();


// void TParamsMgr::SetPID(TParamData& pd, uint16_t pid){
//     switch(pd.type){
//         case PT_INT32:
//             {TInt32* ptr = (TInt32*)(pd.pAddr);
//             ptr->pid = params_idx;}
//             break;
//         case PT_UINT32:
//             {uint32_t* ptr = (uint32_t*)(pd.pAddr);
//             ptr->pid = params_idx;}
//             break;
//         case PT_STRING:
//             {std::string* ptr = (std::string*)(pd.pAddr);
//             ptr->pid = params_idx;}
//             break;
//     }
// }



TParamsMgr ParamsMgr;