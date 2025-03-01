#include <iostream>
#include <thread>
#include <cstdlib>
#include <map>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

#include "windows/windows.h"

#ifdef _WIN32
const int os = 0;// "Windows";
#include <Windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    
    case WM_CLOSE:
        /*std::cout << "NIE ZAMKNIESZ MNIE CHUJU\n";
        return 0;*/
        PostQuitMessage(0);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

#elif __APPLE__
const int os = 1;//"Mac OS";
#elif __linux__
const int os = 2;// "Linux";
#include <sys/socket.h>
#else
const int os = 3;//"Inny";
#endif

#define IP_ADDR "127.0.0.1"
#define PORT 8888

HWND hwnd;

//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
int main()
{
    // ALT TAB, CTRL SHIFT ESC
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"Window";

    RegisterClass(&wc);


    MONITORINFO target;
    target.cbSize = sizeof(MONITORINFO);

    HMONITOR hmonitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);
    GetMonitorInfo(hmonitor, &target);

    hwnd = CreateWindowEx(
        WS_EX_TRANSPARENT,
        wc.lpszClassName,
        L"",
        WS_POPUPWINDOW,
        0, 0, target.rcMonitor.right, target.rcMonitor.bottom,
        NULL, NULL, wc.hInstance, NULL);

    if (!hwnd) {
        return 0;
    }

    char szBuffer[1024];

    #ifdef WIN32
        WSADATA wsaData;
        WORD wVersionRequested = MAKEWORD(2, 0);
        if (::WSAStartup(wVersionRequested, &wsaData) != 0)
            return false;
    #endif


        if (gethostname(szBuffer, sizeof(szBuffer)) == SOCKET_ERROR)
        {
    #ifdef WIN32
            WSACleanup();
    #endif
            return false;
        }

        struct hostent* host = gethostbyname(szBuffer);
        if (host == NULL)
        {
    #ifdef WIN32
            WSACleanup();
    #endif
            return false;
        }

        /*std::cout << (int)((struct in_addr*)(host->h_addr))->S_un.S_un_b.s_b1 << "." << 
                     (int)((struct in_addr*)(host->h_addr))->S_un.S_un_b.s_b2 << "." << 
                     (int)((struct in_addr*)(host->h_addr))->S_un.S_un_b.s_b3 << "." << 
                     (int)((struct in_addr*)(host->h_addr))->S_un.S_un_b.s_b4;*/


    //WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    std::cout << "System operacyjny: " << os << std::endl;

    // Server connection
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        throw std::runtime_error("Error while creating socket.\n");
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP_ADDR);
    serverAddr.sin_port = htons(PORT);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(clientSocket);
        throw std::runtime_error("Error while connecting to server.\n");
    }
    
    switch (os)
    {
    case 0:
    {
        std::thread thread(windows, clientSocket);

        MSG msg = {};

        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        break;
    }
    case 1:
    {
        const char* cmd = "zenity --info --text='Hello, Linux MessageBox!'";

        system(cmd);
        break;
    }
    default:
        break;
    }

    closesocket(clientSocket);
    WSACleanup();
    
    return 0;
}