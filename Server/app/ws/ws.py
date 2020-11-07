from json import loads,dumps
from queue import Queue
from ..web import online_list
from ..config import CONFIG
import asyncio
import websockets

events = Queue()
user_list = set()


async def broadcast(message):
    for i in user_list:
        await i.send(message)


def get_action():
    return events.get()


async def check_permit(w):

    while True:
        recv = await w.recv()
        cred_dict = loads(recv)

        if cred_dict["username"] == CONFIG["base"]["username"] and cred_dict["password"] == CONFIG["base"]["password"]:
            user_list.add(w)
            await w.send(dumps({
                "login": True,
                "data": online_list()
            }))
            return True
        else:
            await w.send(dumps({
                "data": False
            }))


async def handle(w):
    try:
        while True:
            recv = await w.recv()
            data = loads(recv)
            events.put(data)
    except Exception as e:
        if w in user_list:
            user_list.remove(w)


async def main_logic(w, path):
    print("[+][ws]Request Uri:{}".format(path))
    try:
        await check_permit(w)

        await handle(w)
    finally:
        if w in user_list:
            user_list.remove(w)

