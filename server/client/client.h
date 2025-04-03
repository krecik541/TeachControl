#pragma once
#include <string>
#include <mutex>
#include <Windows.h>
#include "../GUI/Button.h"

#define BUFFER 16384

class Button; 

class Client {
private:
	const std::string IP;

	std::string name;

	std::string status;
	std::string message;
	std::string title;
	bool blocked = false;

	bool imgVersion = false;

	std::thread* t = nullptr;
	std::mutex mtx;

	RECT rect;
	Button button;
	Button sendButton;
	HBITMAP img;

	int nr = -1;

	bool active;

public:
	static HWND hwnd;

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
	void setThread(std::thread* t);
	std::thread* getThread();
	void setActive();
	bool getActive();
	void setNr(int nr);
	int getNr();
	Button getButton();
	Button getSendButton();

	friend LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

