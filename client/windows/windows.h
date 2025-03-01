#pragma once
#include <Windows.h>
#include <vector>
#include <map>

#define BUFFER 16384

HBITMAP GetScreenShot();
void windows(SOCKET clientSocket);

bool SaveBitmapToVector(HBITMAP hBitmap, std::vector<BYTE>& outData);
HBITMAP LoadBitmapFromVector(const std::vector<BYTE>& data, int width, int height);
void saveClipboard(std::map<UINT, std::vector<BYTE>>& clipboard, std::vector<BYTE>& bitmapData, int& width, int& height);
void restoreClipboard(const std::map<UINT, std::vector<BYTE>>& clipboard, const std::vector<BYTE>& bitmapData, int width, int height);