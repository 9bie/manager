import os, time
from json import loads, dumps
from arc4 import ARC4
from time import time


def rc4_encode(i):
    key = int(time()) / 100
    arc4 = ARC4(str(int(key)))
    enc = arc4.encrypt(i)
    return enc


def rc4_decode(i):
    key = int(time()) / 100
    arc4 = ARC4(str(int(key)))
    dec = arc4.decrypt(i)
    return dec


def generate_client(address, t):
    windows_x86_path = os.path.join("bin", "windows_x86.exe")
    windows_x86_64_path = os.path.join("bin", "windows_x86_64.exe")
    windows_dll = os.path.join("bin", "windows_dll.dll")
    linux_x86 = os.path.join("bin", "linux_x86")
    linux_x86_64 = os.path.join("bin", "linux_x86_64")
    linux_arm = os.path.join("bin", "linux_arm")
    mac_x86 = os.path.join("bin", "mac_x86")
    mac_x86_64 = os.path.join("bin", "mac_x86_64")
    mac_arm = os.path.join("bin", "mac_arm")
    if t == "windows_x86":
        raw = windows_x86_path
        f1 = raw + ".exe"
    elif t == "windows_x86_64":
        raw = windows_x86_64_path
        f1 = raw + ".exe"
    elif t == "windows_dll":
        raw = windows_dll
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
    elif t == "mac_x86":
        raw = mac_x86
        f1 = raw
    elif t == "mac_x86_64":
        raw = mac_x86_64
        f1 = raw
    elif t == "mac_arm":
        raw = mac_arm
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
