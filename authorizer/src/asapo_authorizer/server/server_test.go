package server

import (
	"asapo_authorizer/database"
	"asapo_common/discovery"
	"asapo_common/logger"
	"errors"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"net/http"
	"net/http/httptest"
	"testing"
)

func setup() *database.MockedDatabase {
	mock_db := new(database.MockedDatabase)
	mock_db.On("Connect", mock.AnythingOfType("string")).Return(nil)

	return mock_db
}

func setup_and_init(t *testing.T) *database.MockedDatabase {
	mock_db := new(database.MockedDatabase)
	mock_db.On("Connect", mock.AnythingOfType("string")).Return(nil)

	InitDB(mock_db)
	assertExpectations(t, mock_db)
	return mock_db
}

func assertExpectations(t *testing.T, mock_db *database.MockedDatabase) {
	mock_db.AssertExpectations(t)
	mock_db.ExpectedCalls = nil
	logger.MockLog.AssertExpectations(t)
	logger.MockLog.ExpectedCalls = nil
}

var initDBTests = []struct {
	address string
	answer  error
	message string
}{
	{"bad address", errors.New(""), "error on get bad address"},
	{"good address", nil, "no error on good address"},
}

func TestInitDBWithWrongAddress(t *testing.T) {
	mock_db := setup()

	mock_db.ExpectedCalls = nil

	settings.DatabaseServer = "0.0.0.0:0000"

	for _, test := range initDBTests {
		mock_db.On("Connect", "0.0.0.0:0000").Return(test.answer)

		err := InitDB(mock_db)

		assert.Equal(t, test.answer, err, test.message)
		assertExpectations(t, mock_db)
	}
	db = nil
}

func TestInitDBWithAutoAddress(t *testing.T) {
	mongo_address := "0.0.0.0:0000"
	mock_db := setup()

	mock_db.ExpectedCalls = nil

	settings.DatabaseServer = "auto"
	mock_server := httptest.NewServer(http.HandlerFunc(func(rw http.ResponseWriter, req *http.Request) {
		assert.Equal(t, req.URL.String(), "/asapo-mongodb", "request string")
		rw.Write([]byte(mongo_address))
	}))
	defer mock_server.Close()

	discoveryService = discovery.CreateDiscoveryService(mock_server.Client(), mock_server.URL)

	mock_db.On("Connect", "0.0.0.0:0000").Return(nil)

	err := InitDB(mock_db)

	assert.Equal(t, nil, err, "auto connect ok")
	assertExpectations(t, mock_db)
	db = nil
}

func TestReconnectDB(t *testing.T) {
	mongo_address := "0.0.0.0:0000"
	mock_server := httptest.NewServer(http.HandlerFunc(func(rw http.ResponseWriter, req *http.Request) {
		assert.Equal(t, req.URL.String(), "/asapo-mongodb", "request string")
		rw.Write([]byte(mongo_address))
	}))
	discoveryService = discovery.CreateDiscoveryService(mock_server.Client(), mock_server.URL)

	defer mock_server.Close()

	settings.DatabaseServer = "auto"
	mock_db := setup_and_init(t)
	mock_db.ExpectedCalls = nil

	mongo_address = "1.0.0.0:0000"

	mock_db.On("Close").Return()

	mock_db.On("Connect", "1.0.0.0:0000").Return(nil)

	err := ReconnectDb()
	assert.Equal(t, nil, err, "auto connect ok")
	assertExpectations(t, mock_db)

	db = nil
}

func TestErrorWhenReconnectNotConnectedDB(t *testing.T) {
	err := ReconnectDb()
	assert.NotNil(t, err, "error reconnect")
	db = nil
}


func TestCleanupDBWithoutInit(t *testing.T) {
	mock_db := setup()

	mock_db.AssertNotCalled(t, "Close")

	CleanupDB()
}

func TestCleanupDBInit(t *testing.T) {
	settings.DatabaseServer = "0.0.0.0"
	mock_db := setup_and_init(t)

	mock_db.On("Close").Return()

	CleanupDB()

	assertExpectations(t, mock_db)
}
