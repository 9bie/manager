from .backend import *
from quart import *
app = Quart(__name__)
WEB = "/web"
BACKEND = "/backend"


@app.route(WEB)
async def web_control():
    pass


@app.route(BACKEND)
async def backend():
    # todo:check UA and other
    data = request.form
    return handle(data)


