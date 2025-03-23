#include <iostream>
#include <cstring>
#include <codecvt>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <vector>
#include <map>

#ifdef _WIN32  // Windows
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #include <Windows.h>
#else  // Linux & Mac
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif


#include "client/client.h"

#pragma comment(lib, "Ws2_32.lib")

#define IP_ADDR "127.0.0.1"
#define PORT 8888

#define LEFT client->rect.left
#define TOP client->rect.top
#define RIGHT client->rect.right
#define BOTTOM client->rect.bottom

#define WM_NEW_CLIENT (WM_USER + 1)

HWND button, sendMessage, hwnd;

bool g_running = true;

std::map<int, Client*> clients;

int counter = 0;

//61
//bazwa u¿ytkownika

void server(HWND hwnd) {
    MSG msg = {};
    std::mutex mtx;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed.\n");
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        throw std::runtime_error("Error while creating socket.\n");
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == INVALID_SOCKET) {
        throw std::runtime_error("Error while binding socket.\n");
    }

    if (listen(serverSocket, SOMAXCONN) == INVALID_SOCKET) {
        throw std::runtime_error("Error while Trying to connect.\n");
    }

    while (g_running) {
        struct sockaddr_in client_addr;
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&client_addr, NULL);

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error accepting connection\n";
            break;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        //std::cout << "Po³¹czony klient: " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

        Client* client = new Client(client_ip);
        client->setName("stanowisko" + std::to_string(counter));
        client->setThread(new std::thread(&Client::clientHandler, client, clientSocket));
        client->setNr(counter);

        PostMessage(hwnd, WM_USER + 1, 0, (LPARAM)client);

        clients.insert({ counter++, client });
    }

    closesocket(serverSocket);

    for (auto q : clients) {
        Client* t = q.second;
        t->getThread()->join();
        delete t->getThread();
        delete t;
    }
    clients.clear();

    WSACleanup();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static BOOL bCollectPoints;
    static POINT ptMouseDown[32];
    static int index;
    POINTS ptTmp;
    RECT rc;

    Client* client = (Client*)lParam;
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    switch (uMsg)
    {
    case WM_COMMAND:
    {

        switch (wParam % 2)
        {
        case 0:
        {
            client = clients[wParam / 2];

            HWND button = (HWND)lParam;

            wchar_t buffer[256];
            GetWindowText(button, buffer, sizeof(buffer));
            std::wstring s(buffer);
            if (s == L"ODBLOKUJ")
            {
                SetWindowText(button, L"ZABLOKUJ");
                client->setStatus("unblock");
            }
            else
            {
                SetWindowText(button, L"ODBLOKUJ");
                client->setStatus("block");
            }
            break;
        }
        case 1:
        {
            break;
            // TWORZENIE OKNA DO SEND
        }
        }
        break;
    }
    case WM_PAINT:
    {
        try 
        {
            for (auto& cl : clients) {
                Client* client = cl.second;
                
                if (!client->getActive())
                    continue;
                
                client->setActive(); 

                //FillRect(GetDC(hwnd), &client->rect, (HBRUSH)CreateSolidBrush(0x00AA2DDA

                std::wstring narrow = converter.from_bytes("Resources\\Screenshot\\screenshot" + client->name + (client->ver ? "_A" : "_B") + ".bmp");

                // Za³aduj obraz
                HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, narrow.c_str(), IMAGE_BITMAP, 192, 108, LR_LOADFROMFILE);
                if (!hBitmap) {
                    MessageBox(hwnd, L"Nie mo¿na za³adowaæ obrazu!", L"B³¹d", MB_ICONERROR);
                    return -1;
                }

                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                HDC hdcMem = CreateCompatibleDC(hdc);
                if (!hdcMem) {
                    MessageBox(hwnd, L"CreateCompatibleDC zwróci³o NULL!", L"B³¹d", MB_ICONERROR);
                    DeleteObject(hBitmap);
                    return -1;
                }

                HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);
                if (!oldBitmap) {
                    MessageBox(hwnd, L"SelectObject zwróci³o NULL!", L"B³¹d", MB_ICONERROR);
                    DeleteDC(hdcMem);
                    DeleteObject(hBitmap);
                    return -1;
                }

                // Pobranie informacji o bitmapie
                BITMAP bitmap;
                if (!GetObject(hBitmap, sizeof(bitmap), &bitmap)) {
                    MessageBox(hwnd, L"GetObject nie zwróci³ poprawnych danych!", L"B³¹d", MB_ICONERROR);
                    SelectObject(hdcMem, oldBitmap);
                    DeleteDC(hdcMem);
                    DeleteObject(hBitmap);
                    return -1;
                }

                RECT rectW;
                GetWindowRect(hwnd, &rectW);
                RECT rect = { 35 + (clients.size() - 1) % 4 * (192 + (rectW.right - rectW.left - 838) / 3), 30 + (clients.size() - 1) / 4 * (143 + (rectW.bottom - rectW.top - 780) / 4), 0, 0 };
                rect.right = rect.left + 192;
                rect.bottom = rect.top + 143;

                // Rysowanie obrazu
                BitBlt(hdc, rect.left, rect.top + 15, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

                // Zwolnienie zasobów
                SelectObject(hdcMem, oldBitmap);
                DeleteDC(hdcMem);
                DeleteObject(hBitmap);
                EndPaint(hwnd, &ps);
            }
        }
        catch (...)
        {

        }
        break;
    }
    case WM_DESTROY:
        g_running = false;
        PostQuitMessage(0);
        break;
    case WM_NEW_CLIENT:
    {
        RECT rectW;
        GetWindowRect(hwnd, &rectW);
        RECT rect = { 35 + (clients.size() - 1) % 4 * (192 + (rectW.right - rectW.left - 838) / 3), 30 + (clients.size() - 1) / 4 * (143 + (rectW.bottom - rectW.top - 780) / 4), 0, 0 };
        rect.right = rect.left + 192;
        rect.bottom = rect.top + 143;


        client->button = CreateWindowEx(0, L"BUTTON", client->bl ? L"ODBLOKUJ" : L"ZABLOKUJ",
            WS_CHILD | WS_VISIBLE, rect.left, rect.bottom - 20, 96, 20,
            hwnd, (HMENU)(client->nr * 2), GetModuleHandle(NULL), NULL);

        client->sendButton = CreateWindowEx(0, L"BUTTON", L"WYŒLIJ",
            WS_CHILD | WS_VISIBLE, rect.right - 96, rect.bottom - 20, 96, 20,
            hwnd, (HMENU)(client->nr * 2 + 1), GetModuleHandle(NULL), NULL);

        rect.top = rect.top + 25;
        std::wstring name = converter.from_bytes(client->name);
        DrawText(GetWindowDC(hwnd), name.c_str(), -1, &rect, DT_RIGHT);
        

        break;
    }
    case WM_TIMER:

        break;
    default:
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"Window";
    wc.style = 0;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    //wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClass(&wc);

    hwnd = CreateWindowEx(
        WS_EX_WINDOWEDGE,
        wc.lpszClassName,
        L"Kontrola sali",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 958, 820,
        NULL, NULL, wc.hInstance, NULL);

    if (!hwnd) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    std::thread serverThread(server, hwnd);

    MSG msg = {};

    while (GetMessage(&msg, NULL, 0, 0) && g_running) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    g_running = false;
    serverThread.join();

    return 0;
}