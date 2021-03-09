//+build !test

package main

import (
	"asapo_authorizer/cli"
	"asapo_authorizer/server"
	log "asapo_common/logger"
	"asapo_common/version"
	"flag"
	"os"
)

var (
	flHelp = flag.Bool("help", false, "Print usage")
)

func main() {
	cli.ProgramName = "asapo-authorizer"

	var fname = flag.String("config", "", "config file path (mandatory)")

	if ret := version.ShowVersion(os.Stdout, "ASAPO Authorizer"); ret {
		return
	}

	log.SetSoucre("authorizer")

	flag.Parse()
	if *flHelp {
		flag.Usage()
		cli.PrintAllCommands()
		return
	}

	if *fname=="" {
		log.Fatal("config file path is missed")

	}
	logLevel, err := server.ReadConfig(*fname)
	if err != nil {
		log.Fatal(err.Error())
	}

	log.SetLevel(logLevel)

	if len(flag.Args()) == 0 {
		server.Start()
	}

	if err := cli.DoCommand(flag.Arg(0), flag.Args()[1:]); err != nil {
		log.Fatal(err.Error())
	}

}
