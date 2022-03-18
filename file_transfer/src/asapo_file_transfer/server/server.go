package server

import (
"asapo_common/utils"
)

type serverSettings struct {
	Port                    int
	LogLevel                string
	SecretFile              string
	key 					string
}

var settings serverSettings
var authJWT utils.Auth

