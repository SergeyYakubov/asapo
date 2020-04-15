//+build kubernetes_accessible

// for manual tests - kubernetes cluster should be accessible in $HOME/.kube/config

package request_handler

import (
	"asapo_discovery/common"
	"fmt"
	"github.com/stretchr/testify/suite"
	"k8s.io/client-go/kubernetes"
	"testing"
)

type KubernetesHandlerTestSuite struct {
	suite.Suite
	client  *kubernetes.Clientset
	handler KubernetesRequestHandler
}

func TestKubernetesHandlerTestSuite(t *testing.T) {
	suite.Run(t, new(KubernetesHandlerTestSuite))
}

var Kubernetes_settings common.Settings

func (suite *KubernetesHandlerTestSuite) SetupTest() {
//	var err error
	Kubernetes_settings = common.Settings{Receiver: common.ReceiverInfo{MaxConnections: 10, StaticEndpoints: []string{}}}
	Kubernetes_settings.Kubernetes.Mode="external"
}

func (suite *KubernetesHandlerTestSuite) TearDownTest() {

}

func (suite *KubernetesHandlerTestSuite) TestInit() {
	err := suite.handler.Init(Kubernetes_settings)
	suite.NoError(err, "init ok")
}

func (suite *KubernetesHandlerTestSuite) TestRoundRobinBroker() {
	err := suite.handler.Init(Kubernetes_settings)
	suite.handler.Init(consul_settings)
	suite.NoError(err, "")

	for i:=0;i<4;i++ {
		res, err := suite.handler.GetSingleService(common.NameBrokerService)
		suite.NoError(err, "")
		fmt.Println(string(res))
	}
}

func (suite *KubernetesHandlerTestSuite) TestWrongServiceName() {
	err := suite.handler.Init(Kubernetes_settings)
	suite.handler.Init(consul_settings)

	_, err = suite.handler.GetSingleService("bla")
	suite.Error(err, "")
}

func (suite *KubernetesHandlerTestSuite) TestNoRunningInstances() {
// set fts replicas to zero before running it
	err := suite.handler.Init(Kubernetes_settings)
	suite.handler.Init(consul_settings)

	res, err := suite.handler.GetSingleService(common.NameFtsService)
	suite.NoError(err, "")
	suite.Empty(res, "")
}


func (suite *KubernetesHandlerTestSuite) TestGerReceiversBroker() {
	err := suite.handler.Init(Kubernetes_settings)
	suite.NoError(err, "")

	res, err := suite.handler.GetReceivers(false)
	suite.NoError(err, "")
	fmt.Println(string(res))
}