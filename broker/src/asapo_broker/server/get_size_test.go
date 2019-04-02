package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"net/http"
	"testing"
)

type GetSizeTestSuite struct {
	suite.Suite
	mock_db *database.MockedDatabase
}

func (suite *GetSizeTestSuite) SetupTest() {
	statistics.Reset()
	suite.mock_db = new(database.MockedDatabase)
	db = suite.mock_db
	prepareTestAuth()
	logger.SetMockLog()
}

func (suite *GetSizeTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestGetSizeTestSuite(t *testing.T) {
	suite.Run(t, new(GetSizeTestSuite))
}

func (suite *GetSizeTestSuite) TestGetSizeOK() {
	suite.mock_db.On("ProcessRequest", expectedBeamtimeId, "", "size", 0).Return([]byte("{\"size\":10}"), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request size")))
	ExpectCopyClose(suite.mock_db)

	w := doRequest("/database/" + expectedBeamtimeId + "/size" + correctTokenSuffix)
	suite.Equal(http.StatusOK, w.Code, "GetSize OK")
	suite.Equal("{\"size\":10}", string(w.Body.Bytes()), "GetSize sends size")
}
