package server

import (
	"asapo_broker/database"
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
	mock_db := setup()

	mock_db.ExpectedCalls = nil

	settings.DatabaseServer = "auto"
	mock_server := httptest.NewServer(http.HandlerFunc(func(rw http.ResponseWriter, req *http.Request) {
		assert.Equal(t, req.URL.String(), "/mongo", "request string")
		rw.Write([]byte("0.0.0.0:0000"))
	}))
	defer mock_server.Close()

	discoveryService = discoveryAPI{mock_server.Client(), mock_server.URL}

	mock_db.On("Connect", "0.0.0.0:0000").Return(nil)
	err := InitDB(mock_db)

	assert.Equal(t, nil, err, "auto connect ok")
	assertExpectations(t, mock_db)
	db = nil
}

func TestCleanupDBWithoutInit(t *testing.T) {
	mock_db := setup()

	mock_db.AssertNotCalled(t, "Close")

	CleanupDB()
}

func TestCleanupDBInit(t *testing.T) {
	mock_db := setup()

	mock_db.On("Close").Return()

	InitDB(mock_db)
	CleanupDB()

	assertExpectations(t, mock_db)
}
