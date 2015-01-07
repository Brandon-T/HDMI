#ifndef DEVICE_HPP_INCLUDED
#define DEVICE_HPP_INCLUDED

#include <windows.h>
#include <dbt.h>
#include <algorithm>

class Device
{
private:
    HDEVNOTIFY hNotify;

public:
    Device() : hNotify(NULL) {}
    Device(HWND window, GUID dev_guid);
    Device(Device&& dev) : hNotify(NULL) {std::swap(hNotify, dev.hNotify);}
    ~Device() {UnregisterDeviceNotification(hNotify);}

    Device(const Device& dev) = delete;
    Device& operator = (const Device& dev) = delete;
    Device& operator = (Device&& dev) {std::swap(hNotify, dev.hNotify);return *this;}
};

#endif // DEVICE_HPP_INCLUDED
