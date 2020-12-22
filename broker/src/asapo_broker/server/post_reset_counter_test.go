package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"net/http"
	"testing"
)

type ResetCounterTestSuite struct {
	suite.Suite
	mock_db *database.MockedDatabase
}

func (suite *ResetCounterTestSuite) SetupTest() {
	statistics.Reset()
	suite.mock_db = new(database.MockedDatabase)
	db = suite.mock_db
	prepareTestAuth()
	logger.SetMockLog()
}

func (suite *ResetCounterTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestResetCounterTestSuite(t *testing.T) {
	suite.Run(t, new(ResetCounterTestSuite))
}

func (suite *ResetCounterTestSuite) TestResetCounterOK() {
	expectedRequest := database.Request{DbName: expectedDBName, DbCollectionName: expectedStream, GroupId:expectedGroupID, Op: "resetcounter", ExtraParam: "10"}
	suite.mock_db.On("ProcessRequest", expectedRequest).Return([]byte(""), nil)

	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request resetcounter")))

	w := doRequest("/database/"+expectedBeamtimeId+"/"+expectedSource+"/"+expectedStream+"/"+expectedGroupID+"/resetcounter"+correctTokenSuffix+"&value=10", "POST")
	suite.Equal(http.StatusOK, w.Code, "ResetCounter OK")
}
