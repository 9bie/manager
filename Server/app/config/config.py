CONFIG = {"web": {}, "websocket": {}, "database": {}, "base": {}}

# Base Access
CONFIG["base"]["username"] = "admin"
CONFIG["base"]["password"] = "12345"
CONFIG["base"]["secret"] = "this_is secret"
CONFIG["base"]["default_sleep"] = 30
CONFIG["base"]["log"] = "running.log"
CONFIG["base"]["cdn"] = False
CONFIG["base"]["source_ip_tag"] = "X-Real-Ip"
# Web Control Access
CONFIG["web"]["debug"] = True
CONFIG["web"]["host"] = "0.0.0.0"
CONFIG["web"]["port"] = "5000"
CONFIG["web"]["backend"] = "/backend/"
CONFIG["web"]["web"] = "/web2/"


# Web Sockets Access
CONFIG["websocket"]["host"] = "127.0.0.1"
CONFIG["websocket"]["port"] = 3287

# DataBase Access
CONFIG["database"]["is_mysql"] = False


