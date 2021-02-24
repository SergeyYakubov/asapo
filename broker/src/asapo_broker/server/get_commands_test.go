package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"net/http"
	"testing"
)

type GetCommandsTestSuite struct {
	suite.Suite
	mock_db *database.MockedDatabase
}

func (suite *GetCommandsTestSuite) SetupTest() {
	statistics.Reset()
	suite.mock_db = new(database.MockedDatabase)
	db = suite.mock_db
	prepareTestAuth()
	logger.SetMockLog()
}

func (suite *GetCommandsTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestGetCommandsTestSuite(t *testing.T) {
	suite.Run(t, new(GetCommandsTestSuite))
}

var testsGetCommand = []struct {
	command string
	stream string
	groupid string
	reqString string
	queryParams string
	externalParam string
}{
	{"last", expectedStream, "", expectedStream + "/0/last","","0"},
	{"id", expectedStream, "", expectedStream + "/0/1","","1"},
	{"meta", "default", "", "default/0/meta/0","","0"},
	{"nacks", expectedStream, expectedGroupID, expectedStream + "/" + expectedGroupID + "/nacks","","0_0"},
	{"next", expectedStream, expectedGroupID, expectedStream + "/" + expectedGroupID + "/next","",""},
	{"next", expectedStream, expectedGroupID, expectedStream + "/" +
		expectedGroupID + "/next","&resend_nacks=true&delay_ms=10000&resend_attempts=3","10000_3"},
	{"size", expectedStream, "", expectedStream  + "/size","",""},
	{"size", expectedStream, "", expectedStream  + "/size","&incomplete=true","true"},
	{"streams", "0", "", "0/streams","",""},
	{"lastack", expectedStream, expectedGroupID, expectedStream + "/" + expectedGroupID + "/lastack","",""},
}


func (suite *GetCommandsTestSuite) TestGetCommandsCallsCorrectRoutine() {
	for _, test := range testsGetCommand {
		suite.mock_db.On("ProcessRequest", database.Request{DbName: expectedDBName, DbCollectionName: test.stream, GroupId: test.groupid, Op: test.command, ExtraParam: test.externalParam}).Return([]byte("Hello"), nil)
		logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request "+test.command)))
		w := doRequest("/database/" + expectedBeamtimeId + "/" + expectedSource + "/" + test.reqString+correctTokenSuffix+test.queryParams)
		suite.Equal(http.StatusOK, w.Code, test.command+ " OK")
		suite.Equal("Hello", string(w.Body.Bytes()), test.command+" sends data")
	}
}
