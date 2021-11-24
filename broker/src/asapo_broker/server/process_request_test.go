package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"asapo_common/structs"
	"asapo_common/utils"
	"errors"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"io"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
	"time"
)

var correctTokenSuffix, correctTokenSuffixWrite, wrongTokenSuffix, suffixWithWrongToken, expectedBeamtimeId, expectedDBName string

const expectedGroupID = "bid2a5auidddp1vl71d0"
const wrongGroupID = "_bid2a5auidddp1vl71"
const expectedSource = "datasource"
const expectedStream = "stream"

type MockAuthServer struct {
}

func (a *MockAuthServer) AuthorizeToken(tokenJWT string) (token Token, err error) {
	if tokenJWT == "ok" {
		return Token{
			structs.IntrospectTokenResponse{
				Sub:         "bt_" + expectedBeamtimeId,
				AccessTypes: []string{"read"},
			},
		}, nil
	}
	if tokenJWT == "ok_write" {
		return Token{
			structs.IntrospectTokenResponse{
				Sub:         "bt_" + expectedBeamtimeId,
				AccessTypes: []string{"read", "write"},
			},
		}, nil
	}

	return Token{}, AuthorizationError{errors.New("wrong JWT token"),http.StatusUnauthorized}
}

func prepareTestAuth() {
	expectedBeamtimeId = "beamtime_id"
	expectedDBName = expectedBeamtimeId + "_" + expectedSource

	auth = &MockAuthServer{}
	correctTokenSuffix = "?token=ok"
	correctTokenSuffixWrite = "?token=ok_write"
	wrongTokenSuffix = "?blablabla=aa"
	suffixWithWrongToken = "?token=wrong"
}

type request struct {
	path    string
	cmd     string
	answer  int
	message string
}

func containsMatcher(substrings ...string) func(str string) bool {
	return func(str string) bool {
		for _, substr := range substrings {
			if !strings.Contains(str, substr) {
				return false
			}
		}
		return true
	}
}

func doRequest(path string, extra_params ...string) *httptest.ResponseRecorder {
	m := "GET"
	if len(extra_params) > 0 {
		m = extra_params[0]
	}
	var body io.Reader = nil
	if len(extra_params) > 1 {
		body = strings.NewReader(extra_params[1])
	}
	ver := "/v0.1"
	if len(extra_params) > 2 {
		ver = extra_params[2]
	}

	mux := utils.NewRouter(listRoutes)
	req, _ := http.NewRequest(m, ver+path, body)
	w := httptest.NewRecorder()
	mux.ServeHTTP(w, req)
	return w
}

func TestProcessRequestWithoutDatabaseName(t *testing.T) {
	w := doRequest("/beamtime/next")
	assert.Equal(t, http.StatusNotFound, w.Code, "no database name")
}

func ExpectReconnect(mock_db *database.MockedDatabase) {
	mock_db.On("Close").Return()
	mock_db.On("Connect", mock.AnythingOfType("string")).Return(nil)
	mock_db.On("SetSettings", mock.Anything).Return()

}

type ProcessRequestTestSuite struct {
	suite.Suite
	mock_db *database.MockedDatabase
}

func (suite *ProcessRequestTestSuite) SetupTest() {
	statistics.Reset()
	suite.mock_db = new(database.MockedDatabase)
	db = suite.mock_db
	prepareTestAuth()
	logger.SetMockLog()
}

func (suite *ProcessRequestTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestProcessRequestTestSuite(t *testing.T) {
	suite.Run(t, new(ProcessRequestTestSuite))
}

func (suite *ProcessRequestTestSuite) TestProcessRequestWithWrongToken() {
	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("wrong JWT token")))

	w := doRequest("/beamtime/" + expectedBeamtimeId + "/" + expectedSource + "/" + expectedStream + "/" + expectedGroupID + "/next" + suffixWithWrongToken)

	suite.Equal(http.StatusUnauthorized, w.Code, "wrong token")
}

func (suite *ProcessRequestTestSuite) TestProcessRequestWithNoToken() {
	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("cannot extract")))

	w := doRequest("/beamtime/" + expectedBeamtimeId + "/" + expectedSource + "/" + expectedStream + "/" + expectedGroupID + "/next" + wrongTokenSuffix)

	suite.Equal(http.StatusBadRequest, w.Code, "no token")
}

func (suite *ProcessRequestTestSuite) TestProcessRequestWithWrongDatabaseName() {

	expectedRequest := database.Request{Beamtime: expectedBeamtimeId,DataSource: expectedSource, Stream: expectedStream, GroupId: expectedGroupID, Op: "next"}

	suite.mock_db.On("ProcessRequest", expectedRequest).Return([]byte(""),
		&database.DBError{utils.StatusNoData, ""})

	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request next")))

	w := doRequest("/beamtime/" + expectedBeamtimeId + "/" + expectedSource + "/" + expectedStream + "/" + expectedGroupID + "/next" + correctTokenSuffix)

	suite.Equal(http.StatusConflict, w.Code, "wrong database name")
}

