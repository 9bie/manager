# coding:utf-8
import asyncio
from app.utils import *
from threading import Timer
import time
from json import dumps,loads
import html

Conn = {}
Events = {"global":[]}
Do = {}

def get_conn(uuid):
    return Conn[uuid]

def get_list(length=0):
    if length == 0 or length>=len(Conn):
        return Conn
    else:
        # todo: 整个分页
        return Conn
def del_events(uuid):
    if uuid in Events:
        Events[uuid] = []


def add_events(uuid,data):
    if uuid not in Events:
        return 
    else:
        print("[!]Event add from %s\n==========\n%s\n=========="%(uuid,data["data"]))
        data["action"] = html.escape(data["action"])
        data["data"] = html.escape(data["data"])
        Events[uuid].append(data)
        return 


def get_events(uuid,length=20):
    if uuid not  in Events:
        return []
    if len(Events[uuid]) < length:
        return Events[uuid]
    else:
        return Events[uuid][:-length]


def do_action(uuid, do_something):
    if uuid not in Conn and uuid not in Do:
        return 
    if uuid == "all":
        for i in Do.values():
            i.append(do_something)
    else:
        Do[uuid].append(do_something)
    return 

def clear_all(t=600):
    del_conn = []
    for i in Conn.values():
        if int(time.time()) - i["heartbeat"] > int(t):
            del_conn.append(i["uuid"])
    for i in del_conn:
        offline(i)


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
        if(data["uuid"] not in Conn):
            print("[!][BackEnd]Online:{}".format(ip))
            event = {
                "is_client":True,
                "action":"global",
                "time":time.asctime(time.localtime(time.time())),
                "data":"[+]%s:Online IP:%s" % (time.asctime(time.localtime(time.time())),ip)
            }
            add_events("global", event )
        data["info"]["remarks"] = html.escape(data["info"]["remarks"])
        data["info"]["system"] = html.escape(data["info"]["system"])
        data["info"]["user"] = html.escape(data["info"]["user"])
        data["info"]["iip"] = html.escape(data["info"]["iip"])


        Conn[data["uuid"]] = {
        "ip":html.escape(ip),
        "uuid":html.escape(data["uuid"]),
        "sleep":data["sleep"],
        "heartbeat":int(time.time()),
        "info":data["info"],
        }
        if data["uuid"] not in Events:
            Events[data["uuid"]] = []
        else:
            if data["result"]:
                for i in data["result"]:
                    
                   
                    event = {
                        "is_client":True,
                        "action":i["action"],
                        "time":time.asctime(time.localtime(time.time())),
                        "data":i["data"]
                    }
                    add_events(data["uuid"],event)

        if data["uuid"] not in Do:
            Do[data["uuid"]] = []

    ret = dumps(Do[data["uuid"]])

    Do[data["uuid"]] = []
    return ret
