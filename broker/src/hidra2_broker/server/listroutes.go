package server

import (
	"hidra2_broker/utils"
)

var listRoutes = utils.Routes{
	utils.Route{
		"GetNext",
		"Get",
		"/next/",
		routeGetNext,
	},
}
