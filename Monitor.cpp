#include "Monitor.hpp"

void ListDisplaySettings(int index, DWORD flags)
{
    DISPLAY_DEVICE dd;
    dd.cb = sizeof(DISPLAY_DEVICE);
    if(EnumDisplayDevices(NULL, index, &dd, 0))
    {
        DISPLAY_DEVICE monitor;
        monitor.cb = sizeof(DISPLAY_DEVICE);
        EnumDisplayDevices(dd.DeviceName, index, &monitor, 0);

        DEVMODE dm;
        dm.dmSize = sizeof(DEVMODE);
        EnumDisplaySettings(dd.DeviceName, flags, &dm);
        printf("Device name: %s\n", dd.DeviceName);
        printf("Monitor name: %s\n", monitor.DeviceID);
        printf("Refresh rate, in hertz: %lu\n", dm.dmDisplayFrequency);
        printf("Color depth: %lu\n", dm.dmBitsPerPel);
        printf("Location: %lu, %lu\n", dm.dmPosition.x, dm.dmPosition.y);
        printf("Screen resolution, in pixels: %lu x %lu\n",
               dm.dmPelsWidth, dm.dmPelsHeight);
        printf("Is Primary: %s\n", dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE ? "True" : "False");
        printf("Is Active: %s\n", dd.StateFlags & DISPLAY_DEVICE_ACTIVE ? "True" : "False");
        printf("Is Attached: %s\n", dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP ? "True" : "False");
        printf("Is Mirroring: %s\n\n\n", dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER ? "True" : "False");
        ++index;
    }
}

void EnumerateDisplays(void(*onDisplay)(DISPLAY_DEVICE* dsp_device, DEVMODE* default_mode))
{
    DWORD dsp_num = 0;
    DISPLAY_DEVICE dsp_device = {0};
    dsp_device.cb = sizeof(DISPLAY_DEVICE);

    while (EnumDisplayDevices(NULL, dsp_num, &dsp_device, 0))
    {
        DEVMODE default_mode = {0};
        default_mode.dmSize = sizeof(DEVMODE);
        if (!EnumDisplaySettings(dsp_device.DeviceName, ENUM_REGISTRY_SETTINGS, &default_mode))
        {
            std::printf("Failed to Enumerate Display Settings for: %s -- %lu\n", dsp_device.DeviceName, GetLastError());
        }

        onDisplay(&dsp_device, &default_mode);

        memset(&dsp_device, 0, sizeof(dsp_device));
        dsp_device.cb = sizeof(dsp_device);
        ++dsp_num;
    }
}

BOOL CALLBACK MonitorProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    MONITORINFOEX info;
    memset(&info, 0, sizeof(info));
    info.cbSize = sizeof(info);

    GetMonitorInfo(hMonitor, &info);
    bool (*onMonitor)(HMONITOR, MONITORINFOEX*) = reinterpret_cast<bool (*)(HMONITOR hMonitor, MONITORINFOEX* info)>(dwData);
    return onMonitor(hMonitor, &info);
}

void EnumerateMonitors(bool (*onMonitor)(HMONITOR hMonitor, MONITORINFOEX* info))
{
    EnumDisplayMonitors(NULL, NULL, &MonitorProc, reinterpret_cast<LPARAM>(onMonitor));
}

std::string MonitorName(unsigned char* edid_data)
{
    static unsigned char name[] = {0x00, 0x00, 0x00, 0xFC, 0x00};
    static unsigned char header[] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};

    auto find_info = [&](int s, int e, unsigned char* data, int len) -> void*
    {
        auto it = std::search(&edid_data[s], &edid_data[e], &data[0], &data[len]);
        if (it != &edid_data[e])
            return it;
        return nullptr;
    };

    if (!find_info(0, sizeof(header), &header[0], sizeof(header)))
        return std::string();

    void* o = find_info(54, 125, &name[0], sizeof(name));
    if (o)
    {
        std::string str = std::string((char*)o + sizeof(name), 12);
        str.erase(std::find_if(str.rbegin(), str.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), str.end());
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return str;
    }

    return std::string();
}

MonitorDisplay::MonitorDisplay() : SetDisplayConfig(NULL)
{
    HMODULE mod = GetModuleHandle("User32.dll");
    SetDisplayConfig = reinterpret_cast<decltype(SetDisplayConfig)>(GetProcAddress(mod, "SetDisplayConfig"));
}

std::string MonitorDisplay::GetDisplayInfo(DEV_BROADCAST_DEVICEINTERFACE* dev_interface)
{
    std::string info(dev_interface->dbcc_name);
    std::string::size_type first = info.find("\\\\?\\");
    std::string::size_type last = info.find_last_of('#');
    if (first != std::string::npos && last != std::string::npos)
    {
        info.erase(last);
        info.erase(first, 4);
        std::replace(info.begin(), info.end(), '#', '\\');
        return info;
    }
    return std::string();
}

std::string MonitorDisplay::ReadRegistryKey(HKEY Root, std::string SubRoot, std::string KeyToRead)
{
    HKEY hKey;
    if (RegOpenKeyEx(Root, SubRoot.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwSize = 0;
        DWORD KeyType = REG_BINARY;
        RegQueryValueEx(hKey, KeyToRead.c_str(), NULL, &KeyType, NULL, &dwSize);

        std::string Result = std::string(dwSize, '\0');

        if (RegQueryValueEx(hKey, KeyToRead.c_str(), NULL, &KeyType, reinterpret_cast<std::uint8_t*>(&Result[0]), &dwSize) == ERROR_SUCCESS)
        {
            Result.resize(dwSize);
            RegCloseKey(hKey);
            return Result;
        }
        RegCloseKey(hKey);
    }
    return std::string();
}

void MonitorDisplay::Extend()
{
    SetDisplayConfig(0, NULL, 0, NULL, SDC_APPLY | SDC_TOPOLOGY_EXTEND);
}

void MonitorDisplay::Clone()
{
    SetDisplayConfig(0, NULL, 0, NULL, SDC_APPLY | SDC_TOPOLOGY_CLONE);
}

void MonitorDisplay::Internal()
{
    SetDisplayConfig(0, NULL, 0, NULL, SDC_APPLY | SDC_TOPOLOGY_INTERNAL);
}

void MonitorDisplay::External()
{
    SetDisplayConfig(0, NULL, 0, NULL, SDC_APPLY | SDC_TOPOLOGY_EXTERNAL);
}

void MonitorDisplay::TurnOn()
{
    SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, -1);
}

void MonitorDisplay::TurnOff()
{
    SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
}

std::string MonitorDisplay::Name(DEV_BROADCAST_DEVICEINTERFACE* dev_interface)
{
    std::string info = GetDisplayInfo(dev_interface);
    info = ReadRegistryKey(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Enum\\" + info + "\\Device Parameters", "EDID");
    return MonitorName(reinterpret_cast<unsigned char*>(&info[0]));
}
