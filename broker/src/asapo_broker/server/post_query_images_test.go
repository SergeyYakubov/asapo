package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"net/http"
	"testing"
)

type QueryTestSuite struct {
	suite.Suite
	mock_db *database.MockedDatabase
}

func (suite *QueryTestSuite) SetupTest() {
	statistics.Reset()
	suite.mock_db = new(database.MockedDatabase)
	db = suite.mock_db
	prepareTestAuth()
	logger.SetMockLog()
}

func (suite *QueryTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestQueryTestSuite(t *testing.T) {
	suite.Run(t, new(QueryTestSuite))
}

func (suite *QueryTestSuite) TestQueryOK() {
	query_str := "aaaa"

	suite.mock_db.On("ProcessRequest", database.Request{DbName: expectedDBName, Stream: expectedStream,Op: "querymessages", ExtraParam: query_str}).Return([]byte("{}"), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request querymessages")))

	w := doRequest("/beamtime/"+expectedBeamtimeId+"/"+expectedSource+"/"+expectedStream+"/0/querymessages"+correctTokenSuffix, "POST", query_str)
	suite.Equal(http.StatusOK, w.Code, "Query OK")
}

