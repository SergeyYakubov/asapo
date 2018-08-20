package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"asapo_common/utils"
	"errors"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

var correctTokenSuffix, wrongTokenSuffix, suffixWithWrongToken, expectedBeamtimeId string

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

func doRequest(path string) *httptest.ResponseRecorder {
	mux := utils.NewRouter(listRoutes)
	req, _ := http.NewRequest("GET", path, nil)
	w := httptest.NewRecorder()
	mux.ServeHTTP(w, req)
	return w
}

func TestGetNextWithoutDatabaseName(t *testing.T) {
	w := doRequest("/database/next")
	assert.Equal(t, http.StatusNotFound, w.Code, "no database name")
}

func ExpectCopyClose(mock_db *database.MockedDatabase) {
	mock_db.On("Copy").Return(mock_db)
	mock_db.On("Close").Return()
}

type GetNextTestSuite struct {
	suite.Suite
	mock_db *database.MockedDatabase
}

func (suite *GetNextTestSuite) SetupTest() {
	statistics.Reset()
	suite.mock_db = new(database.MockedDatabase)
	db = suite.mock_db
	prepareTestAuth()
	logger.SetMockLog()
}

func (suite *GetNextTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestGetNextTestSuite(t *testing.T) {
	suite.Run(t, new(GetNextTestSuite))
}

func (suite *GetNextTestSuite) TestGetNextWithWrongToken() {
	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("wrong token")))

	w := doRequest("/database/" + expectedBeamtimeId + "/next" + suffixWithWrongToken)

	suite.Equal(http.StatusUnauthorized, w.Code, "wrong token")
}

func (suite *GetNextTestSuite) TestGetNextWithNoToken() {
	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("cannot extract")))

	w := doRequest("/database/" + expectedBeamtimeId + "/next" + wrongTokenSuffix)

	suite.Equal(http.StatusUnauthorized, w.Code, "no token")
}

func (suite *GetNextTestSuite) TestGetNextWithWrongDatabaseName() {
	suite.mock_db.On("GetNextRecord", expectedBeamtimeId).Return([]byte(""),
		&database.DBError{utils.StatusWrongInput, ""})

	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("get next request")))
	ExpectCopyClose(suite.mock_db)

	w := doRequest("/database/" + expectedBeamtimeId + "/next" + correctTokenSuffix)

	suite.Equal(http.StatusBadRequest, w.Code, "wrong database name")
}

func (suite *GetNextTestSuite) TestGetNextWithInternalDBError() {
	suite.mock_db.On("GetNextRecord", expectedBeamtimeId).Return([]byte(""), errors.New(""))
	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("get next request")))
	ExpectCopyClose(suite.mock_db)

	w := doRequest("/database/" + expectedBeamtimeId + "/next" + correctTokenSuffix)
	suite.Equal(http.StatusInternalServerError, w.Code, "internal error")
}

func (suite *GetNextTestSuite) TestGetNextWithGoodDatabaseName() {
	suite.mock_db.On("GetNextRecord", expectedBeamtimeId).Return([]byte("Hello"), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("get next request")))
	ExpectCopyClose(suite.mock_db)

	w := doRequest("/database/" + expectedBeamtimeId + "/next" + correctTokenSuffix)
	suite.Equal(http.StatusOK, w.Code, "GetNext OK")
	suite.Equal("Hello", string(w.Body.Bytes()), "GetNext sends data")
}

func (suite *GetNextTestSuite) TestGetNextAddsCounter() {
	suite.mock_db.On("GetNextRecord", expectedBeamtimeId).Return([]byte("Hello"), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("get next request in "+expectedBeamtimeId)))
	ExpectCopyClose(suite.mock_db)

	doRequest("/database/" + expectedBeamtimeId + "/next" + correctTokenSuffix)
	suite.Equal(1, statistics.GetCounter(), "GetNext increases counter")
}
