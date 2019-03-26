package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"net/http"
	"testing"
)

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

func (suite *GetNextTestSuite) TestGetNextCallsCorrectRoutine() {
	suite.mock_db.On("GetRecordFromDb", expectedBeamtimeId, expectedGroupID, "next", 0).Return([]byte("Hello"), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("get next request")))
	ExpectCopyClose(suite.mock_db)

	w := doRequest("/database/" + expectedBeamtimeId + "/" + expectedGroupID + "/next" + correctTokenSuffix)
	suite.Equal(http.StatusOK, w.Code, "GetNext OK")
	suite.Equal("Hello", string(w.Body.Bytes()), "GetNext sends data")
}
