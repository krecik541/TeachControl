#pragma once
#include <Windows.h>
//#include "../client/client.h"


class Client;

class Button {
private:
	int id;
	HWND hwnd;
	Client *client;
public:
	static HWND hwndW;
	static HWND sendHWND;
	static HWND title;
	static HWND message;
	void create(LPCWSTR title, int posLeft, int posTop, int id, Client* client);
	void setVisible(bool f);
	void changeText();
	void send();
};