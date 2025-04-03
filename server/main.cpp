#include <iostream>
#include <cstring>
#include <codecvt>
#include <sstream>
#include <string>
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
#include "GUI/Button.h"

#pragma comment(lib, "Ws2_32.lib")

#define IP_ADDR "127.0.0.1"
#define PORT 8888

#define WM_NEW_CLIENT (WM_USER + 1)
#define WM_DEL_CLIENT (WM_USER + 2)

HWND hwnd, sendHWND;

bool g_running = true;

std::map<int, Client*> clients;

Client* cl;

int counter = 0;

//61
//bazwa użytkownika



void server(HWND hwnd);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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

    WNDCLASS sendClass = {};
    sendClass.lpfnWndProc = WindowProc;
    sendClass.hInstance = hInstance;
    sendClass.lpszClassName = L"Window";
    sendClass.style = 0;
    sendClass.cbClsExtra = 0;
    sendClass.cbWndExtra = 0;
    sendClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    sendClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    sendClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    sendClass.lpszMenuName = NULL;
    //wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClass(&sendClass);

    sendHWND = CreateWindowEx(
        WS_EX_WINDOWEDGE,
        sendClass.lpszClassName,
        L"Kontrola sali",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 250, 400,
        hwnd, NULL, sendClass.hInstance, NULL);

    if (!sendHWND) {
        return 0;
    }

    RECT rect = {};
    rect.left = 15;
    rect.top = 30;
    rect.right = 235;
    rect.bottom = 370;


    HWND hTitle = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
        WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, rect.left, rect.top, 205, 30, sendHWND, NULL, hInstance, NULL);
    
    HWND hMessage = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
        WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, rect.left, rect.top + 60, 205, 150, sendHWND, NULL, hInstance, NULL);
    
    //HWND hMessage = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
        //WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL, rect.left, rect.top + 60, 205, 150, sendHWND, NULL, hInstance, NULL);
    
    HWND hCancelButton = CreateWindowEx(0, L"BUTTON", L"ANULUJ",
        WS_CHILD | WS_VISIBLE, rect.left, rect.bottom - 60, 60, 30,
        sendHWND, (HMENU)(0), GetModuleHandle(NULL), NULL);
    
    HWND hSendButton = CreateWindowEx(0, L"BUTTON", L"WYSLIJ",
        WS_CHILD | WS_VISIBLE, rect.right - 60 - 20, rect.bottom - 60, 60, 30,
        sendHWND, (HMENU)(65535), GetModuleHandle(NULL), NULL);
    
    Button::hwndW = hwnd;
    Button::sendHWND = sendHWND;
    Button::title = hTitle;
    Button::message = hMessage;
    Client::hwnd = hwnd;

    ShowWindow(sendHWND, SW_SHOW);
    UpdateWindow(sendHWND);



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

