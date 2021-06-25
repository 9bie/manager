package main

import (
	"github.com/9bie/manager/Client/core"
	"time"
)

//export export
func Export() {
	for {
		time.Sleep(10)
	}
}
func main() {
	for {
		c := core.NewClient()
		c.Pool()
		time.Sleep(10)
	}

}
