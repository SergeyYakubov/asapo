package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"net/http"
	"testing"
)

type GetLastTestSuite struct {
	suite.Suite
	mock_db *database.MockedDatabase
}

func (suite *GetLastTestSuite) SetupTest() {
	statistics.Reset()
	suite.mock_db = new(database.MockedDatabase)
	db = suite.mock_db
	prepareTestAuth()
	logger.SetMockLog()
}

func (suite *GetLastTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestGetLastTestSuite(t *testing.T) {
	suite.Run(t, new(GetLastTestSuite))
}

func (suite *GetLastTestSuite) TestGetLastCallsCorrectRoutine() {
	suite.mock_db.On("ProcessRequest", expectedDBName, expectedGroupID, "last", "0").Return([]byte("Hello"), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request last")))

	w := doRequest("/database/" + expectedBeamtimeId + "/" + expectedStream + "/" + expectedGroupID + "/last" + correctTokenSuffix)
	suite.Equal(http.StatusOK, w.Code, "GetLast OK")
	suite.Equal("Hello", string(w.Body.Bytes()), "GetLast sends data")
}
