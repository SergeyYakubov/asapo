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

func (db *MockedDatabase) Copy() Agent {
	db.Called()
	return db
}

func (db *MockedDatabase) GetNextRecord(db_name string) (answer []byte, err error) {
	args := db.Called(db_name)
	return args.Get(0).([]byte), args.Error(1)
}

func (db *MockedDatabase) GetRecordByID(db_name string, id int) (answer []byte, err error) {
	args := db.Called(db_name, id)
	return args.Get(0).([]byte), args.Error(1)
}
