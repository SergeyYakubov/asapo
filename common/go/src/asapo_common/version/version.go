package version

import (
	"flag"
	"fmt"
	"io"
)

var version string

func GetVersion() string {
    return version
}

func ShowVersion(w io.Writer, name string) bool {

	flVersion := flag.Bool("v", false, "Print version information")
	flag.Parse()
	if *flVersion {
		fmt.Fprintf(w, "%s, version %s\n", name, version)
		return true
	}
	return false
}
