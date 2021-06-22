# coding:utf-8
import asyncio
from app.utils import *
from threading import Timer
from time import sleep


Conn = {}


def get_list():
    l = []
    for k in Conn:
        l.append({
            "uuid": k,
            "conn": Conn[k],

        })
    return l


def get_action():
    return events.get()


def do_action(uuid, do_something):
    if uuid not in Conn:
        return False
    Conn[uuid]["do"].append(do_something)
    return True


def online(uuid):
    print("[!][BackEnd]Online:{}".format(uuid))


def offline(uuid):
    if uuid in Conn:
        Conn.pop(uuid)


def handle(packet,ip):
    raw = rc4_decode(packet)
    data = loads(raw)
    first = False
    if "uuid" not in data or "info" not in data:
        return "", 404
    else:
        pass
    return rc4_encode(
        
    )
