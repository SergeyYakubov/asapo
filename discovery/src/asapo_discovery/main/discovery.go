//+build !test

package main

import (
	"flag"
	log "asapo_common/logger"
	"asapo_discovery/server"
	"os"
	"asapo_discovery/request_handler"
)

func NewDefaultHandler() request_handler.Agent {
	switch server.GetHandlerMode() {
	case "static":
		return new(request_handler.StaticRequestHandler)
	case "consul":
		return new(request_handler.ConsulRequestHandler)
	default:
		log.Fatal("wrong handler")
		return nil
	}
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

	err = server.SetHandler(NewDefaultHandler())
	if err != nil {
		log.Fatal(err.Error())
	}
	server.Start()
}
