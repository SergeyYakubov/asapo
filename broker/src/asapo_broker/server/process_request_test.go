package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
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
)

var correctTokenSuffix, wrongTokenSuffix, suffixWithWrongToken, expectedBeamtimeId string

const expectedGroupID = "bid2a5auidddp1vl71d0"
const wrongGroupID = "bid2a5auidddp1vl71"

func prepareTestAuth() {
	expectedBeamtimeId = "beamtime_id"
	auth = utils.NewHMACAuth("secret")
	token, err := auth.GenerateToken(&expectedBeamtimeId)
	if err != nil {
		panic(err)
	}
	correctTokenSuffix = "?token=" + token
	wrongTokenSuffix = "?blablabla=aa"
	suffixWithWrongToken = "?token=blabla"
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

	mux := utils.NewRouter(listRoutes)
	req, _ := http.NewRequest(m, path, body)
	w := httptest.NewRecorder()
	mux.ServeHTTP(w, req)
	return w
}

func TestProcessRequestWithoutDatabaseName(t *testing.T) {
	w := doRequest("/database/next")
	assert.Equal(t, http.StatusNotFound, w.Code, "no database name")
}

func ExpectCopyClose(mock_db *database.MockedDatabase) {
	mock_db.On("Copy").Return(mock_db)
	mock_db.On("Close").Return()
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
	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("wrong token")))

	w := doRequest("/database/" + expectedBeamtimeId + "/" + expectedGroupID + "/next" + suffixWithWrongToken)

	suite.Equal(http.StatusUnauthorized, w.Code, "wrong token")
}

func (suite *ProcessRequestTestSuite) TestProcessRequestWithNoToken() {
	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("cannot extract")))

	w := doRequest("/database/" + expectedBeamtimeId + "/" + expectedGroupID + "/next" + wrongTokenSuffix)

	suite.Equal(http.StatusUnauthorized, w.Code, "no token")
}

func (suite *ProcessRequestTestSuite) TestProcessRequestWithWrongDatabaseName() {
	suite.mock_db.On("ProcessRequest", expectedBeamtimeId, expectedGroupID, "next", "0").Return([]byte(""),
		&database.DBError{utils.StatusWrongInput, ""})

	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("processing request next")))
	ExpectCopyClose(suite.mock_db)

	w := doRequest("/database/" + expectedBeamtimeId + "/" + expectedGroupID + "/next" + correctTokenSuffix)

	suite.Equal(http.StatusBadRequest, w.Code, "wrong database name")
}

func (suite *ProcessRequestTestSuite) TestProcessRequestWithInternalDBError() {
	suite.mock_db.On("ProcessRequest", expectedBeamtimeId, expectedGroupID, "next", "0").Return([]byte(""), errors.New(""))
	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("processing request next")))
	ExpectCopyClose(suite.mock_db)

	w := doRequest("/database/" + expectedBeamtimeId + "/" + expectedGroupID + "/next" + correctTokenSuffix)
	suite.Equal(http.StatusInternalServerError, w.Code, "internal error")
}

func (suite *ProcessRequestTestSuite) TestProcessRequestAddsCounter() {
	suite.mock_db.On("ProcessRequest", expectedBeamtimeId, expectedGroupID, "next", "0").Return([]byte("Hello"), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request next in "+expectedBeamtimeId)))
	ExpectCopyClose(suite.mock_db)

	doRequest("/database/" + expectedBeamtimeId + "/" + expectedGroupID + "/next" + correctTokenSuffix)
	suite.Equal(1, statistics.GetCounter(), "ProcessRequest increases counter")
}

func (suite *ProcessRequestTestSuite) TestProcessRequestWrongGroupID() {
	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("wrong groupid")))
	w := doRequest("/database/" + expectedBeamtimeId + "/" + wrongGroupID + "/next" + correctTokenSuffix)
	suite.Equal(http.StatusBadRequest, w.Code, "wrong group id")
}

func (suite *ProcessRequestTestSuite) TestProcessRequestAddsDataset() {
	suite.mock_db.On("ProcessRequest", expectedBeamtimeId, expectedGroupID, "next_dataset", "0").Return([]byte("Hello"), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request next_dataset in "+expectedBeamtimeId)))
	ExpectCopyClose(suite.mock_db)

	doRequest("/database/" + expectedBeamtimeId + "/" + expectedGroupID + "/next" + correctTokenSuffix + "&dataset=true")
}