void server(HWND hwnd) {
    MSG msg = {};
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
        //std::cout << "Połączony klient: " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

        Client* client = new Client(client_ip);
        for(int i = 0; i < clients.size(); i++)
            if(!clients[i])
            {
                client->setNr(i);
                break;
            }
        client->setName("stanowisko" + std::to_string(counter));// ? getName from file
        client->setThread(new std::thread(&Client::clientHandler, client, clientSocket));

        if (client->getNr() == -1)
            client->setNr(counter++);

        clients.insert({ client->getNr(), client});

        PostMessage(hwnd, WM_USER + 1, 0, (LPARAM)client);
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
        if (wParam == 0)
            ShowWindow(sendHWND, SW_HIDE);
        else if (wParam == 65535)
        {
            wchar_t buffer[256];
            GetWindowText(Button::title, buffer, sizeof(buffer));
            std::wstring s(buffer);
            cl->title = converter.to_bytes(s);
            GetWindowText(Button::message, buffer, sizeof(buffer));
            s = buffer;
            cl->message = converter.to_bytes(s);
            cl->status = "messageBox";
        }
        else if (wParam > clients.size() * 10)
            break;
        else
        {
            client = clients[wParam / 2];
            if (wParam % 2 == 0)
                client->getButton().changeText();
            else
            {
                client->getSendButton().send();
                cl = client;
            }
        }
        break;
    }
    case WM_PAINT:
    {
        RECT rectW;
        GetWindowRect(hwnd, &rectW);
        try
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(sendHWND, &ps); // Pobranie HDC dla obszaru rysowania

            RECT r = { 15, 10, 235, 30 };
            SetTextColor(hdc, RGB(0, 0, 0)); // Czerwony tekst
            SetBkMode(hdc, TRANSPARENT); // Przezroczyste tło
            DrawText(hdc, L"Tytul wiadomosci", -1, &r, DT_CENTER);
            r = {15, 70, 235, 90};
            DrawText(hdc, L"Tresc wiadomosci", -1, &r, DT_CENTER);
            EndPaint(sendHWND, &ps); // Zakończenie rysowania


            for (auto& cl : clients) {
                Client* client = cl.second;

                if (!client->getActive())
                    continue;

                client->setActive();

                std::wstring narrow = converter.from_bytes("Resources\\Screenshot\\screenshot" + client->name + (client->imgVersion ? "_A" : "_B") + ".bmp");

                // Załaduj obraz
                HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, narrow.c_str(), IMAGE_BITMAP, 192, 108, LR_LOADFROMFILE);
                if (!hBitmap) {
                    break;
                    //throw std::runtime_error("Nie można załadować obrazu!\n");;
                }

                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                HDC hdcMem = CreateCompatibleDC(hdc);
                if (!hdcMem) {
                    DeleteObject(hBitmap);
                    break;
                    //throw std::runtime_error("CreateCompatibleDC zwróciło NULL!\n");;
                }

                HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);
                if (!oldBitmap) {
                    DeleteDC(hdcMem);
                    DeleteObject(hBitmap);
                    break;
                    //throw std::runtime_error("SelectObject zwróciło NULL!\n");;
                }

                // Pobranie informacji o bitmapie
                BITMAP bitmap;
                if (!GetObject(hBitmap, sizeof(bitmap), &bitmap)) {
                    SelectObject(hdcMem, oldBitmap);
                    DeleteDC(hdcMem);
                    DeleteObject(hBitmap);
                    break;
                    //throw std::runtime_error("GetObject nie zwrócił poprawnych danych!\n");;
                }

                RECT rect = { 35 + (clients.size() - 1) % 4 * (192 + (rectW.right - rectW.left - 838) / 3), 30 + (clients.size() - 1) / 4 * (143 + (rectW.bottom - rectW.top - 780) / 4), 0, 0 };
                rect.right = rect.left + 192;
                rect.bottom = rect.top + 143;

                // Rysowanie obrazu
                BitBlt(hdc, rect.left, rect.top + 15, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

                rect.top = rect.top + 25;
                std::wstring name = converter.from_bytes(client->name);
                DrawText(GetWindowDC(hwnd), name.c_str(), -1, &rect, DT_RIGHT);

                // Zwolnienie zasobów
                SelectObject(hdcMem, oldBitmap);
                DeleteDC(hdcMem);
                DeleteObject(hBitmap);
                EndPaint(hwnd, &ps);
            }
        }
        catch (...)
        {
            //MessageBox(hwnd, L"SelectObject zwróciło NULL!", L"Błąd", MB_ICONERROR);
        }
        break;
    }
    case WM_SIZE:
        RECT rectW;
        GetWindowRect(hwnd, &rectW);
        rectW.right -= rectW.left;
        rectW.bottom -= rectW.top;
        rectW.left = 0;
        rectW.top = 0;
        FillRect(GetDC(hwnd), &rectW, (HBRUSH)CreateSolidBrush(0x00B4B4B4));
        for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); it++)
            it->second->active = true;
        break;
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

        client->button.create(client->blocked ? L"ODBLOKUJ" : L"ZABLOKUJ", rect.left, rect.bottom - 20, client->nr * 2, client);
        client->sendButton.create(L"WYŚLIJ", rect.right - 96, rect.bottom - 20, client->nr * 2 + 1, client);

        rect.top = rect.top + 25;
        std::wstring name = converter.from_bytes(client->name);
        DrawText(GetWindowDC(hwnd), name.c_str(), -1, &rect, DT_RIGHT);

        break;
    }
    case WM_DEL_CLIENT:
    {
        client->getThread()->join();
        clients.erase(clients.find(client->nr));
        delete client;
        break;
    }
    default:
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}