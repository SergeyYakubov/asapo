package version

import (
	"flag"
	"fmt"
	"io"
	"os"
)

var version

func ShowVersion(w io.Writer, name string) bool {
	flags := flag.NewFlagSet("version", flag.ExitOnError)
	flag.Bool("version", false, "Print version information") // to have it in main help
	flVersion := flags.Bool("version", false, "Print version information")
	flags.Bool("help", false, "Print usage") // define help flag but ignore it
	flags.Parse(os.Args[1:])
	if *flVersion {
		fmt.Fprintf(w, "%s version %s\n", name, version)
		return true
	}
	return false
}
