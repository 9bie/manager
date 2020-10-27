from .backend import *
from ..utils import generate_client
from quart import *

# import logging
app = Quart(__name__, static_folder="static")
WEB = "/web/"
BACKEND = "/backend/"


# handler = logging.FileHandler()
# app.logger.addHandler(handler)


@app.route(WEB)
async def web_control():
    return await render_template("manager.html")


@app.route(BACKEND, methods=['POST'])
async def backend():
    # todo:check UA and other
    data = await request.get_data()
    return handle(data, request.remote_addr)
    # 如果使用了反代，请把这行代码修改为
    # return handle(data,request.headers['X-Real-Ip'])
    # 同时在nginx中添加如下参数
    # proxy_set_header Host $host: 8080;
    # proxy_set_header X-Real-IP $remote_addr;
    # proxy_set_header REMOTE-HOST $remote_addr;
    # proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;


@app.route('/generate/', methods=['GET'])
async def generate():
    domain = request.args.get("domain")
    version = request.args.get("version")
    ok, result,filename = generate_client(domain, version)
    if not ok:
        return result
    else:
        return await send_from_directory(app.static_folder, result, as_attachment=True,attachment_filename="")
