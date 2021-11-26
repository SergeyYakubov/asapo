package server

import (
	"asapo_common/logger"
	"asapo_common/utils"
	"asapo_common/version"
	"asapo_discovery/common"
	"asapo_discovery/request_handler"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
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
	var s common.Settings = common.Settings{Receiver: common.ReceiverInfo{MaxConnections: 10, StaticEndpoints: []string{"ip1", "ip2"}},
		Broker: common.BrokerInfo{StaticEndpoint: "ip_broker"}, Mongo: common.MongoInfo{StaticEndpoint: "ip_mongo"},
		FileTransferService: common.FtsInfo{StaticEndpoint: "ip_fts"}}

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

type requestTest struct {
request string
code int
message string
}

var receiverTests = []requestTest {
	{"/" + version.GetDiscoveryApiVersion()+"/asapo-receiver",http.StatusBadRequest,"protocol missing"},
	{"/" + version.GetDiscoveryApiVersion()+"/asapo-receiver?protocol=v1000.2",http.StatusUnsupportedMediaType,"wrong protocol"},
	{"/" + version.GetDiscoveryApiVersion()+"/asapo-receiver?protocol=v0.2",http.StatusOK,"ok"},
}

func (suite *GetServicesTestSuite) TestGetReceivers() {
	for _,test:= range receiverTests {
		if test.code == http.StatusOK {
			logger.MockLog.On("WithFields", mock.Anything)
			logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("request")))
		} else {
			logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("validating producer")))
		}

		w := doRequest(test.request)

		suite.Equal(test.code, w.Code, test.message)
		if test.code == http.StatusOK {
			suite.Equal(w.Body.String(), "{\"MaxConnections\":10,\"Uris\":[\"ip1\",\"ip2\"]}", "result")
		}
		assertExpectations(suite.T())
	}

}

var brokerTests = []requestTest {
	{"/" + version.GetDiscoveryApiVersion()+"/asapo-broker",http.StatusBadRequest,"protocol missing"},
	{"/" + version.GetDiscoveryApiVersion()+"/asapo-broker?protocol=v1000.2",http.StatusUnsupportedMediaType,"wrong protocol"},
	{"/" + version.GetDiscoveryApiVersion()+"/asapo-broker?protocol=v0.2",http.StatusOK,"ok"},
}
func (suite *GetServicesTestSuite) TestGetBroker() {
	for _,test:= range brokerTests {
		if test.code == http.StatusOK {
			logger.MockLog.On("WithFields", mock.Anything)
			logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("request")))
		} else {
			logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("validating consumer")))
		}

		w := doRequest(test.request)

		suite.Equal(test.code, w.Code, test.message)
		if test.code == http.StatusOK {
			suite.Equal(w.Body.String(), "ip_broker", "result")
		}
		assertExpectations(suite.T())
	}
}


func (suite *GetServicesTestSuite) TestGetMongo() {
	logger.MockLog.On("WithFields", mock.Anything)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("request")))

	w := doRequest("/asapo-mongodb")

	suite.Equal(http.StatusOK, w.Code, "code ok")
	suite.Equal(w.Body.String(), "ip_mongo", "result")
	assertExpectations(suite.T())
}

func (suite *GetServicesTestSuite) TestGetFts() {
	logger.MockLog.On("WithFields", mock.Anything)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("request")))

	w := doRequest("/" + version.GetDiscoveryApiVersion()+"/asapo-file-transfer?protocol=v0.1")

	suite.Equal(http.StatusOK, w.Code, "code ok")
	suite.Equal(w.Body.String(), "ip_fts", "result")
	assertExpectations(suite.T())
}

func (suite *GetServicesTestSuite) TestGetVersions() {
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("request")))

	w := doRequest("/" + version.GetDiscoveryApiVersion() + "/version")

	suite.Equal(http.StatusOK, w.Code, "code ok")
	// we dont really check what it returns, just that route is ok
	suite.Contains(w.Body.String(), version.GetVersion(), "core version")
	suite.Contains(w.Body.String(), "supportedProtocols", "protocols")
	assertExpectations(suite.T())
}
