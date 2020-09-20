from json import loads,dumps
from queue import Queue
from ..web import online_list
import asyncio
import websockets

events = Queue()
user_list = set()


def broadcast(message):
    for i in user_list:
        i.send(message)


def get_action():
    return events.get()


async def check_permit(w):

    while True:
        recv = await w.recv()
        cred_dict = loads(recv)

        if cred_dict["username"] == "admin" and cred_dict["password"] == "123456":
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
    while True:
        recv = await w.recv()
        data = loads(recv)
        events.put(data)


async def main_logic(w, path):
    print("[+][ws]Request Uri:{}".format(path))
    try:
        await check_permit(w)

        await handle(w)
    finally:
        user_list.remove(w)

