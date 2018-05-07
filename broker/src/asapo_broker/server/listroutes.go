package server

import (
	"asapo_broker/utils"
)

var listRoutes = utils.Routes{
	utils.Route{
		"GetNext",
		"Get",
		"/database/{dbname}/next",
		routeGetNext,
	},
}
