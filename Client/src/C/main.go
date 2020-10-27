package main

import (
	"C/core"
	"time"
)
//export export
func Export(){

}
func main() {
	for  {
		c := core.NewClient()
		c.Pool()
		time.Sleep(10)
	}

}
