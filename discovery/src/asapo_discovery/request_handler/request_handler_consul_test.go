package request_handler

import (
	"github.com/stretchr/testify/suite"
	"testing"
	"github.com/hashicorp/consul/api"
	"strconv"
	"asapo_common/utils"
)

type ConsulHandlerTestSuite struct {
	suite.Suite
	client  *api.Client
	handler ConsulRequestHandler
}

func TestConsulHandlerTestSuite(t *testing.T) {
	suite.Run(t, new(ConsulHandlerTestSuite))
}

var consul_settings utils.Settings

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
	consul_settings = utils.Settings{Receiver: utils.ReceiverInfo{MaxConnections: 10, StaticEndpoints: []string{}}}

	suite.client, err = api.NewClient(api.DefaultConfig())
	if err != nil {
		panic(err)
	}

	suite.registerAgents("receiver")
	suite.registerAgents("broker")

}

func (suite *ConsulHandlerTestSuite) TearDownTest() {
	suite.client.Agent().ServiceDeregister("receiver1234")
	suite.client.Agent().ServiceDeregister("receiver1235")
	suite.client.Agent().ServiceDeregister("broker1234")
	suite.client.Agent().ServiceDeregister("broker1235")
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
	res, err := suite.handler.GetReceivers()
	suite.NoError(err, "")
	suite.Equal("{\"MaxConnections\":10,\"Uris\":[\"127.0.0.1:1234\",\"127.0.0.1:1235\"]}", string(res), "uris")
}

func (suite *ConsulHandlerTestSuite) TestGetReceiversWhenNotConnected() {
	consul_settings.ConsulEndpoints = []string{"blabla"}
	suite.handler.Init(consul_settings)
	_, err := suite.handler.GetReceivers()
	suite.Error(err, "")
}

func (suite *ConsulHandlerTestSuite) TestGetBrokerWhenNotConnected() {
	consul_settings.ConsulEndpoints = []string{"blabla"}
	suite.handler.Init(consul_settings)
	_, err := suite.handler.GetBroker()
	suite.Error(err, "")
}

func (suite *ConsulHandlerTestSuite) TestGetBrokerRoundRobin() {
	suite.handler.Init(consul_settings)
	res, err := suite.handler.GetBroker()
	suite.NoError(err, "")
	suite.Equal("127.0.0.1:1234", string(res), "uris")

	res, err = suite.handler.GetBroker()
	suite.NoError(err, "")
	suite.Equal("127.0.0.1:1235", string(res), "uris")

	res, err = suite.handler.GetBroker()
	suite.NoError(err, "")
	suite.Equal("127.0.0.1:1234", string(res), "uris")

}


func (suite *ConsulHandlerTestSuite) TestGetBrokerEmpty() {
	suite.client.Agent().ServiceDeregister("broker1234")
	suite.client.Agent().ServiceDeregister("broker1235")

	suite.handler.Init(consul_settings)
	res, err := suite.handler.GetBroker()
	suite.NoError(err, "")
	suite.Equal("", string(res), "uris")
}


