from .ws import *


def create_server(host, port):
    # 把ip换成自己本地的ip
    print("[+]Websocket Server Running...")
    start_server = websockets.serve(main_logic, host, port)
    '''
        asyncio.get_event_loop().run_until_complete(start_server)
        asyncio.get_event_loop().run_forever()
    '''
    return start_server

