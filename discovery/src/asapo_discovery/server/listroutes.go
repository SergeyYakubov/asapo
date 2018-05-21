package server

import (
	"asapo_discovery/utils"
)

var listRoutes = utils.Routes{
	utils.Route{
		"GetReceivers",
		"Get",
		"/receivers",
		routeGetReceivers,
	},
}
