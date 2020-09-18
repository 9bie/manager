from .web import app,get_action,do_action
import asyncio


async def create_server(debug):
    start_server = app.run(debug=debug)
    asyncio.get_event_loop().run_until_complete(start_server)
    asyncio.get_event_loop().run_forever()
