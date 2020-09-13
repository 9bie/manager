package C

import (
	"runtime"
	"os"
)
func setOSConfigure(config string)bool {
	err :=  os.Setenv("SYSOPBIERM",config)
	if err != nil {
		return false
	}else {
		return true
	}
}

func getOSConfigure()string{
	return os.Getenv("SYSOPBIERM")
}
// 0-linux 1-windwos 2=unkonw
func getOSVersion() int{

	sysType := runtime.GOOS

	if sysType == "linux" {
		return 0
	}

	if sysType == "windows" {
		return 1
	}
	return 2
}
func main(){

}