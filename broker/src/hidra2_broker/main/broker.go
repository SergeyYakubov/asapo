//+build !test

package main

import (
	"flag"
	"hidra2_broker/database"
	log "hidra2_broker/logger"
	"hidra2_broker/server"
	"os"
)

func NewDefaultDatabase() database.Agent {
	return new(database.Mongodb)
}

func PrintUsage() {
	log.Fatal("Usage: " + os.Args[0] + " -config <config file> [-logging debug|info|warning|error|fatal]")
}

func main() {
	var fname = flag.String("config", "", "config file path")
	var log_level = flag.String("logging", "info", "logging level")

	flag.Parse()
	if *fname == "" {
		PrintUsage()
	}
	log.SetLevel(*log_level)

	err := server.ReadConfig(*fname)
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
