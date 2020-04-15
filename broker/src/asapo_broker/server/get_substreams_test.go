package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"net/http"
	"testing"
)

type GetSubstreamsTestSuite struct {
	suite.Suite
	mock_db *database.MockedDatabase
}

func (suite *GetSubstreamsTestSuite) SetupTest() {
	statistics.Reset()
	suite.mock_db = new(database.MockedDatabase)
	db = suite.mock_db
	prepareTestAuth()
	logger.SetMockLog()
}

func (suite *GetSubstreamsTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestGetSubstreamsTestSuite(t *testing.T) {
	suite.Run(t, new(GetSubstreamsTestSuite))
}

func (suite *GetSubstreamsTestSuite) TestGetSubstreamsCallsCorrectRoutine() {
	suite.mock_db.On("ProcessRequest", expectedDBName, "0", "", "substreams", "").Return([]byte("Hello"), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request substreams")))
	w := doRequest("/database/" + expectedBeamtimeId + "/" + expectedStream + "/0/substreams" + correctTokenSuffix)
	suite.Equal(http.StatusOK, w.Code, "GetSubstreams OK")
	suite.Equal("Hello", string(w.Body.Bytes()), "GetSubstreams sends data")
}
