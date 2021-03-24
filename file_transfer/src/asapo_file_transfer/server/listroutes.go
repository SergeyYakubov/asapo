package server

import (
	"asapo_common/utils"
)

var listRoutes = utils.Routes{
	utils.Route{
		"Transfer File",
		"POST",
		"/{apiver}/transfer",
		routeFileTransfer,
	},
	utils.Route{
		"HealthCheck",
		"Get",
		"/health-check",
		routeGetHealth,
	},
}
