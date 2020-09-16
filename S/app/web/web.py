from .backend import *
from ...config import *
from flask import *
app = Flask(__name__)


@app.route(WEB)
def web_control():
    pass


@app.route(BACKEND)
def backend():
    # todo:check UA and other
    data = request.form
    return handle(data)




@app.route("/ws")
def websocket():
    pass