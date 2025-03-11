#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "client.h"

Client::Client()
{
}

Client::Client(std::string IP) : IP(IP)
{
}

Client::Client(std::string IP, std::string name) : IP(IP), name(name)
{
}

void Client::clientHandler(SOCKET clientSocket)
{
    char clientID[256];
    
    while (true) {
        
        this->mtx.lock();
        if (status == "messageBox")
        {
            send(clientSocket, this->status.c_str(), this->status.size() + 1, 0);
            send(clientSocket, this->title.c_str(), this->title.size() + 1, 0);
            send(clientSocket, this->message.c_str(), this->message.size() + 1, 0);
        }
        else if (status == "block")
        {
            this->bl = true;
            std::string message = "block";
            send(clientSocket, message.c_str(), message.size() + 1, 0);
        }
        else if (status == "unblock")
        {
            this->bl = false;
            std::string message = "unblock";
            send(clientSocket, message.c_str(), message.size() + 1, 0);
        }
        else
        {
            std::string message = "None";
            send(clientSocket, message.c_str(), message.size() + 1, 0);
        }

        this->ver = !this->ver;
        
        std::fstream file;
        file.open("Resources\\Screenshot\\screenshot" + this->name + (this->ver ? "_A" : "_B") + ".bmp", std::ios::binary | std::ios::out);


        if (!file.good()) {
            std::cerr << "Could not open file: " << "Rescources/Screenshot/screenshot" + this->name << std::endl;
            this->mtx.unlock();
            return;
        }

        char buffer[BUFFER];
        //std::cout << recv(clientSocket, buffer, BUFFER, NULL) << std::endl;
        recv(clientSocket, buffer, BUFFER, NULL);
        std::string s = buffer;
        int counter, i = 0, bits = std::stoi(s);
        while (bits > 0 && (counter = recv(clientSocket, buffer, BUFFER, NULL)) > 0) {
            file.write(buffer, counter);
            bits -= counter;
            std::cout << ++i << std::endl;
        }

        this->active = true;
        
        file.close();

        this->mtx.unlock();

        InvalidateRect(hwnd, NULL, FALSE);
        UpdateWindow(hwnd); // Natychmiastowe wywo³anie WM_PAINT
    }

    closesocket(clientSocket);
}

std::string Client::getIP()
{
    return this->IP;
}

void Client::setName(std::string name)
{
    this->name = name;
}

std::string Client::getName()
{
    return this->name;
}

void Client::setStatus(std::string status)
{
    this->status = status;
}

std::string Client::getStatus()
{
    return this->status;
}

void Client::setMessage(std::string message)
{
    this->message = message;
}

std::string Client::getMessage()
{
    return this->message;
}

void Client::setTitle(std::string title)
{
    this->title = title;
}

std::string Client::getTitle()
{
    return this->title;
}

void Client::setThread(std::thread* t)
{
    this->t = t;
}

std::thread* Client::getThread()
{
    return this->t;
}

void Client::setActive()
{
    this->active = false;
}

bool Client::getActive()
{
    return active;
}

void Client::setNr(int nr)
{
    this->nr = nr;
}

int Client::getNr()
{
    return nr;
}
