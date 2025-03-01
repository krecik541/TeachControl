#pragma once
#include <string>
#include <mutex>
#include <Windows.h>

#define BUFFER 16384

class Client {
private:
	const std::string IP;
	std::string name;

	std::string status;
	std::string message;
	std::string title;

	HBITMAP img;

public:
	std::thread* t = nullptr;
	std::mutex mtx;
	HWND button;

public:
	Client();
	Client(std::string IP);
	Client(std::string IP, std::string name);

	void clientHandler(SOCKET clientSocket);

	std::string getIP();
	void setName(std::string name);
	std::string getName();
	void setStatus(std::string status);
	std::string getStatus();
	void setMessage(std::string message);
	std::string getMessage();
	void setTitle(std::string title);
	std::string getTitle();
};

