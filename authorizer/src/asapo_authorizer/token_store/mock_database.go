// +build !release

package token_store

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

func (db *MockedDatabase) ProcessRequest(request Request, extraParams ...interface{}) (answer []byte, err error) {
	args := db.Called(request,extraParams)
	return args.Get(0).([]byte), args.Error(1)
}


type FakeDatabase struct {
}

func (db *FakeDatabase) Connect(address string) error {
	return nil
}

func (db *FakeDatabase) Close() {
	return
}

func (db *FakeDatabase) Ping() error {
	return nil
}

func (db *FakeDatabase) ProcessRequest(request Request, extraParams ...interface{}) (answer []byte, err error) {
	return nil,nil
}
