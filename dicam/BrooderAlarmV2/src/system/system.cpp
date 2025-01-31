#include "system.hpp"

std::string TSystem::GetChipIdAsString(){
    uint64_t chipId = ESP.getEfuseMac(); // Get the 48-bit MAC address as a uint64_t
    uint32_t high = (uint32_t)(chipId >> 32); // Extract the high and low 32-bit parts
    uint32_t low = (uint32_t)chipId;
    char macStr[18]; // 12 characters for the MAC + 5 for formatting + 1 for null terminator
    snprintf(macStr, sizeof(macStr), "%04X%08X", high, low);
    return std::string(macStr); // Convert the formatted string to a std::string
}

void TSystem::InitParamsValue(){
    sp.hw_ver = 0;
    sp.sw_ver = 1;
    sp.build_time = BUILD_EPOCH;
    sp.dev_id = GetChipIdAsString();
}

void TSystem::BuildParams(){
    // Parameter Initialization Table
    // Format:
    //   param_pid       Pointer to param      Parent PID  ClassID/PAC         Data Type    Stored in NVS?
    auto none_pid       = _PD(&sp.none,         NONE_PID,  PAC_None,            PT_UINT32,  false);
    auto sys_pid        = _PD(&sp.sys,          none_pid,  PAC_System,          PT_UINT32,  false);
    auto sys_type_pid   = _PD(&sp.sys_type,     sys_pid,   PAC_SystemType,      PT_UINT32,  false);
    auto sys_ctrl_pid   = _PD(&sp.sys_ctrl,     sys_pid,   PAC_SystemControl,   PT_UINT32,  false);
    auto hw_ver_pid     = _PD(&sp.hw_ver,       sys_pid,   PAC_HardwareVersion, PT_UINT32,  false);
    auto sw_ver_pid     = _PD(&sp.sw_ver,       sys_pid,   PAC_SoftwareVersion, PT_UINT32,  false);
    auto build_time_pid = _PD(&sp.build_time,   sys_pid,   PAC_BuildDateTime,   PT_UINT32,  false);
    auto sig_strength   = _PD(&sp.sig_strength, sys_pid,   PAC_SignalStrength,  PT_INT32,   false);
    auto reset_cnt_pid  = _PD(&sp.reset_cnt,    sys_pid,   PAC_ResetCount,      PT_UINT32,  true); 
    auto uptime_pid     = _PD(&sp.uptime,       sys_pid,   PAC_Uptime,          PT_UINT32,  false);
    auto dev_id_pid     = _PD(&sp.dev_id,       sys_pid,   PAC_DeviceID,        PT_STRING,  false);
    auto mem_stat_pid   = _PD(&sp.mem_stat,     sys_pid,   PAC_MemoryStatus,    PT_UINT32,  false);
    auto param_cnt_pid  = _PD(&sp.param_cnt,    sys_pid,   PAC_TotalParams,     PT_UINT32,  false);
    auto local_time_pid = _PD(&sp.local_time,   sys_pid,   PAC_LocalTime,       PT_UINT32,  false);
}
