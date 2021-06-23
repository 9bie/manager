package shell

import (
	"io"
	"io/ioutil"
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
	Param   string `json:"param"`
}
type Remark struct {
	Remark string `json:"remark"`
}



func (s Shell) ExecuteCmd() string {

	cmd := exec.Command(s.Command, s.Param)

	stdout, _ := cmd.StdoutPipe()

	defer stdout.Close() // 保证关闭输出流

	if err := cmd.Start(); err != nil { // 运行命令
		//fmt.Println(err.Error())
		return err.Error()

	}

	if opBytes, err := ioutil.ReadAll(stdout); err != nil { // 读取输出结果

		return err.Error()

	} else {
		//fmt.Println("execute", string(opBytes))
		return string(opBytes)
	}
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
