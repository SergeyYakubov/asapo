package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"net/http"
	"testing"
)

type GetNacksTestSuite struct {
	suite.Suite
	mock_db *database.MockedDatabase
}

func (suite *GetNacksTestSuite) SetupTest() {
	statistics.Reset()
	suite.mock_db = new(database.MockedDatabase)
	db = suite.mock_db
	prepareTestAuth()
	logger.SetMockLog()
}

func (suite *GetNacksTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestGetNacksTestSuite(t *testing.T) {
	suite.Run(t, new(GetNacksTestSuite))
}

func (suite *GetNacksTestSuite) TestGetNacksCallsCorrectRoutine() {
	suite.mock_db.On("ProcessRequest", expectedDBName, expectedSubstream, expectedGroupID, "nacks", "1_0").Return([]byte("Hello"), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request nacks")))

	w := doRequest("/database/" + expectedBeamtimeId + "/" + expectedStream + "/" + expectedSubstream + "/" + expectedGroupID + "/nacks" + correctTokenSuffix+"&from=1&to=0")
	suite.Equal(http.StatusOK, w.Code, "GetNacks OK")
	suite.Equal("Hello", string(w.Body.Bytes()), "GetNacks sends data")
}

func (suite *GetNacksTestSuite) TestGetNacksCallsCorrectRoutineWithoutLimits() {
	suite.mock_db.On("ProcessRequest", expectedDBName, expectedSubstream, expectedGroupID, "nacks", "0_0").Return([]byte("Hello"), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request nacks")))

	w := doRequest("/database/" + expectedBeamtimeId + "/" + expectedStream + "/" + expectedSubstream + "/" + expectedGroupID + "/nacks" + correctTokenSuffix)
	suite.Equal(http.StatusOK, w.Code, "GetNacks OK")
	suite.Equal("Hello", string(w.Body.Bytes()), "GetNacks sends data")
}
