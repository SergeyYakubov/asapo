package request_handler

import (
	"asapo_discovery/common"
	"github.com/stretchr/testify/assert"
	"testing"
)


var uris = []string{"ip1","ip2"}
const max_conn = 1

var static_settings common.Settings = common.Settings{
	Receiver: common.ReceiverInfo {
		MaxConnections: max_conn,
		StaticEndpoints:uris,
	},
	Broker: common.BrokerInfo { StaticEndpoint:"ip_broker" },
	Monitoring: common.MonitoringInfo { StaticEndpoint:"ip_monitoring" },
	Mongo: common.MongoInfo {StaticEndpoint:"ip_mongo"},
	FileTransferService: common.FtsInfo {StaticEndpoint:"ip_fts"},
}



var rh StaticRequestHandler;

func TestStaticHandlerInitOK(t *testing.T) {
	err := rh.Init(static_settings)
	assert.Nil(t, err)
}

func TestStaticHandlerGetReceviersOK(t *testing.T) {
	rh.Init(static_settings)
	res,err := rh.GetReceivers(false)
	assert.Equal(t,string(res), "{\"MaxConnections\":1,\"Uris\":[\"ip1\",\"ip2\"]}")
	assert.Nil(t, err)
}

func TestStaticHandlerGetMonitoringServersOK(t *testing.T) {
	rh.Init(static_settings)
	res,err := rh.GetSingleService(common.NameMonitoringServer)
	assert.Equal(t,string(res), "ip_monitoring")
	assert.Nil(t, err)
}

func TestStaticHandlerGetBrokerOK(t *testing.T) {
	rh.Init(static_settings)
	res,err := rh.GetSingleService(common.NameBrokerService)
	assert.Equal(t,string(res), "ip_broker")
	assert.Nil(t, err)
}

func TestStaticHandlerGetMongoOK(t *testing.T) {
	rh.Init(static_settings)
	res,err := rh.GetSingleService(common.NameMongoService)
	assert.Equal(t,string(res), "ip_mongo")
	assert.Nil(t, err)
}


func TestStaticHandlerGetFtsOK(t *testing.T) {
	rh.Init(static_settings)
	res,err := rh.GetSingleService(common.NameFtsService)
	assert.Equal(t,string(res), "ip_fts")
	assert.Nil(t, err)
}
