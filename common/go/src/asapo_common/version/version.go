package version

import (
	"flag"
	"fmt"
	"io"
)

var version string
var consumerProtocolVersion string
var producerProtocolVersion string

func GetProducerProtocolVersion() string {
	return producerProtocolVersion
}

func GetConsumerProtocolVersion() string {
	return consumerProtocolVersion
}


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
