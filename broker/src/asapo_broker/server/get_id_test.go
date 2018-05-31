package server

import (
	"asapo_broker/database"
	"asapo_broker/logger"
	"asapo_broker/utils"
	"errors"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"net/http"
	"testing"
)

func TestGetIdWithoutDatabaseName(t *testing.T) {
	w := doRequest("/database/123")
	assert.Equal(t, http.StatusNotFound, w.Code, "no database name")
}

func ExpectCopyCloseOnID(mock_db *database.MockedDatabase) {
	mock_db.On("Copy").Return(mock_db)
	mock_db.On("Close").Return()
}

type GetIDTestSuite struct {
	suite.Suite
	mock_db *database.MockedDatabase
}

func (suite *GetIDTestSuite) SetupTest() {
	statistics.Reset()
	suite.mock_db = new(database.MockedDatabase)
	db = suite.mock_db
	logger.SetMockLog()
	ExpectCopyCloseOnID(suite.mock_db)
}

func (suite *GetIDTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestGetIDTestSuite(t *testing.T) {
	suite.Run(t, new(GetIDTestSuite))
}

func (suite *GetIDTestSuite) TestGetIDWithWrongDatabaseName() {
	suite.mock_db.On("GetRecordByID", "foo", 1).Return([]byte(""),
		&database.DBError{utils.StatusWrongInput, ""})

	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("get id request in foo")))

	w := doRequest("/database/foo/1")

	suite.Equal(http.StatusBadRequest, w.Code, "wrong database name")
}

func (suite *GetIDTestSuite) TestGetIDWithInternalDBError() {
	suite.mock_db.On("GetRecordByID", "foo", 1).Return([]byte(""), errors.New(""))
	logger.MockLog.On("Error", mock.MatchedBy(containsMatcher("get id request in foo")))

	w := doRequest("/database/foo/1")
	suite.Equal(http.StatusInternalServerError, w.Code, "internal error")
}

func (suite *GetIDTestSuite) TestGetIDOK() {
	suite.mock_db.On("GetRecordByID", "dbname", 1).Return([]byte("Hello"), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("get id request in dbname")))

	w := doRequest("/database/dbname/1")
	suite.Equal(http.StatusOK, w.Code, "GetID OK")
	suite.Equal("Hello", string(w.Body.Bytes()), "GetID sends data")
}
