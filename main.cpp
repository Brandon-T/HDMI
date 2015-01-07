#include <iostream>
#include <vector>

#include "GUIDs.hpp"
#include "Device.hpp"
#include "Monitor.hpp"


std::string ErrorMessage(std::uint32_t Error)
{
    LPTSTR lpMsgBuf = NULL;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, NULL);
    return lpMsgBuf;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static std::vector<Device> devs;

    switch(msg)
    {
        case WM_CREATE:
        {
            devs.emplace_back(Device(hwnd, devices[3]));
        }
        break;

        case WM_DEVICECHANGE:
        {
            DEV_BROADCAST_DEVICEINTERFACE* info = (DEV_BROADCAST_DEVICEINTERFACE*) lParam;

            switch(wParam)
            {
                case DBT_DEVICEARRIVAL:
                    MonitorDisplay().Clone();
                    std::cout<<MonitorDisplay().Name(info)<<" Connected.\n";
                    std::cout<<"Set Cloned Display.\n\n";
                    break;

                case DBT_DEVICEREMOVECOMPLETE:
                    MonitorDisplay().Internal();
                    std::cout<<MonitorDisplay().Name(info)<<" Disconnected.\n";
                    std::cout<<"Set Internal Display.\n\n";
                    break;

                default:
                    std::cout<<"wParam: "<<(void*)wParam<<"\n";
                    break;
            }
        }
        break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int Create()
{
    WNDCLASSEX wx = {0};
    wx.cbSize = sizeof(WNDCLASSEX);
    wx.lpfnWndProc = WndProc;
    wx.hInstance = GetModuleHandle(NULL);
    wx.lpszClassName = "HDMI_MONITOR";
    if (RegisterClassEx(&wx))
    {
        MSG msg = {0};
        CreateWindowEx(0, "HDMI_MONITOR", "HDMI_MONITOR", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
        while(GetMessage(&msg, NULL, 0, 0) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return msg.wParam;
    }
    return 0;
}


int main()
{
    MonitorDisplay().TurnOff();
    Sleep(1000);
    MonitorDisplay().TurnOn();
    //return Create();
}
