//+build !test

package main

import (
	"hidra2_broker/database"
	"hidra2_broker/server"
	"log"
	"os"
)

func NewDefaultDatabase() database.Agent {
	return new(database.Mongodb)
}

func PrintUsage() {
	log.Fatal("Usage: " + os.Args[0] + " <config file>")
}

func main() {
	if len(os.Args) != 2 {
		PrintUsage()
	}

	fname := os.Args[1]
	err := server.ReadConfig(fname)
	if err != nil {
		log.Fatal(err.Error())
	}

	err = server.InitDB(NewDefaultDatabase())
	if err != nil {
		log.Fatal(err.Error())
	}
	defer server.CleanupDB()
	server.Start()
}
