package request_handler

import (
	"github.com/stretchr/testify/assert"
	"testing"
    "asapo_discovery/utils"
)


var uris = []string{"ip1","ip2"}
const max_conn = 1

var static_settings utils.Settings= utils.Settings{Receiver:utils.ReceiverInfo{MaxConnections:max_conn,ForceEndpoints:uris},Broker:utils.BrokerInfo{
	ForceEndpoint:"ip_broker"}}



var rh StaticRequestHandler;

func TestStaticHandlerInitOK(t *testing.T) {
	err := rh.Init(static_settings)
	assert.Nil(t, err)
}

func TestStaticHandlerGetReceviersOK(t *testing.T) {
	rh.Init(static_settings)
	res,err := rh.GetReceivers()
	assert.Equal(t,string(res), "{\"MaxConnections\":1,\"Uris\":[\"ip1\",\"ip2\"]}")
	assert.Nil(t, err)
}

func TestStaticHandlerGetBrokerOK(t *testing.T) {
	rh.Init(static_settings)
	res,err := rh.GetBroker()
	assert.Equal(t,string(res), "ip_broker")
	assert.Nil(t, err)
}
