package server

import (
	"asapo_common/utils"
)

var listRoutes = utils.Routes{
	utils.Route{
		"GetNext",
		"Get",
		"/database/{dbname}/next",
		routeGetNext,
	},
	utils.Route{
		"GetID",
		"Get",
		"/database/{dbname}/{id}",
		routeGetByID,
	},

	utils.Route{
		"Health",
		"Get",
		"/health",
		routeGetHealth,
	},
}
