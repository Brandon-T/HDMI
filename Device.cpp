#include "Device.hpp"

Device::Device(HWND window, GUID dev_guid) : hNotify(NULL)
{
    if (window)
    {
        DEV_BROADCAST_DEVICEINTERFACE filter;
        memset(&filter, 0, sizeof(filter));
        filter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
        filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        filter.dbcc_classguid = dev_guid;
        hNotify = RegisterDeviceNotification(window, &filter, DEVICE_NOTIFY_WINDOW_HANDLE); //DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES
    }
}
