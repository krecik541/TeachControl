#include "Button.h"
#include<iostream>
#include <string>
#include "../client/client.h"

HWND Button::hwndW = nullptr;
HWND Button::sendHWND = nullptr;
HWND Button::title = nullptr;
HWND Button::message = nullptr;

void Button::create(LPCWSTR title, int posLeft, int posTop, int id, Client* client)
{
    this->id = id;
    this->client = client;
    this->hwnd = CreateWindowEx(0, L"BUTTON", title,
        WS_CHILD | WS_VISIBLE, posLeft, posTop - 20, 96, 20,
        Button::hwndW, (HMENU)(id), GetModuleHandle(NULL), NULL);

}

void Button::setVisible(bool f)
{
    ShowWindow(hwnd, f ? SW_SHOW : SW_HIDE);
}

void Button::changeText()
{
    wchar_t buffer[256];
    GetWindowText(this->hwnd, buffer, sizeof(buffer));
    std::wstring s(buffer);
    if (s == L"ODBLOKUJ")
    {
        SetWindowText(this->hwnd, L"ZABLOKUJ");
        client->setStatus("unblock");
    }
    else
    {
        SetWindowText(this->hwnd, L"ODBLOKUJ");
        client->setStatus("block");
    }
}

void Button::send()
{
    HWND hww = this->hwndW;
    RECT rect = {};
    rect.left = 15;
    rect.top = 15;
    rect.right = 185;
    rect.bottom = 30;

    DrawText(GetWindowDC(sendHWND), L"abc", -1, &rect, DT_RIGHT);
    
    ShowWindow(sendHWND, SW_SHOW);
    UpdateWindow(sendHWND);
}
