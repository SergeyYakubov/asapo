package common

import (
	"strconv"
	"strings"
)

const  (
	NameMongoService = "asapo-mongodb"
	NameFtsService = "asapo-file-transfer"
	NameBrokerService = "asapo-broker"
	NameReceiverService = "asapo-receiver"
)

const ApiVersion = "v0.1"

func VersionToNumber(ver string) int {
	ver = strings.TrimPrefix(ver,"v")
	floatNum, err := strconv.ParseFloat(ver, 64)
	if err!=nil {
		return 0
	}
	return int(floatNum*1000)
}
