CONFIG = {"web": {}, "websocket": {}, "database": {}, "base": {}}

# Base Access
CONFIG["base"]["secret"] = "this_is secret"
CONFIG["base"]["default_sleep"] = 30
CONFIG["base"]["log"] = "running.log"
# Web Control Access
CONFIG["web"]["debug"] = True
CONFIG["web"]["host"] = "0.0.0.0"
CONFIG["web"]["port"] = "5000"

# Web Sockets Access
CONFIG["websocket"]["host"] = "127.0.0.1"
CONFIG["websocket"]["port"] = 3287

# DataBase Access
CONFIG["database"]["is_mysql"] = False
