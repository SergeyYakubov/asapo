package request_handler

import (
	"github.com/stretchr/testify/suite"
	"testing"
     "github.com/hashicorp/consul/api"
	"strconv"
)

type ConsulHandlerTestSuite struct {
	suite.Suite
	client *api.Client
	handler ConsulRequestHandler
}

func TestConsulHandlerTestSuite(t *testing.T) {
	suite.Run(t, new(ConsulHandlerTestSuite))
}

func (suite *ConsulHandlerTestSuite) SetupTest() {
	var err error
	suite.client, err = api.NewClient(api.DefaultConfig())
	if err != nil {
		panic(err)
	}
	for i:=1234;i<1236;i++ {
	reg := &api.AgentServiceRegistration{
		ID:   "receiver"+strconv.Itoa(i),
		Name: "receiver",
		Port: i,
		Check: &api.AgentServiceCheck{
			Interval:"10m",
			Status:"passing",
			HTTP: "http://localhost:5000/health",
		},
	}
	err = suite.client.Agent().ServiceRegister(reg)
	if err != nil {
		panic(err)
	}
	}
}

func (suite *ConsulHandlerTestSuite) TearDownTest() {
	suite.client.Agent().ServiceDeregister("receiver1234")
	suite.client.Agent().ServiceDeregister("receiver1235")
}


func (suite *ConsulHandlerTestSuite) TestInitDefaultUri() {
	err := suite.handler.Init(10,[]string{})
	suite.NoError(err,  "empty list")
}

func (suite *ConsulHandlerTestSuite) TestInitWrongUri() {
	err := suite.handler.Init(10,[]string{"blabla"})
	suite.Error(err,  "wrong consul uri")
	suite.Nil(suite.handler.client,  "client nli after error")

}

func (suite *ConsulHandlerTestSuite) TestInitOkUriFirst() {
	err := suite.handler.Init(10,[]string{"http://127.0.0.1:8500"})
	suite.NoError(err,  "")
}

func (suite *ConsulHandlerTestSuite) TestInitOkUriNotFirst() {
	err := suite.handler.Init(10,[]string{"blabla","http://127.0.0.1:8500"})
	suite.NoError(err,  "")
}


func (suite *ConsulHandlerTestSuite) TestGetReceivers() {
	suite.handler.Init(10,[]string{})
	res,err := suite.handler.GetReceivers()
	suite.NoError(err,  "")
	suite.Equal("{\"MaxConnections\":10,\"Uris\":[\"127.0.0.1:1234\",\"127.0.0.1:1235\"]}",string(res),"uris")
}

func (suite *ConsulHandlerTestSuite) TestGetReceiversWhenNotConnected() {
	suite.handler.Init(10,[]string{"blabla"})
	_,err := suite.handler.GetReceivers()
	suite.Error(err,  "")
}
