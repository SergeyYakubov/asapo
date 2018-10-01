//+build !test

package main

import (
	"asapo_broker/database"
	"asapo_broker/server"
	log "asapo_common/logger"
	"asapo_common/version"
	"flag"
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

	if ret := version.ShowVersion(os.Stdout, "ASAPO Broker"); ret {
		return
	}

	log.SetSoucre("broker")
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
