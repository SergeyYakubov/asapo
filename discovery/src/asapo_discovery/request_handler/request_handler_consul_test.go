package request_handler

import (
	"github.com/stretchr/testify/suite"
	"testing"
	"github.com/hashicorp/consul/api"
	"strconv"
	"asapo_discovery/common"
)

type ConsulHandlerTestSuite struct {
	suite.Suite
	client  *api.Client
	handler ConsulRequestHandler
}

func TestConsulHandlerTestSuite(t *testing.T) {
	suite.Run(t, new(ConsulHandlerTestSuite))
}

var consul_settings common.Settings

func (suite *ConsulHandlerTestSuite) registerAgents(name string) {
	for i := 1234; i < 1236; i++ {
		reg := &api.AgentServiceRegistration{
			ID:   name + strconv.Itoa(i),
			Name: name,
			Port: i,
			Check: &api.AgentServiceCheck{
				Interval: "10m",
				Status:   "passing",
				HTTP:     "http://localhost:5000/health",
			},
		}
		err := suite.client.Agent().ServiceRegister(reg)
		if err != nil {
			panic(err)
		}
	}

}

func (suite *ConsulHandlerTestSuite) SetupTest() {
	var err error
	consul_settings = common.Settings{Receiver: common.ReceiverInfo{MaxConnections: 10, StaticEndpoints: []string{}}}

	suite.client, err = api.NewClient(api.DefaultConfig())
	if err != nil {
		panic(err)
	}
	common.NameReceiverService = "asapo-receiver-test"
	common.NameBrokerService = "asapo-broker-test"
	suite.registerAgents("asapo-receiver-test")
	suite.registerAgents("asapo-broker-test")
}

func (suite *ConsulHandlerTestSuite) TearDownTest() {
	suite.client.Agent().ServiceDeregister("asapo-receiver-test1234")
	suite.client.Agent().ServiceDeregister("asapo-receiver-test1235")
	suite.client.Agent().ServiceDeregister("asapo-broker-test1234")
	suite.client.Agent().ServiceDeregister("asapo-broker-test1235")
}

func (suite *ConsulHandlerTestSuite) TestInitDefaultUri() {
	err := suite.handler.Init(consul_settings)
	suite.NoError(err, "empty list")
}

func (suite *ConsulHandlerTestSuite) TestInitWrongUri() {
	consul_settings.ConsulEndpoints = []string{"blabla"}
	err := suite.handler.Init(consul_settings)
	suite.Error(err, "wrong consul uri")
	suite.Nil(suite.handler.client, "client nli after error")

}

func (suite *ConsulHandlerTestSuite) TestInitOkUriFirst() {
	consul_settings.ConsulEndpoints = []string{"http://127.0.0.1:8500"}

	err := suite.handler.Init(consul_settings)
	suite.NoError(err, "")
}

func (suite *ConsulHandlerTestSuite) TestInitOkUriNotFirst() {
	consul_settings.ConsulEndpoints = []string{"blabla", "http://127.0.0.1:8500"}

	err := suite.handler.Init(consul_settings)
	suite.NoError(err, "")
}

func (suite *ConsulHandlerTestSuite) TestGetReceivers() {
	suite.handler.Init(consul_settings)
	res, err := suite.handler.GetReceivers(false)
	suite.NoError(err, "")
	suite.Equal("{\"MaxConnections\":10,\"Uris\":[\"127.0.0.1:1234\",\"127.0.0.1:1235\"]}", string(res), "uris")
}

func (suite *ConsulHandlerTestSuite) TestGetReceiversWithIB() {
	consul_settings.Receiver.UseIBAddress = true
	suite.handler.Init(consul_settings)
	res, err := suite.handler.GetReceivers(true)
	suite.NoError(err, "")
	suite.Equal("{\"MaxConnections\":10,\"Uris\":[\"10.10.0.1:1234\",\"10.10.0.1:1235\"]}", string(res), "uris")
}


func (suite *ConsulHandlerTestSuite) TestGetReceiversStatic() {
	consul_settings.Receiver.StaticEndpoints= []string{"127.0.0.1:0000"}
	suite.handler.Init(consul_settings)
	res, err := suite.handler.GetReceivers(false)
	suite.NoError(err, "")
	suite.Equal("{\"MaxConnections\":10,\"Uris\":[\"127.0.0.1:0000\"]}", string(res), "uris")
}

func (suite *ConsulHandlerTestSuite) TestGetReceiversWhenNotConnected() {
	consul_settings.ConsulEndpoints = []string{"blabla"}
	suite.handler.Init(consul_settings)
	_, err := suite.handler.GetReceivers(false)
	suite.Error(err, "")
}

func (suite *ConsulHandlerTestSuite) TestGetBrokerWhenNotConnected() {
	consul_settings.ConsulEndpoints = []string{"blabla"}
	suite.handler.Init(consul_settings)
	_, err := suite.handler.GetSingleService(common.NameBrokerService)
	suite.Error(err, "")
}

func (suite *ConsulHandlerTestSuite) TestGetBrokerRoundRobin() {
	suite.handler.Init(consul_settings)
	res, err := suite.handler.GetSingleService(common.NameBrokerService)
	suite.NoError(err, "")
	suite.Equal("127.0.0.1:1234", string(res), "uris")

	res, err = suite.handler.GetSingleService(common.NameBrokerService)
	suite.NoError(err, "")
	suite.Equal("127.0.0.1:1235", string(res), "uris")

	res, err = suite.handler.GetSingleService(common.NameBrokerService)
	suite.NoError(err, "")
	suite.Equal("127.0.0.1:1234", string(res), "uris")

}

func (suite *ConsulHandlerTestSuite) TestGetBrokerStatic() {
	consul_settings.Broker.StaticEndpoint="127.0.0.1:0000"
	suite.handler.Init(consul_settings)
	res, err := suite.handler.GetSingleService(common.NameBrokerService)
	suite.NoError(err, "")
	suite.Equal("127.0.0.1:0000", string(res), "uris")

	res, err = suite.handler.GetSingleService(common.NameBrokerService)
	suite.NoError(err, "")
	suite.Equal("127.0.0.1:0000", string(res), "uris")
}

func (suite *ConsulHandlerTestSuite) TestGetBrokerEmpty() {
	suite.client.Agent().ServiceDeregister("asapo-broker-test1234")
	suite.client.Agent().ServiceDeregister("asapo-broker-test1235")

	suite.handler.Init(consul_settings)
	res, err := suite.handler.GetSingleService(common.NameBrokerService)
	suite.NoError(err, "")
	suite.Equal("", string(res), "uris")
}

