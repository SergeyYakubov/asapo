package server

import (
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"asapo_discovery/logger"
	"asapo_discovery/utils"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
	"asapo_discovery/request_handler"
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

type GetReceiversTestSuite struct {
	suite.Suite
}

func (suite *GetReceiversTestSuite) SetupTest() {
	requestHandler = new(request_handler.StaticRequestHandler)
	var s utils.Settings= utils.Settings{Receiver:utils.ReceiverInfo{MaxConnections:10,ForceEndpoints:[]string{"ip1","ip2"}},
	Broker:utils.BrokerInfo{ForceEndpoint:"ip_broker"}}

	requestHandler.Init(s)
	logger.SetMockLog()
}

func (suite *GetReceiversTestSuite) TearDownTest() {
	logger.UnsetMockLog()
	requestHandler = nil
}

func TestGetReceiversTestSuite(t *testing.T) {
	suite.Run(t, new(GetReceiversTestSuite))
}

func assertExpectations(t *testing.T) {
	logger.MockLog.AssertExpectations(t)
	logger.MockLog.ExpectedCalls = nil
}

func (suite *GetReceiversTestSuite) TestWrongPath() {
	w := doRequest("/blabla")
	suite.Equal(http.StatusNotFound, w.Code, "wrong path")
}

func (suite *GetReceiversTestSuite) TestGetReceivers() {
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing get receivers")))

	w := doRequest("/receivers")

	suite.Equal(http.StatusOK, w.Code, "code ok")
	suite.Equal(w.Body.String(), "{\"MaxConnections\":10,\"Uris\":[\"ip1\",\"ip2\"]}", "result")
	assertExpectations(suite.T())
}


func (suite *GetReceiversTestSuite) TestGetBroker() {
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing get broker")))

	w := doRequest("/broker")

	suite.Equal(http.StatusOK, w.Code, "code ok")
	suite.Equal(w.Body.String(), "ip_broker", "result")
	assertExpectations(suite.T())
}


