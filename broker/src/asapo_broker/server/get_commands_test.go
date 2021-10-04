package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"net/http"
	"net/url"
	"strings"
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
	source string
	stream string
	groupid string
	reqString string
	queryParams string
	externalParam string
}{
	{"last", expectedSource,expectedStream, "", expectedStream + "/0/last","","0"},
	{"id", expectedSource,expectedStream, "", expectedStream + "/0/1","","1"},
	{"meta", expectedSource,"default", "", "default/0/meta/0","","0"},
	{"nacks",expectedSource, expectedStream, expectedGroupID, expectedStream + "/" + expectedGroupID + "/nacks","","0_0"},
	{"next", expectedSource,expectedStream, expectedGroupID, expectedStream + "/" + expectedGroupID + "/next","",""},
	{"next", expectedSource,expectedStream, expectedGroupID, expectedStream + "/" +
		expectedGroupID + "/next","&resend_nacks=true&delay_ms=10000&resend_attempts=3","10000_3"},
	{"size", expectedSource,expectedStream, "", expectedStream  + "/size","",""},
	{"size",expectedSource, expectedStream, "", expectedStream  + "/size","&incomplete=true","true"},
	{"streams",expectedSource, "0", "", "0/streams","","0/"},
	{"lastack", expectedSource,expectedStream, expectedGroupID, expectedStream + "/" + expectedGroupID + "/lastack","",""},
}


func (suite *GetCommandsTestSuite) TestGetCommandsCallsCorrectRoutine() {
	for _, test := range testsGetCommand {
		suite.mock_db.On("ProcessRequest", database.Request{DbName: expectedDBName, Stream: test.stream, GroupId: test.groupid, Op: test.command, ExtraParam: test.externalParam}).Return([]byte("Hello"), nil)
		logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request "+test.command)))
		w := doRequest("/beamtime/" + expectedBeamtimeId + "/" + test.source + "/" + test.reqString+correctTokenSuffix+test.queryParams)
		suite.Equal(http.StatusOK, w.Code, test.command+ " OK")
		suite.Equal("Hello", string(w.Body.Bytes()), test.command+" sends data")
	}
}

func (suite *GetCommandsTestSuite) TestGetCommandsCorrectlyProcessedEncoding() {
	badSymbols:="%$&./\\_$&\""
	for _, test := range testsGetCommand {
		newstream := test.stream+badSymbols
		newsource := test.source+badSymbols
		newgroup :=""
		if test.groupid!="" {
			newgroup = test.groupid+badSymbols
		}
		encodedStream:=url.PathEscape(newstream)
		encodedSource:=url.PathEscape(newsource)
		encodedGroup:=url.PathEscape(newgroup)
		test.reqString = strings.Replace(test.reqString,test.groupid,encodedGroup,1)
		test.reqString = strings.Replace(test.reqString,test.source,encodedSource,1)
		test.reqString = strings.Replace(test.reqString,test.stream,encodedStream,1)
		dbname := expectedBeamtimeId + "_" + newsource
		suite.mock_db.On("ProcessRequest", database.Request{DbName: dbname, Stream: newstream, GroupId: newgroup, Op: test.command, ExtraParam: test.externalParam}).Return([]byte("Hello"), nil)
		logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("processing request "+test.command)))
		w := doRequest("/beamtime/" + expectedBeamtimeId + "/" + encodedSource + "/" + test.reqString+correctTokenSuffix+test.queryParams)
		suite.Equal(http.StatusOK, w.Code, test.command+ " OK")
		suite.Equal("Hello", string(w.Body.Bytes()), test.command+" sends data")
	}
}

