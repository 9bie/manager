
# 发送流程
### Client
每个Client都会有一个UUID，在存货期间生成，之后开始30秒循环发送配置信息

### Server
用作转发与广播作用


### 1. Client->Server
构建`Work`结构体，详情可以看`core/core.go`第24行，关于这个结构体作用可以看下面，如果是第一次执行，没有需要返回的值，所以Result字段为空。如果下面那个阶段已经完成，`Result`已经有消息，则返回数据

这次发包的作用是，返回上一直执行完命令的回显，发送自身系统信息
（Info字段）让服务器上线，同时请求服务器查询是否有需要的消息。循环这次
### 2. Server->Client
服务器检查事件队列，判断是否有消息，有消息则判断消息队列中的发送对象是指定uuid还是广播，如果是指定uuid，就检查上线客户中是否有该uuid，有该uuid则
发送消息，如果是广播，则对在线客户所有发送广播，其中结构体为`[Action]`，可以查看`core/core.go`第30行，记住，发送的是一个队列而不是一个单独的结构体
Client收到了`Action`结构体后，根据其`Do`所对应的值，对`Data`进行二次解析来作为功能的参数，至于`Do`字段对应的字段，可以查阅`core/core.go`第80行的switch


例如值为cmd，则会使用`shell.Shell`的内容，来解析data，之后用里面作为参数进行执行命令，返回放置`Result`结构体中


## 下发命令（开发重点）
前端直接向websocket端发送json结构体即可，例如要求xxx-xxxx-xxxx执行cmd命令
```json
{
                        "uuid": "xxx-xxxx-xxxx",
                        "do":{
                            "do":"cmd",
                            "data":"ewogICAgICAgICAgICAgICAgICAgICJjb21tYW5kIjoiY21kLmV4ZSIsCiAgICAgICAgICAgICAgICAgICAgInBhcmFtIjoiL2Mgd2hvYW1pIgp9"
                        }
}
```
do字段就对应的下面的`Result`字段
do中的data字段为，base64编码后的Shell结构体，详细结构体对应可以查阅`core/core.go`第80行的switch，当前用的是cmd所以对应的就是shell字段

这个event就被Server塞入队列中，做为
# 结构体作用

## Work
通讯结构体，位于`core/core.go`
```golang
type Work struct {
    Uuid       string            `json:"uuid"` // 唯一指定标识符
    Result     []Result          `json:"result"` // 作为执行完指令返回的数据的列表
    Info       utils.Information `json:"info"` // 包含基础系统信息
    NextSecond int               `json:"next_second"` // 下一次请求时间
}
```
## Info
详细系统信息，位于`utils/utils.go`
```
type Information struct {
    IP      string `json:"ip"`       // 外网ip
    User    string `json:"user"`     // 当前用户
    Remarks string `json:"remarks"`  // 备注
    IIP     string `json:"iip"`      // 内网ip
    System  string `json:"system"`   // 操作系统
}
```

## Action
下发命令用的结构体，位于`core/core.go`
```golang
type Action struct {
    Do   string `json:"do"` // 用于标识指令，比如cmd,download等
    Data string `json:"data"` // base64后的具体的参数，实际上依旧是json字符串，用于二级解析
}
```
## Result
返回结果，位于`core/core.go`
```golang
type Result struct {
    Action string `json:"action"` // 对应Action的do
    Data   string `json:"data"`   // 返回值，用于呈现给用户的
}
```
## Shell
执行命令用的结构体，位于`shell/shell.go`
```
type Shell struct {
    Command string `json:"command"` // 程序位置
    Param   string `json:"param"`   // 程序参数
}
```



# 完整流程
模拟一个完整上线到执行完毕命令
## Client第一次循环（上线，沉睡）
客户端发送`Work`->
```json
{
    "uuid": "xxx-xxx-xxx-xxx",
    "result": [],
    "info": {
        "ip": "1.1.1.1",
        "user": "administrator",
        "remarks": "default",
        "iip": "172.16.1.1",
        "system": "windows"
    },
    "next_second": 30
}
```
客户端上线成功,服务器检查队列，没有任何任务，返回空的`[Action]`

<- 服务器
```json
[{
    "do": "",
    "data": ""
}]
```

客户响应`[Action]`内容，发现没东西响应，发送结果包,并设置sleep为30

沉睡30秒，之后到循环上面步骤
## 命令下发（前端重点）
前端对这个uuid下发了一条消息

前端 ->
```json
{
                     "uuid": "xxx-xxx-xxx-xxx",
                     "do":{
                         "do":"cmd",
                         "data":"ewogICAgICAgICAgICAgICAgICAgICJjb21tYW5kIjoiY21kLmV4ZSIsCiAgICAgICAgICAgICAgICAgICAgInBhcmFtIjoiL2Mgd2hvYW1pIgp9"
                     }
}
```

服务器收到请求，并且把这个消息加入了队列
## Client第二次循环
30秒一过，客户继续发送`Work`

客户端发送`Work`->
```json
{
    "uuid": "xxx-xxx-xxx-xxx",
    "result": [],
    "info": {
        "ip": "1.1.1.1",
        "user": "administrator",
        "remarks": "default",
        "iip": "172.16.1.1",
        "system": "windows"
    },
    "next_second": 30
}
```

此时服务器队检查队列，发现了一条uuid对应的事件，构造了`[Action]`包

<- 服务器`[Action]`
```json
[{
    "do": "cmd",
    "data": "ewogICAgICAgICAgICAgICAgICAgICJjb21tYW5kIjoiY21kLmV4ZSIsCiAgICAgICAgICAgICAgICAgICAgInBhcmFtIjoiL2Mgd2hvYW1pIgp9"
}]
```

客户端收到了`[Action]`，解析并根据对应消息解析了内容，构造返回数据包

继续沉睡30秒，循环
# Client第三次循环
客户响应`Work` ->
```json
{
    "uuid": "xxx-xxx-xxx-xxx",
    "result": [{
                "action": "cmd",
                "data": "我是返回的结果"
               }],
    "info": {
        "ip": "1.1.1.1",
        "user": "administrator",
        "remarks": "default",
        "iip": "172.16.1.1",
        "system": "windows"
    },
    "next_second": 30
}
```

服务器收到`result`，把内容广播给前端，前端解析给用户，然后任务队列中已经无任务。继续返回


<- 服务器
```json
[{
    "do": "",
    "data": ""
}]
```

客户此步骤无限循环直到前端再次塞入消息