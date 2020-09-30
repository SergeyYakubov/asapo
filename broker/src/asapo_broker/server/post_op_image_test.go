package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"net/http"
	"testing"
)

type ImageOpTestSuite struct {
	suite.Suite
	mock_db *database.MockedDatabase
}

func (suite *ImageOpTestSuite) SetupTest() {
	statistics.Reset()
	suite.mock_db = new(database.MockedDatabase)
	db = suite.mock_db
	prepareTestAuth()
	logger.SetMockLog()
}

func (suite *ImageOpTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestImageOpTestSuite(t *testing.T) {
	suite.Run(t, new(ImageOpTestSuite))
}

func (suite *ImageOpTestSuite) TestAckImageOpOK() {
	query_str := "{\"Id\":1,\"Op\":\"ackimage\"}"
	suite.mock_db.On("ProcessRequest", expectedDBName, expectedSubstream, expectedGroupID, "ackimage", query_str).Return([]byte(""), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request ackimage")))
	w := doRequest("/database/" + expectedBeamtimeId + "/" + expectedStream + "/" + expectedSubstream + "/" + expectedGroupID + "/1" + correctTokenSuffix,"POST",query_str)
	suite.Equal(http.StatusOK, w.Code, "ackimage OK")
}
