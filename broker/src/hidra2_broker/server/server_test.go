package server

import (
	"errors"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"hidra2_broker/database"
	"testing"
)

func setup() (*database.MockedDatabase, *Server) {
	db := new(database.MockedDatabase)
	srv := new(Server)
	db.On("Connect", mock.AnythingOfType("string")).Return(nil)
	return db, srv

}

func assertExpectations(t *testing.T, db *database.MockedDatabase) {
	db.AssertExpectations(t)
	db.ExpectedCalls = nil
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
	db, srv := setup()
	db.ExpectedCalls = nil

	for _, test := range initDBTests {
		db.On("Connect", mock.AnythingOfType("string")).Return(test.answer)

		err := srv.InitDB(db)

		assert.Equal(t, test.answer, err, test.message)
		assertExpectations(t, db)
	}
}

func TestCleanupDBWithoutInit(t *testing.T) {
	db, srv := setup()

	db.AssertNotCalled(t, "Close")

	srv.CleanupDB()
}

func TestCleanupDBInit(t *testing.T) {
	db, srv := setup()

	db.On("Close").Return()

	srv.InitDB(db)
	srv.CleanupDB()

	assertExpectations(t, db)
}
