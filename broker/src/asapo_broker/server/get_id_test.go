package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
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

type GetIDTestSuite struct {
	suite.Suite
	mock_db *database.MockedDatabase
}

func (suite *GetIDTestSuite) SetupTest() {
	prepareTestAuth()
	statistics.Reset()
	suite.mock_db = new(database.MockedDatabase)
	db = suite.mock_db
	logger.SetMockLog()
}

func (suite *GetIDTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestGetIDTestSuite(t *testing.T) {
	suite.Run(t, new(GetIDTestSuite))
}

func (suite *GetIDTestSuite) TestGetIdCallsCorrectRoutine() {
	suite.mock_db.On("ProcessRequest", expectedDBName, expectedGroupID, "id", "1").Return([]byte("Hello"), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request")))

	w := doRequest("/database/" + expectedBeamtimeId + "/" + expectedStream + "/" + expectedGroupID + "/1" + correctTokenSuffix)
	suite.Equal(http.StatusOK, w.Code, "GetImage OK")
	suite.Equal("Hello", string(w.Body.Bytes()), "GetID sends data")
}
