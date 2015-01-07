#ifndef MONITOR_HPP_INCLUDED
#define MONITOR_HPP_INCLUDED

#include <windows.h>
#include <dbt.h>
#include <setupapi.h>
#include <cstdio>
#include <string>
#include <algorithm>
//#include <devguid.h>

void ListDisplaySettings(int index, DWORD flags = ENUM_CURRENT_SETTINGS);

void EnumerateDisplays(void (*onDisplay)(DISPLAY_DEVICE* dsp_device, DEVMODE* default_mode));

void EnumerateMonitors(bool (*onMonitor)(HMONITOR hMonitor, MONITORINFOEX* info));

std::string MonitorName(unsigned char* edid_data);

class MonitorDisplay
{
private:
    const unsigned int SDC_USE_SUPPLIED_DISPLAY_CONFIG = 0x00000020;
    const unsigned int SDC_VALIDATE = 0x00000040;
    const unsigned int SDC_APPLY = 0x00000080;
    const unsigned int SDC_NO_OPTIMIZATION = 0x00000100;
    const unsigned int SDC_SAVE_TO_DATABASE = 0x00000200;
    const unsigned int SDC_ALLOW_CHANGES = 0x00000400;
    const unsigned int SDC_PATH_PERSIST_IF_REQUIRED = 0x00000800;
    const unsigned int SDC_FORCE_MODE_ENUMERATION = 0x00001000;
    const unsigned int SDC_ALLOW_PATH_ORDER_CHANGES = 0x00002000;

    const unsigned int SDC_TOPOLOGY_INTERNAL = 0x00000001;
    const unsigned int SDC_TOPOLOGY_CLONE = 0x00000002;
    const unsigned int SDC_TOPOLOGY_EXTEND = 0x00000004;
    const unsigned int SDC_TOPOLOGY_EXTERNAL = 0x00000008;
    const unsigned int SDC_TOPOLOGY_SUPPLIED = 0x00000010;

    LONG (*SetDisplayConfig)(UINT32, void*, UINT32, void*, UINT32);

    std::string GetDisplayInfo(DEV_BROADCAST_DEVICEINTERFACE* dev_interface);
    std::string ReadRegistryKey(HKEY Root, std::string SubRoot, std::string KeyToRead);

public:
    MonitorDisplay();

    void Extend();
    void Clone();
    void External();
    void Internal();

    void TurnOn();
    void TurnOff();

    std::string Name(DEV_BROADCAST_DEVICEINTERFACE* dev_interface);
};


#endif // MONITOR_HPP_INCLUDED
