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
	log.Fatal(server.InitDB(NewDefaultDatabase()))
	defer server.CleanupDB()
	server.Start()
}
