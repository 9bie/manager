CONFIG = {"web": {}, "websocket": {}, "database": {}, "base": {}}

# Base Access
CONFIG["base"]["username"] = "admin"
CONFIG["base"]["password"] = "12345"
CONFIG["base"]["default_sleep"] = 30
CONFIG["base"]["log"] = "running.log"
CONFIG["base"]["cdn"] = False # 如果有使用反代和cdn请使用这个
CONFIG["base"]["source_ip_tag"] = "X-Real-Ip" # 务必让反代或者的cdn请求头中的这个值包含源用户的真实IP
# Web Control Access
CONFIG["web"]["debug"] = True
CONFIG["web"]["host"] = "0.0.0.0"
CONFIG["web"]["port"] = "5000"
CONFIG["web"]["backend"] = "/backend/"
CONFIG["web"]["web"] = "/web/"


# Web Sockets Access
CONFIG["websocket"]["host"] = "0.0.0.0"
CONFIG["websocket"]["port"] = 3287

# DataBase Access
CONFIG["database"]["is_mysql"] = False


