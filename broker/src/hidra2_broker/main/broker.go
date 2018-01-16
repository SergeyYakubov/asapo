//+build !test

package main

import (
	"hidra2_broker/database"
	"hidra2_broker/server"
)

func NewDefaultDatabase() database.Agent {
	return new(database.Mongodb)
}

// global variable since we only have one instance
var srv server.Server

func main() {
	srv.InitDB(NewDefaultDatabase())
	defer srv.CleanupDB()
	srv.Start()
}
