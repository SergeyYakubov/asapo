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
	substream string
	groupid string
	reqString string
	externalParam string
}{
	{"last", expectedSubstream, expectedGroupID, expectedSubstream + "/" + expectedGroupID + "/last","0"},
	{"id", expectedSubstream, expectedGroupID, expectedSubstream + "/" + expectedGroupID + "/1","1"},
	{"meta", "default", "", "default/0/meta/0","0"},
	{"nacks", expectedSubstream, expectedGroupID, expectedSubstream + "/" + expectedGroupID + "/nacks","0_0"},
	{"next", expectedSubstream, expectedGroupID, expectedSubstream + "/" + expectedGroupID + "/next","0"},
	{"size", expectedSubstream, "", expectedSubstream  + "/size","0"},
	{"substreams", "0", "", "0/substreams",""},
	{"lastack", expectedSubstream, expectedGroupID, expectedSubstream + "/" + expectedGroupID + "/lastack",""},

}


func (suite *GetCommandsTestSuite) TestGetCommandsCallsCorrectRoutine() {
	for _, test := range testsGetCommand {
		suite.mock_db.On("ProcessRequest", expectedDBName, test.substream, test.groupid, test.command, test.externalParam).Return([]byte("Hello"), nil)
		logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request "+test.command)))
		w := doRequest("/database/" + expectedBeamtimeId + "/" + expectedStream + "/" + test.reqString+correctTokenSuffix)
		suite.Equal(http.StatusOK, w.Code, test.command+ " OK")
		suite.Equal("Hello", string(w.Body.Bytes()), test.command+" sends data")
	}
}
