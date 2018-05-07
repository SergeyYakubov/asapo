//+build !test

package main

import (
	"flag"
	"asapo_broker/database"
	log "asapo_broker/logger"
	"asapo_broker/server"
	"os"
)

func NewDefaultDatabase() database.Agent {
	return new(database.Mongodb)
}

func PrintUsage() {
	log.Fatal("Usage: " + os.Args[0] + " -config <config file>")
}

func main() {
	var fname = flag.String("config", "", "config file path")

	flag.Parse()
	if *fname == "" {
		PrintUsage()
	}

	logLevel, err := server.ReadConfig(*fname)
	if err != nil {
		log.Fatal(err.Error())
	}

	log.SetLevel(logLevel)

	err = server.InitDB(NewDefaultDatabase())
	if err != nil {
		log.Fatal(err.Error())
	}
	defer server.CleanupDB()
	server.Start()
}
