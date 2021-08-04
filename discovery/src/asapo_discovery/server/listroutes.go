package server

import (
	"asapo_common/utils"
	"asapo_discovery/common"
)

var listRoutes = utils.Routes{
	utils.Route{
		"GetMonitoringServers",
		"Get",
		"/" + common.NameMonitoringServer,
		routeGetMonitoringServers,
	},
	utils.Route{
		"GetReceivers",
		"Get",
		"/{apiver}/" + common.NameReceiverService,
		routeGetReceivers,
	},
	utils.Route{
		"GetBroker",
		"Get",
		"/{apiver}/"+common.NameBrokerService,
		routeGetBroker,
	},
	utils.Route{
		"GetMongo",
		"Get",
		"/" + common.NameMongoService,
		routeGetMongo,
	},
	utils.Route{
		"GetVersion",
		"Get",
		"/{apiver}/version",
		routeGetVersion,
	},
	utils.Route{
		"GetFTS",
		"Get",
		"/{apiver}/" + common.NameFtsService,
		routeGetFileTransferService,
	},
	utils.Route{
		"Health",
		"Get",
		"/health",
		routeGetHealth,
	},

}
