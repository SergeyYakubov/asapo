//+build !test

package main

import (
	log "asapo_common/logger"
	"asapo_authorizer/server"
	"asapo_common/version"
	"flag"
	"os"
)

func PrintUsage() {
	log.Fatal("Usage: " + os.Args[0] + " -config <config file>")
}

func main() {
	var fname = flag.String("config", "", "config file path")

	if ret := version.ShowVersion(os.Stdout, "ASAPO Authorizer"); ret {
		return
	}

	log.SetSoucre("authorizer")

	flag.Parse()
	if *fname == "" {
		PrintUsage()
	}

	logLevel, err := server.ReadConfig(*fname)
	if err != nil {
		log.Fatal(err.Error())
	}

	log.SetLevel(logLevel)

	server.Start()
}
