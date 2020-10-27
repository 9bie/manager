from .web import app,get_action,do_action,online_list
from quart import *


def create_server(config):
    print("[+]Web Server Running...")
    # start_server = app.run(debug=debug)
    return app.run_task(host=config["web"]["host"],
                        port=config["web"]["port"],
                        debug=config["web"]["debug"])

