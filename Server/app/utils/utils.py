import os, time
from json import loads, dumps
from time import time


def generate_client(address, t):
    windows_x86_path = os.path.join("bin", "windows_x86.exe")
    windows_x86_64_path = os.path.join("bin", "windows_x86_64.exe")
    windows_dll_x86 = os.path.join("bin", "windows_dll_x86.dll")
    windows_dll_x86_64 = os.path.join("bin", "windows_dll_x86_64.dll")
    linux_x86 = os.path.join("bin", "linux_x86")
    linux_x86_64 = os.path.join("bin", "linux_x86_64")
    linux_arm = os.path.join("bin", "linux_arm")
    mac = os.path.join("bin", "mac")
    if t == "windows_x86":
        raw = windows_x86_path
        f1 = raw + ".exe"
    elif t == "windows_x86_64":
        raw = windows_x86_64_path
        f1 = raw + ".exe"
    elif t == "windows_dll_x86":
        raw = windows_dll_x86
        f1 = raw + ".dll"
    elif t == "windows_dll_x86_64":
        raw = windows_dll_x86_64
        f1 = raw + ".dll"
    elif t == "linux_x86":
        raw = linux_x86
        f1 = raw
    elif t == "linux_x86_64":
        raw = linux_x86_64
        f1 = raw
    elif t == "linux_arm":
        raw = linux_arm
        f1 = raw

    elif t == "mac":
        raw = mac
        f1 = raw
    else:
        return False, "未知错误", ""
    if os.path.isfile(raw) is False:
        return False, "源文件不存在", ""
    f = open(raw, "rb")
    b = f.read()
    space = 80 - len(address)
    last = address + " " * space
    i = 0
    newberry = bytearray(b)
    p = newberry.find(("b" * 80).encode())
    for i2 in last:
        newberry[p + i] = ord(i2)
        i += 1
    filename = str(int(time())) + ".bin"
    path = os.path.join("app", "web", "static", filename)
    f2 = open(path, "wb")
    f2.write(newberry)
    f2.close()
    f.close()
    return True, filename, f1
