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

func (db *MockedDatabase) Ping() error {
	args := db.Called()
	return args.Error(0)
}


func (db *MockedDatabase) ProcessRequest(request Request) (answer []byte, err error) {
	args := db.Called(request)
	return args.Get(0).([]byte), args.Error(1)
}
