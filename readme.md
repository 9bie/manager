# Manage

BIE的HMP远控。

## Server-控制端

由 Golang 编写，使用了Echo模块和Cstruct模块。基于WS控制的前端。

## Client-被控端

由C编写，代码极其HMP，请谨慎阅读。

目前功能：

    - 命令终端
    - 文件下载
    - 自动增大体积
    - 自删除

## How 2 Run
直接运行S.exe即可，或者使用下面参数

S.exe 参数如下：

    - password  网页端进入密码，默认为bie
    - web_port  网页端监听端口
    - backend_port 后端监听端口

之后请按照网页指引   