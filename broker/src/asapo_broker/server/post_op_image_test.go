package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"net/http"
	"testing"
)

type MessageOpTestSuite struct {
	suite.Suite
	mock_db *database.MockedDatabase
}

func (suite *MessageOpTestSuite) SetupTest() {
	statistics.Reset()
	suite.mock_db = new(database.MockedDatabase)
	db = suite.mock_db
	prepareTestAuth()
	logger.SetMockLog()
}

func (suite *MessageOpTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestMessageOpTestSuite(t *testing.T) {
	suite.Run(t, new(MessageOpTestSuite))
}

func (suite *MessageOpTestSuite) TestAckMessageOpOK() {
	query_str := "{\"Id\":1,\"Op\":\"ackmessage\"}"
	suite.mock_db.On("ProcessRequest", database.Request{DbName: expectedDBName, Stream: expectedStream, GroupId: expectedGroupID, Op: "ackmessage", ExtraParam: query_str}).Return([]byte(""), nil)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request ackmessage")))
	w := doRequest("/beamtime/" + expectedBeamtimeId + "/" + expectedSource + "/" + expectedStream + "/" + expectedGroupID + "/1" + correctTokenSuffix,"POST",query_str)
	suite.Equal(http.StatusOK, w.Code, "ackmessage OK")
}


func (suite *MessageOpTestSuite) TestAckMessageOpErrorWrongOp() {
	query_str := "\"Id\":1,\"Op\":\"ackmessage\"}"
	w := doRequest("/beamtime/" + expectedBeamtimeId + "/" + expectedSource + "/" + expectedStream + "/" + expectedGroupID + "/1" + correctTokenSuffix,"POST",query_str)
	suite.Equal(http.StatusBadRequest, w.Code, "ackmessage wrong")
}

func (suite *MessageOpTestSuite) TestAckMessageOpErrorWrongID() {
	query_str := "{\"Id\":1,\"Op\":\"ackmessage\"}"
	w := doRequest("/beamtime/" + expectedBeamtimeId + "/" + expectedSource + "/" + expectedStream + "/" + expectedGroupID + "/bla" + correctTokenSuffix,"POST",query_str)
	suite.Equal(http.StatusBadRequest, w.Code, "ackmessage wrong")
}
