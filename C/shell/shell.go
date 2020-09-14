package shell

import (
	"io/ioutil"
	"log"
	"os/exec"
)

func ExecuteCmd(command string, arg string) {
	cmd := exec.Command(command, arg)

	if stdout, err := cmd.StdoutPipe(); err != nil { //获取输出对象，可以从该对象中读取输出结果

		return err

	}

	defer stdout.Close() // 保证关闭输出流

	if err := cmd.Start(); err != nil { // 运行命令

		log.Fatal(err)

	}

	if opBytes, err := ioutil.ReadAll(stdout); err != nil { // 读取输出结果

		log.Fatal(err)

	} else {

		log.Println(string(opBytes))

	}
}
func Download(url string, path string) {

}
