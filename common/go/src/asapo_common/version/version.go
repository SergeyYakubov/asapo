package version

import (
	"flag"
	"fmt"
	"io"
)

var version string

var consumerProtocolVersion string
var producerProtocolVersion string
var discoveryApiVersion string
var authorizerApiVersion string
var ftsApiVersion string
var brokerApiVersion string

func GetDiscoveryApiVersion() string {
	return discoveryApiVersion
}
func GetAuthorizerApiVersion() string {
	return authorizerApiVersion
}
func GetFtsApiVersion() string {
	return ftsApiVersion
}
func GetBrokerApiVersion() string {
	return brokerApiVersion
}

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
