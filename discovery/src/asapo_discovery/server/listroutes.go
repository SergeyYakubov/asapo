package server

import (
	"asapo_common/utils"
)

var listRoutes = utils.Routes{
	utils.Route{
		"GetReceivers",
		"Get",
		"/receivers",
		routeGetReceivers,
	},
	utils.Route{
		"GetBroker",
		"Get",
		"/broker",
		routeGetBroker,
	},
	utils.Route{
		"GetMongo",
		"Get",
		"/mongo",
		routeGetMongo,
	},
	utils.Route{
		"GetFTS",
		"Get",
		"/fts",
		routeGetFileTransferService,
	},

}
