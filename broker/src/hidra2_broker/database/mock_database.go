// +build !release

package database

import (
	"github.com/stretchr/testify/mock"
)

type MockedDatabase struct {
	mock.Mock
}

func (db *MockedDatabase) Connect(address string) error {
	args := db.Called(address)
	return args.Error(0)
}

func (db *MockedDatabase) Close() {
	db.Called()
}

func (db *MockedDatabase) GetNextRecord(db_name string) (answer []byte, code int) {
	args := db.Called(db_name)
	return args.Get(0).([]byte), args.Int(1)
}
