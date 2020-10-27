# Manage
一款HMP的b/s全平台通用的客户管理工具

![不要吐槽前端](doc/img/index.png)

# 开发文档
核心是前端构建json包作为结构体传入后端，后端根据json包中指定的UUID决定是转发数据包还是广播数据包

之后Client段接受到json解析json类型字段做出相应动作并返回数据给Server，最后Server再把数据呈现给前端

详细实现可以看:[详细实现以及完整流程](doc/archive.md)

# 关于Release

其自行编译Client使用交叉编译放置于`Server/bin`下面

# 如何编译
## Server
自带env虚拟环境，跑就完事了
## Client
请将此目录下的Client添加到项目GOPATH

或者是使用Client目录下的install.bat编译，之后检查client目录下的bin文件夹！




