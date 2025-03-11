#pragma once
#include <string>
#include <mutex>
#include <Windows.h>

#define BUFFER 16384

extern HWND hwnd;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class Client {
private:
	const std::string IP;
	std::string name;

	std::string status;
	std::string message;
	std::string title;
	bool bl = false;

	bool ver = false;

	std::thread* t = nullptr;
	std::mutex mtx;

	RECT rect;
	HWND button;
	HWND sendButton;
	HBITMAP img;

	int nr;

	static int sizeX;
	static int sizeY;
	int posX;
	int posY;

	bool active;

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
	void setThread(std::thread* t);
	std::thread* getThread();
	void setActive();
	bool getActive();
	void setNr(int nr);
	int getNr();

	friend LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

