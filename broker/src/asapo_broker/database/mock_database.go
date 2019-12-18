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

func (db *MockedDatabase) ProcessRequest(db_name string, data_collection_name string, group_id string, op string, extra_param string) (answer []byte, err error) {
	args := db.Called(db_name, data_collection_name, group_id, op, extra_param)
	return args.Get(0).([]byte), args.Error(1)
}
