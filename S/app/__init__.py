# swap layout
from .ws import create_server, get_action, broadcast
from .web import create_server, get_action, do_action
import asyncio
from json import loads


async def __backend2ws():
    while True:
        action = web.get_action()
        ws.broadcast(loads(action))


async def __ws2backend():
    while True:
        action = ws.get_action()
        web.do_action(action["uuid"], action["do"])


def create_app(config):
    swap_w2b = asyncio.create_task(
        __ws2backend()
    )
    swap_b2w = asyncio.create_task(
        __backend2ws()
    )
    ws_task = asyncio.create_task(
        ws.create_server(config["websocket"]["host"], config["websocket"]["port"])
    )

    web_task = asyncio.create_task(
        web.create_server(config["web"]["debug"])
    )
    await swap_b2w
    await swap_w2b
    await ws_task
    await web_task
