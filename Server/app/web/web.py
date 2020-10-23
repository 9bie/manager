from .backend import *

from quart import *
# import logging
app = Quart(__name__,static_folder="static")
WEB = "/web/"
BACKEND = "/backend/"

# handler = logging.FileHandler()
# app.logger.addHandler(handler)


@app.route(WEB)
async def web_control():
    return await render_template("manager.html")


@app.route(BACKEND,methods=['POST'])
async def backend():
    # todo:check UA and other
    data = await request.get_data()
    return handle(data)


