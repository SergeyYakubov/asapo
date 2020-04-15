package server

import (
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"asapo_common/logger"
	"asapo_common/utils"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
	"asapo_discovery/request_handler"
	"asapo_discovery/common"
)

func containsMatcher(substr string) func(str string) bool {
	return func(str string) bool { return strings.Contains(str, substr) }
}

func doRequest(path string) *httptest.ResponseRecorder {
	mux := utils.NewRouter(listRoutes)
	req, _ := http.NewRequest("GET", path, nil)
	w := httptest.NewRecorder()
	mux.ServeHTTP(w, req)
	return w
}

type GetServicesTestSuite struct {
	suite.Suite
}

func (suite *GetServicesTestSuite) SetupTest() {
	requestHandler = new(request_handler.StaticRequestHandler)
	var s common.Settings= common.Settings{Receiver:common.ReceiverInfo{MaxConnections:10,StaticEndpoints:[]string{"ip1","ip2"}},
	Broker:common.BrokerInfo{StaticEndpoint:"ip_broker"},Mongo:common.MongoInfo{StaticEndpoint:"ip_mongo"},
		FileTransferService:common.FtsInfo{StaticEndpoint:"ip_fts"}}

	requestHandler.Init(s)
	logger.SetMockLog()
}

func (suite *GetServicesTestSuite) TearDownTest() {
	logger.UnsetMockLog()
	requestHandler = nil
}

func TestGetServicesTestSuite(t *testing.T) {
	suite.Run(t, new(GetServicesTestSuite))
}

func assertExpectations(t *testing.T) {
	logger.MockLog.AssertExpectations(t)
	logger.MockLog.ExpectedCalls = nil
}

func (suite *GetServicesTestSuite) TestWrongPath() {
	w := doRequest("/blabla")
	suite.Equal(http.StatusNotFound, w.Code, "wrong path")
}

func (suite *GetServicesTestSuite) TestGetReceivers() {
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing get "+common.NameReceiverService)))

	w := doRequest("/asapo-receiver")

	suite.Equal(http.StatusOK, w.Code, "code ok")
	suite.Equal(w.Body.String(), "{\"MaxConnections\":10,\"Uris\":[\"ip1\",\"ip2\"]}", "result")
	assertExpectations(suite.T())
}


func (suite *GetServicesTestSuite) TestGetBroker() {
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing get "+common.NameBrokerService)))

	w := doRequest("/asapo-broker")

	suite.Equal(http.StatusOK, w.Code, "code ok")
	suite.Equal(w.Body.String(), "ip_broker", "result")
	assertExpectations(suite.T())
}

func (suite *GetServicesTestSuite) TestGetMongo() {
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing get "+common.NameMongoService)))

	w := doRequest("/asapo-mongodb")

	suite.Equal(http.StatusOK, w.Code, "code ok")
	suite.Equal(w.Body.String(), "ip_mongo", "result")
	assertExpectations(suite.T())
}

func (suite *GetServicesTestSuite) TestGetFts() {
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing get "+common.NameFtsService)))

	w := doRequest("/asapo-file-transfer")

	suite.Equal(http.StatusOK, w.Code, "code ok")
	suite.Equal(w.Body.String(), "ip_fts", "result")
	assertExpectations(suite.T())
}
