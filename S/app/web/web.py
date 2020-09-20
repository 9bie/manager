from .backend import *
from quart import *

app = Quart(__name__,static_folder="static")
WEB = "/web/"
BACKEND = "/backend/"


@app.route(WEB)
async def web_control():
    return await render_template("manager.html")


@app.route(BACKEND)
async def backend():
    # todo:check UA and other
    data = request.form
    return handle(data)


