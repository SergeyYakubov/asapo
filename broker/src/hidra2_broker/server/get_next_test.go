package server

import (
	"errors"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"hidra2_broker/database"
	"hidra2_broker/logger"
	"hidra2_broker/utils"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

type request struct {
	path    string
	cmd     string
	answer  int
	message string
}

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
	logger.SetMockLog()
	ExpectCopyClose(suite.mock_db)
}

func (suite *GetNextTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestGetNextTestSuite(t *testing.T) {
	suite.Run(t, new(GetNextTestSuite))
}

func (suite *GetNextTestSuite) TestGetNextWithWrongDatabaseName() {
	suite.mock_db.On("GetNextRecord", "foo").Return([]byte(""),
		&database.DBError{utils.StatusWrongInput, ""})

	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("get next request in foo")))

	w := doRequest("/database/foo/next")

	suite.Equal(http.StatusBadRequest, w.Code, "wrong database name")
}

func (suite *GetNextTestSuite) TestGetNextWithInternalDBError() {
	suite.mock_db.On("GetNextRecord", "foo").Return([]byte(""), errors.New(""))
	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("get next request in foo")))

	w := doRequest("/database/foo/next")
	suite.Equal(http.StatusInternalServerError, w.Code, "internal error")
}

func (suite *GetNextTestSuite) TestGetNextWithGoodDatabaseName() {
	suite.mock_db.On("GetNextRecord", "dbname").Return([]byte("Hello"), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("get next request in dbname")))

	w := doRequest("/database/dbname/next")
	suite.Equal(http.StatusOK, w.Code, "GetNext OK")
	suite.Equal("Hello", string(w.Body.Bytes()), "GetNext sends data")
}

func (suite *GetNextTestSuite) TestGetNextAddsCounter() {
	suite.mock_db.On("GetNextRecord", "dbname").Return([]byte("Hello"), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("get next request in dbname")))

	doRequest("/database/dbname/next")
	suite.Equal(1, statistics.GetCounter(), "GetNext increases counter")
}
