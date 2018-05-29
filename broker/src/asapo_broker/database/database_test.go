package database

import (
	"github.com/stretchr/testify/mock"
	"testing"
)

// we run this test just to get 100% coverage for mock_database.go
func TestMockDataBase(t *testing.T) {
	var db MockedDatabase
	db.On("Connect", mock.AnythingOfType("string")).Return(nil)
	db.On("Close").Return()
	db.On("Copy").Return(nil)
	db.On("GetNextRecord", "").Return([]byte(""), nil)
	db.On("GetRecordByID", "").Return([]byte(""), nil)
	db.Connect("")
	db.GetNextRecord("")
	db.Close()
	db.Copy()
	var err DBError
	err.Error()
}
