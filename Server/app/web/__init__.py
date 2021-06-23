from .web import app
from ..config import CONFIG


def create_server():
    print("[+]Web Server Running...")
    # start_server = app.run(debug=debug)
    print("[+]BackEnd address: {}".format(CONFIG["web"]["backend"]))
    print("[+]Web Control address: {}".format(CONFIG["web"]["control"]))

    return app.run_task(host=CONFIG["web"]["host"],
                        port=CONFIG["web"]["port"],
                        debug=CONFIG["web"]["debug"])

