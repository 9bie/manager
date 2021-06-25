# swap layout
from .config import *
#from .ws import create_server, get_action, broadcast
from .web import create_server
import asyncio
from json import dumps
import threading
import os
import signal
import platform
loop = asyncio.get_event_loop()


# def __backend2ws():
#     while True:
#         action = web.get_action()
#         print("[!][B2W]Action:{}".format(action))
#         asyncio.run(
#             ws.broadcast(dumps(action))
#         )


# def __ws2backend():
#     while True:
#         action = ws.get_action()
#         print("[!][W2B]Action:{}".format(action))
#         if "uuid" in action and "do" in action:
#             web.do_action(action["uuid"], action["do"])




def create_app():
    print("[+]App Running...")
    # threading.Thread(target=__ws2backend).start()
    # threading.Thread(target=__backend2ws).start()

    # loop.run_until_complete(ws.create_server(CONFIG["websocket"]["host"], CONFIG["websocket"]["port"]))
    loop.run_until_complete(
        web.create_server()
    )
    loop.run_forever()


def close_app():
    if platform.system() == "Windows":
        pid = os.getpid()
        os.popen('taskkill.exe /F /pid:' + str(pid))
    else:
        os.kill(os.getpid(),signal.SIGSTOP)
