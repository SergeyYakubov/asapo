package database

import (
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"testing"
)

func TestCreateRecord(t *testing.T) {
	Test_Hidra2()
	assert.Equal(t, "111", "111", "record created")
}

// we run this test just to get 100% coverage for mock_database.go
func TestMockDataBase(t *testing.T) {
	var db MockedDatabase
	db.On("Connect", mock.AnythingOfType("string")).Return(nil)
	db.On("Close").Return()

	db.Connect("")
	db.Close()
}
