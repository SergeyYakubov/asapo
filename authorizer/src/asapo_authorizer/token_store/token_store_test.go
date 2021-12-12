package token_store

import (
	"asapo_authorizer/common"
	"asapo_common/logger"
	"asapo_common/utils"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/suite"
	"strings"
	"testing"
	"time"
)

const expectedSource = "datasource"
const expectedStream = "stream"

func ExpectReconnect(mock_db *MockedDatabase) {
	mock_db.On("Close").Return()
	mock_db.On("Connect", mock.Anything).Return(nil)
}

func assertExpectations(t *testing.T, mock_db *MockedDatabase) {
	mock_db.AssertExpectations(t)
	mock_db.ExpectedCalls = nil
	logger.MockLog.AssertExpectations(t)
	logger.MockLog.ExpectedCalls = nil
}


type TokenStoreTestSuite struct {
	suite.Suite
	mock_db *MockedDatabase
	store Store
}

func (suite *TokenStoreTestSuite) SetupTest() {
	common.Settings.UpdateRevokedTokensIntervalSec = 0
	suite.mock_db = new(MockedDatabase)
	suite.store = new(TokenStore)
	suite.mock_db.On("Connect", mock.Anything).Return( nil)
	suite.store.Init(suite.mock_db)
	logger.SetMockLog()
}

func (suite *TokenStoreTestSuite) TearDownTest() {
	assertExpectations(suite.T(), suite.mock_db)
	logger.UnsetMockLog()
	suite.store = nil
}

func TestTokenStoreTestSuite(t *testing.T) {
	suite.Run(t, new(TokenStoreTestSuite))
}

func containsMatcher(substr string) func(str string) bool {
	return func(str string) bool { return strings.Contains(str, substr) }
}

func (suite *TokenStoreTestSuite) TestProcessRequestWithConnectionError() {
	ExpectReconnect(suite.mock_db)
	suite.mock_db.On("ProcessRequest", mock.Anything, mock.Anything).Return([]byte(""),
		&DBError{utils.StatusServiceUnavailable, ""})
	logger.MockLog.On("WithFields", mock.Anything)
	logger.MockLog.On("Debug", mock.MatchedBy(containsMatcher("reconnected")))

	err := suite.store.AddToken(TokenRecord{})
	time.Sleep(time.Second)
	suite.Error(err, "need reconnect")
}


func (suite *TokenStoreTestSuite) TestProcessRequestAddToken() {
	req := Request{
		DbName:     "asapo_admin",
		Collection: "tokens",
		Op:         "create_record",
	}
	suite.mock_db.On("ProcessRequest", req, mock.Anything).Return([]byte(""), nil)
	err := suite.store.AddToken(TokenRecord{})
	suite.Equal(err, nil, "ok")
}


func (suite *TokenStoreTestSuite) TestProcessRequestGetTokenList() {
	req := Request{
		DbName:     "asapo_admin",
		Collection: "tokens",
		Op:         "read_records",
	}
	suite.mock_db.On("ProcessRequest", req, mock.Anything).Return([]byte(""), nil)
	_,err := suite.store.GetTokenList()
	suite.Equal(err, nil, "ok")
}


func (suite *TokenStoreTestSuite) TestProcessRequestRevokeToken() {
	req := Request{
		DbName:     "asapo_admin",
		Collection: "tokens",
		Op:         "read_record",
	}
	expectedToken := TokenRecord{Id :"1234", Token:"token",Revoked: false}
	expectedRevokedToken := expectedToken
	expectedRevokedToken.Revoked = true
	suite.mock_db.On("ProcessRequest", req,mock.Anything).Return([]byte(""), nil).Run(func(args mock.Arguments) {
		rec := args.Get(1).([]interface{})[1].(*TokenRecord)
		*rec = expectedToken
	})

	req = Request{
		DbName:     KAdminDb,
		Collection: KTokens,
		Op:         "update_record"}
	suite.mock_db.On("ProcessRequest", req,mock.Anything).Return([]byte(""), nil).Run(func(args mock.Arguments) {
		rec := args.Get(1).([]interface{})[3].(*TokenRecord)
		*rec = expectedRevokedToken
	})

	req = Request{
		DbName:     "asapo_admin",
		Collection: "revoked_tokens",
		Op:         "create_record",
	}
	suite.mock_db.On("ProcessRequest", req, mock.Anything).Return([]byte(""), nil)

	token,err := suite.store.RevokeToken(expectedToken.Token,"")
	suite.Equal(err, nil, "ok")
	suite.Equal(token, expectedRevokedToken, "ok")
}

func (suite *TokenStoreTestSuite) TestProcessRequestCheckRevokedToken() {
	suite.mock_db.On("Close")
	suite.store.Close()
	common.Settings.UpdateRevokedTokensIntervalSec = 5
	suite.store.Init(suite.mock_db)
	req := Request{
		DbName:     "asapo_admin",
		Collection: "revoked_tokens",
		Op:         "read_records",
	}
	suite.mock_db.On("ProcessRequest", req, mock.Anything).Return([]byte(""), nil)
	time.Sleep(time.Second*1)
	res,err := suite.store.IsTokenRevoked("123")
	suite.Equal(err, nil, "ok")
	suite.Equal(false, res, "ok")
}