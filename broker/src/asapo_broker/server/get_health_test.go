package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"errors"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"net/http"
	"testing"
)

type GetHealthTestSuite struct {
	suite.Suite
	mock_db *database.MockedDatabase
}

func (suite *GetHealthTestSuite) SetupTest() {
	statistics.Reset()
	suite.mock_db = new(database.MockedDatabase)
	db = suite.mock_db
	logger.SetMockLog()
}

func (suite *GetHealthTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	db = nil
}

func TestGetHealthTestSuite(t *testing.T) {
	suite.Run(t, new(GetHealthTestSuite))
}

func (suite *GetHealthTestSuite) TestGetHealthOk() {
	suite.mock_db.On("Ping").Return(nil)
	w := doRequest("/health","GET","","")
	suite.Equal(http.StatusNoContent, w.Code)
}

func (suite *GetHealthTestSuite) TestGetHealthTriesToReconnectsToDataBase() {
	suite.mock_db.On("Ping").Return(errors.New("ping error"))
	suite.mock_db.On("SetSettings", mock.Anything).Return()

	ExpectReconnect(suite.mock_db)

	w := doRequest("/health","GET","","")
	suite.Equal(http.StatusNoContent, w.Code)
}
