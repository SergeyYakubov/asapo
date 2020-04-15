package server

import (
	"asapo_common/utils"
)

var listRoutes = utils.Routes{
	utils.Route{
		"Authorize",
		"POST",
		"/authorize",
		routeAuthorize,
	},
	utils.Route{
		"HealthCheck",
		"Get",
		"/health-check",
		routeGetHealth,
	},
	utils.Route{
		"Folder Token",
		"POST",
		"/folder",
		routeFolderToken,
	},
}
