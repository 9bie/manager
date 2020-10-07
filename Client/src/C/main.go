package main

import (
	"C/core"
)

func main() {
	c := core.NewClient()
	c.Pool()
}
