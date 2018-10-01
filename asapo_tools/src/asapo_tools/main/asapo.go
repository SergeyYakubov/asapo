package main

import (
	"flag"
	"fmt"
	"os"
	"asapo_common/version"
	"asapo_tools/cli"
)

var (
	flHelp = flag.Bool("help", false, "Print usage")
)

func main() {

	if ret := version.ShowVersion(os.Stdout, "ASAPO"); ret {
		return
	}

	flag.Parse()

	if *flHelp || flag.NArg() == 0 {
		flag.Usage()
		cli.PrintAllCommands()
		return
	}

		if err := cli.DoCommand(flag.Arg(0), flag.Args()[1:]); err != nil {
			fmt.Fprintln(os.Stderr, err)
			os.Exit(1)
		}
}
