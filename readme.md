# Manage
一款HMP的b/s全平台通用的客户管理工具

![不要吐槽前端](doc/img/index.png)

## todo

- 后台密码验证
- 客户名单分页
- 日志分页
- 还有啥暂时想不到了
- 多用户模式


## 注意

对客户端仅有略微过滤，有潜在的XSS和SSTI风险。

## 关于反代

修改`app/config/config.py`下的`CONFIG["base"]["cdn"] = False`改为True

之后把`CONFIG["base"]["source_ip_tag"] = "X-Real-Ip"`修改为反代返回给你正确IP的tag



# 关于Release

其自行编译Client使用交叉编译放置于`Server/bin`下面

# 如何编译
## Server
需要以下依赖：

	- quert

## Client

 - 把此项目clone到你的GOPATH
 - mv $GOPATH/9bie/manager/Client
 - go build

你编译的config.go中的http地址必须是你py项目config.py里的`CONFIG["web"]["control"] `地址


