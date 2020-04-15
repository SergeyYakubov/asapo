package server

import (
	"asapo_common/utils"
	"asapo_discovery/common"
)

var listRoutes = utils.Routes{
	utils.Route{
		"GetReceivers",
		"Get",
		"/" + common.NameReceiverService,
		routeGetReceivers,
	},
	utils.Route{
		"GetBroker",
		"Get",
		"/asapo-broker",
		routeGetBroker,
	},
	utils.Route{
		"GetMongo",
		"Get",
		"/" + common.NameMongoService,
		routeGetMongo,
	},
	utils.Route{
		"GetFTS",
		"Get",
		"/" + common.NameFtsService,
		routeGetFileTransferService,
	},
}
