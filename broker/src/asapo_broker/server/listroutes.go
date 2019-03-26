package server

import (
	"asapo_common/utils"
)

var listRoutes = utils.Routes{
	utils.Route{
		"GetNext",
		"Get",
		"/database/{dbname}/{groupid}/next",
		routeGetNext,
	},
	utils.Route{
		"GetLast",
		"Get",
		"/database/{dbname}/{groupid}/last",
		routeGetLast,
	},
	utils.Route{
		"GetID",
		"Get",
		"/database/{dbname}/{id}",
		routeGetByID,
	},
	utils.Route{
		"GetID",
		"Post",
		"/creategroup",
		routeCreateGroupID,
	},

	utils.Route{
		"Health",
		"Get",
		"/health",
		routeGetHealth,
	},
}
