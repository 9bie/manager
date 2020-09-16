# coding:utf-8
import queue
from .utils import  *
events = queue.Queue()
Conn = {}


def get_action():
    return events.get()


def handle(packet):
    raw = rc4_decode(packet)
    data = loads(raw)
    if "uuid" not in data or "info" not in data:
        return
    else:
        if data["uuid"] not in Conn:
            Conn[data["uuid"]] = {
                "info":data["info"],
                "next_second": int(time())+60
            }
        if "next_second" in data:
            # 更新表中的时间为下次检测日期
            Conn[data["uuid"]]["next_second"] = int(time())+data["next_second"]


