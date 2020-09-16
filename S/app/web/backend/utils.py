from json import loads, dumps
from arc4 import ARC4
from time import time


def rc4_encode(i):
    key = int(time()) / 100
    arc4 = ARC4(str(int(key)))
    enc = arc4.encrypt(i)
    return enc


def rc4_decode(i):
    key = int(time())/100
    arc4 = ARC4(str(int(key)))
    dec = arc4.decrypt(i)
    return dec




