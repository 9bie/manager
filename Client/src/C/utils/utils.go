package utils

import (
	"C/config"
	"crypto/rc4"
	"encoding/base64"
	"fmt"
	"math/rand"
	"net"
	"os"
	"os/user"
	"runtime"
	"strconv"
	"time"
)

type Information struct {
	User    string `json:"user"`
	Remarks string `json:"remarks"`
	IIP     string `json:"iip"`
	System  string `json:"system"`
}

func GetInformation() Information {
	return Information{
		User:    GetUser(),
		Remarks: GetRemarks(),
		IIP:     GetIPAddress(),
		System:  runtime.GOOS,
	}
}

func RandStringRunes(n int) string {
	var letterRunes = []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
	b := make([]rune, n)
	for i := range b {
		b[i] = letterRunes[rand.Intn(len(letterRunes))]
	}
	return string(b)
}

func ImmediateRC4(input []byte) []byte {
	t := time.Now().Unix()
	t2 := int(t) / 100
	s := strconv.Itoa(t2)
	var key []byte = []byte(s)
	rc4obj1, _ := rc4.NewCipher(key)
	rc4str1 := []byte(input)
	plaintext := make([]byte, len(rc4str1))
	rc4obj1.XORKeyStream(plaintext, rc4str1)
	return plaintext

}
func GetUser() string {
	if u, err := user.Current(); err == nil {
		return u.Username
	}
	return "unknown"
}
func EasyCrypto(input string) string {
	bytes := []byte(input)
	for i := 0; i < len(bytes); i++ {
		bytes[i] = bytes[i] + 1
	}
	encoded := base64.StdEncoding.EncodeToString(bytes)
	return encoded
}
func EasyDeCrypto(input string) string {
	decoded, err := base64.StdEncoding.DecodeString(input)
	if err != nil {
		return "err"
	}
	for i := 0; i < len(decoded); i++ {
		decoded[i] = decoded[i] - 1
	}
	return string(decoded)

}

func SetRemarks(remarks string) string {
	err := os.Setenv("SysRemarks", EasyCrypto(remarks))
	if err != nil {
		return err.Error()
	} else {
		return "Change Remark Successful"
	}
}

func GetRemarks() string {
	if os.Getenv("SysRemarks") == "" {
		return config.Remarks
	}
	return EasyDeCrypto(os.Getenv("SysRemarks"))
}

// 0-linux 1-windwos 2=unkonw
func GetOSVersion() int {

	sysType := runtime.GOOS

	if sysType == "linux" {
		return 0
	}

	if sysType == "windows" {
		return 1
	}
	return 2
}
func GetIPAddress() string {
	var IP string
	adders, err := net.InterfaceAddrs()
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	for _, address := range adders {
		// 检查ip地址判断是否回环地址
		if inet, ok := address.(*net.IPNet); ok && !inet.IP.IsLoopback() {
			if inet.IP.To4() != nil {
				return inet.IP.String()
				// IP += inet.IP.String() + ","
			}
		}
	}
	return IP
}
