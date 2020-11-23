package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"net/http"
	"testing"
)

type GetMetaTestSuite struct {
	suite.Suite
	mock_db *database.MockedDatabase
}

func (suite *GetMetaTestSuite) SetupTest() {
	statistics.Reset()
	suite.mock_db = new(database.MockedDatabase)
	db = suite.mock_db
	prepareTestAuth()
	logger.SetMockLog()
}

func (suite *GetMetaTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestGetMetaTestSuite(t *testing.T) {
	suite.Run(t, new(GetMetaTestSuite))
}

func (suite *GetMetaTestSuite) TestGetMetaOK() {
	suite.mock_db.On("ProcessRequest", database.Request{DbName: expectedDBName, DbCollectionName: expectedSubstream, Op: "meta", ExtraParam: "1"}).Return([]byte(""), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request meta")))
	w := doRequest("/database/" + expectedBeamtimeId + "/" + expectedStream + "/" + expectedSubstream + "/0/meta"  + "/1" + correctTokenSuffix,"GET")
	suite.Equal(http.StatusOK, w.Code, "meta OK")
}

