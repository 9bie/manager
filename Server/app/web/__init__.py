from .web import app,get_action,do_action,online_list
from quart import *


def create_server(debug):
    print("[+]Web Server Running...")
    # start_server = app.run(debug=debug)
    return app.run_task(debug=debug)

