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
		"Authorize",
		"POST",
		"/introspect",
		routeIntrospect,
	},
	utils.Route{
		"Authorize",
		"POST",
		"/admin/revoke",
		routeRevoke,
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
		"/{apiver}/folder",
		routeFolderToken,
	},
	utils.Route{
		"User Token",
		"POST",
		"/admin/issue",
		routeAuthorisedTokenIssue,
	},
}
