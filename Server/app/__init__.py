# swap layout
from .ws import create_server, get_action, broadcast
from .web import create_server, get_action, do_action
import asyncio
from json import dumps
import threading

def __backend2ws():

    while True:
        action = web.get_action()
        print("[!][B2W]Action:{}".format(action))
        asyncio.run(
            ws.broadcast(dumps(action))
        )



def __ws2backend():

    while True:
        action = ws.get_action()
        print("[!][W2B]Action:{}".format(action))

        web.do_action(action["uuid"], action["do"])


def create_app(config):
    print("[+]App Running...")
    threading.Thread(target=__ws2backend).start()
    threading.Thread(target=__backend2ws).start()
    loop = asyncio.get_event_loop()

    loop.run_until_complete(ws.create_server(config["websocket"]["host"], config["websocket"]["port"]))
    loop.run_until_complete(
        web.create_server(config["web"]["debug"])
    )
    loop.run_forever()
