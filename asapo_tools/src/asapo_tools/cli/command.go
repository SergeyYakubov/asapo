package cli

import (
	"flag"
	"fmt"
)

// A command consists of a command name and arguments, passed to this command (all after asapo name ...)
type command struct {
	name string
	args []string
}

// description prints description line and returns true if first command argument is "description".
func (cmd *command) description(d string) bool {
	if len(cmd.args) == 1 && cmd.args[0] == "description" {
		fmt.Fprintf(outBuf, "   %-10s %s\n", cmd.name, d)
		return true
	}
	return false
}

// createDefaultFlagset creates new flagset and adds default help behaviour.
func (cmd *command) createDefaultFlagset(description, args string) *flag.FlagSet {

	flags := flag.NewFlagSet(cmd.name, flag.ExitOnError)
	flags.BoolVar(&flHelp, "help", false, "Print usage")
	flags.Usage = func() {
		fmt.Fprintf(outBuf, "Usage:\t\nasapo %s "+args, cmd.name)
		fmt.Fprintf(outBuf, "\n\n%s\n", description)
		flags.PrintDefaults()
	}

	return flags
}
