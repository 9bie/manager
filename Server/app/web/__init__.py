from .web import app,get_action,do_action,online_list
from quart import *
from ..config import CONFIG


def create_server():
    print("[+]Web Server Running...")
    # start_server = app.run(debug=debug)
    print("[+]BackEnd address: {}".format(CONFIG["web"]["backend"]))
    print("[+]Web Control address: {}".format(CONFIG["web"]["web"]))

    return app.run_task(host=CONFIG["web"]["host"],
                        port=CONFIG["web"]["port"],
                        debug=CONFIG["web"]["debug"])

