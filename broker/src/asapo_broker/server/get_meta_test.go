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
	suite.mock_db.On("ProcessRequest", expectedBeamtimeId, "", "meta", 0).Return([]byte("{\"test\":10}"), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request meta")))
	ExpectCopyClose(suite.mock_db)

	w := doRequest("/database/" + expectedBeamtimeId + "/0/meta/0" + correctTokenSuffix)
	suite.Equal(http.StatusOK, w.Code, "GetSize OK")
	suite.Equal("{\"test\":10}", string(w.Body.Bytes()), "GetMeta sends meta")
}
