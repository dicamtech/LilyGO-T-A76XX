#ifndef _SYSTEM_HPP_
#define _SYSTEM_HPP_

#include <Arduino.h>
#include <stdint.h>
#include "../params/params_mgr.h"
#include "../board/board.h"
#include "../common.h"

struct TSystemParams 
{
    uint32_t none;          // No specific value or unused parameter
    uint32_t sys;           // System identifier or flag
    uint32_t sys_type;      // Type of the system
    uint32_t sys_ctrl;      // System control settings or flags
    uint32_t hw_ver;        // Hardware version (short for hardware_version)
    uint32_t sw_ver;        // Software version (short for software_version)
    uint32_t build_time;    // Build date and time (short for build_data_time)
    int32_t sig_strength;   // Signal strength (short for signal_strength)
    uint32_t reset_cnt;     // Number of resets (short for reset_count)
    uint32_t uptime;        // System uptime in seconds or milliseconds
    std::string dev_id;     // Device ID (short for deviceID)
    uint32_t mem_stat;      // Memory status (short for memory_status)
    uint32_t param_cnt;     // Total number of parameters (short for total_params)
    uint32_t local_time;    // Current local time in the system
};


class TSystem
{
public: //methods
    void Init();
    void Run();

private: //methods
    void InitParamsValue();
    void BuildParams();

    void IncrementUptime();
    void SetBuildTimeData();
    void SetSoftwareVersion();
    void SetHardwareVersion();
    
    void UpdateSignalStringth();
    std::string GetChipIdAsString();

private: //variables
    TSystemParams sp;   // System params
    TBoardMgr board;

};



#endif //_SYSTEM_HPP_