#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <vector>

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

HWND button;

void GUIhandler(Client* clients) {

}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg)
    {
    case WM_COMMAND:
    {
        LPWSTR s;
        GetWindowText(button, s, 10);
        std::cout << s << std::endl;
        SetWindowText(button, L"ODBLOKUJ");

        switch (wParam)
        {
        
        }
        break;
    }
    case WM_PAINT:
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

    HWND hwnd = CreateWindowEx(
        WS_EX_WINDOWEDGE,
        wc.lpszClassName,
        L"Kontrola sali",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, wc.hInstance, NULL);

    if (!hwnd) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    button = CreateWindowEx(0, L"BUTTON", L"ZABLOKUJ"
        , WS_CHILD | WS_VISIBLE, 100, 100, 150, 30, hwnd, NULL, hInstance, NULL);

    MSG msg = {};

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //std::cout << "Dopasowanie nazwy u¿ytkownika do IP\n";
    /*std::mutex mtx;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
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

    std::vector<Client*> clients;

    while (true) {
        struct sockaddr_in client_addr;
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&client_addr, NULL);
        
        if (clientSocket == INVALID_SOCKET) {
            std::cout << "Error accepting connection\n";
            break;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "Po³¹czony klient: " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;
        
        Client *client = new Client(client_ip);
        client->t = new std::thread(&Client::clientHandler, client, clientSocket);
        client->button = CreateWindowEx(0, L"BUTTON", L"Client" + clients.size()
            , WS_CHILD | WS_VISIBLE, 100, 100, 150, 30, hwnd, NULL, hInstance, NULL);
        
        clients.push_back(client);
        
        client->t->detach();
    }

    closesocket(serverSocket);

    for (auto t : clients) {
        t->t->join();
        delete t->t;
        delete t;
    }
    clients.clear();

    WSACleanup();*/

    return 0;
}