func (suite *ProcessRequestTestSuite) TestProcessRequestWithConnectionError() {

	expectedRequest := database.Request{Beamtime: expectedBeamtimeId,DataSource: expectedSource, Stream: expectedStream, GroupId: expectedGroupID, Op: "next"}

	suite.mock_db.On("ProcessRequest", expectedRequest).Return([]byte(""),
		&database.DBError{utils.StatusServiceUnavailable, ""})

	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("processing request next")))
	ExpectReconnect(suite.mock_db)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("reconnected")))

	w := doRequest("/beamtime/" + expectedBeamtimeId + "/" + expectedSource + "/" + expectedStream + "/" + expectedGroupID + "/next" + correctTokenSuffix)
	time.Sleep(time.Second)
	suite.Equal(http.StatusServiceUnavailable, w.Code, "data not found")
}

func (suite *ProcessRequestTestSuite) TestProcessRequestWithInternalDBError() {

	expectedRequest := database.Request{Beamtime: expectedBeamtimeId,DataSource: expectedSource, Stream: expectedStream, GroupId: expectedGroupID, Op: "next"}

	suite.mock_db.On("ProcessRequest", expectedRequest).Return([]byte(""), errors.New(""))
	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("processing request next")))
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("reconnected")))

	ExpectReconnect(suite.mock_db)
	w := doRequest("/beamtime/" + expectedBeamtimeId + "/" + expectedSource + "/" + expectedStream + "/" + expectedGroupID + "/next" + correctTokenSuffix)
	time.Sleep(time.Second)

	suite.Equal(http.StatusServiceUnavailable, w.Code, "internal error")
}

func (suite *ProcessRequestTestSuite) TestProcessRequestAddsCounter() {

	expectedRequest := database.Request{Beamtime: expectedBeamtimeId,DataSource: expectedSource, Stream: expectedStream, GroupId: expectedGroupID, Op: "next"}
	suite.mock_db.On("ProcessRequest", expectedRequest).Return([]byte("Hello"), nil)

	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request next in "+expectedDBName)))

	doRequest("/beamtime/" + expectedBeamtimeId + "/" + expectedSource + "/" + expectedStream + "/" + expectedGroupID + "/next" + correctTokenSuffix)
	suite.Equal(1, statistics.GetCounter(), "ProcessRequest increases counter")
}

func (suite *ProcessRequestTestSuite) TestProcessRequestAddsDataset() {

	expectedRequest := database.Request{Beamtime: expectedBeamtimeId,DataSource: expectedSource, Stream: expectedStream, GroupId: expectedGroupID, DatasetOp: true, Op: "next"}
	suite.mock_db.On("ProcessRequest", expectedRequest).Return([]byte("Hello"), nil)

	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request next in "+expectedDBName)))

	doRequest("/beamtime/" + expectedBeamtimeId + "/" + expectedSource + "/" + expectedStream + "/" + expectedGroupID + "/next" + correctTokenSuffix + "&dataset=true")
}

func (suite *ProcessRequestTestSuite) TestProcessRequestErrorOnWrongProtocol() {
	w := doRequest("/beamtime/"+expectedBeamtimeId+"/"+expectedSource+"/"+expectedStream+"/"+expectedGroupID+"/next"+correctTokenSuffix, "GET", "", "/v1.2")
	suite.Equal(http.StatusUnsupportedMediaType, w.Code, "wrong protocol")
}

func (suite *ProcessRequestTestSuite) TestProcessRequestDeleteStreamReadToken() {
	query_str := "query_string"
	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("wrong token access")))
	w := doRequest("/beamtime/"+expectedBeamtimeId+"/"+expectedSource+"/"+expectedStream+"/delete"+correctTokenSuffix, "POST", query_str)
	suite.Equal(http.StatusUnauthorized, w.Code, "wrong token type")

}

func (suite *ProcessRequestTestSuite) TestProcessRequestDeleteStreamWriteToken() {
	query_str := "query_string"

	expectedRequest := database.Request{Beamtime: expectedBeamtimeId,DataSource: expectedSource, Stream: expectedStream, GroupId: "", Op: "delete_stream", ExtraParam: query_str}
	suite.mock_db.On("ProcessRequest", expectedRequest).Return([]byte("Hello"), nil)

	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request delete_stream in "+expectedDBName)))
	doRequest("/beamtime/"+expectedBeamtimeId+"/"+expectedSource+"/"+expectedStream+"/delete"+correctTokenSuffixWrite, "POST", query_str)
}
