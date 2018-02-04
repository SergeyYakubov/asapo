//+build !test

package main

import (
	"hidra2_broker/database"
	"hidra2_broker/server"
	"log"
)

func NewDefaultDatabase() database.Agent {
	return new(database.Mongodb)
}

func main() {
	err:=server.InitDB(NewDefaultDatabase())
	if err!= nil {
	    log.Fatal(err.Error())
	}
	defer server.CleanupDB()
	server.Start()
}
