package server

import (
	"asapo_authorizer/database"
	"asapo_common/logger"
	"asapo_common/utils"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"testing"
	"time"
)


const expectedSource = "datasource"
const expectedStream = "stream"


func ExpectReconnect(mock_db *database.MockedDatabase) {
	mock_db.On("Close").Return()
	mock_db.On("Connect", mock.AnythingOfType("string")).Return(nil)
}

type ProcessRequestTestSuite struct {
	suite.Suite
	mock_db *database.MockedDatabase
}

func (suite *ProcessRequestTestSuite) SetupTest() {
	suite.mock_db = new(database.MockedDatabase)
	db = suite.mock_db
	logger.SetMockLog()
}

func (suite *ProcessRequestTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestProcessRequestTestSuite(t *testing.T) {
	suite.Run(t, new(ProcessRequestTestSuite))
}


func (suite *ProcessRequestTestSuite) TestProcessRequestWithConnectionError() {
	req := database.Request{}

	suite.mock_db.On("ProcessRequest", req).Return([]byte(""),
		&database.DBError{utils.StatusServiceUnavailable, ""})

	ExpectReconnect(suite.mock_db)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("reconnected")))

	_,err := ProcessRequestInDb(req)
	time.Sleep(time.Second)
	suite.Error(err, "need reconnect")
}


func (suite *ProcessRequestTestSuite) TestProcessRequestAddTokenToDb() {
	req := database.Request{
		DbName:     "test",
		Collection: "test",
		Op:         "test",
		ExtraParam: "test",
	}

	suite.mock_db.On("ProcessRequest", req).Return([]byte("Hello"), nil)


	_,err := ProcessRequestInDb(req)
	suite.Equal(err, nil, "ok")

}
