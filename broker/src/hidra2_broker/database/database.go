package database

import "fmt"

type Agent interface {
	Connect(string) error
	Close()
}

func Test_Hidra2() {
	fmt.Println("aaa")
}
