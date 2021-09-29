// +build !release

package token_store

import (
	"github.com/stretchr/testify/mock"
)

type MockedStore struct {
	mock.Mock
}

func (store *MockedStore) Init(db Agent) error {
	args := store.Called(db)
	return args.Error(0)
}

func (store *MockedStore)  AddToken(token TokenRecord) error {
	args := store.Called(token)
	return args.Error(0)
}

func (store *MockedStore)  RevokeToken(token string,id string)  (TokenRecord, error) {
	args := store.Called(token,id)
	return args.Get(0).(TokenRecord), args.Error(1)
}

func (store *MockedStore)  GetTokenList() ([]TokenRecord,error) {
	args := store.Called()
	return args.Get(0).([]TokenRecord), args.Error(1)
}

func (store *MockedStore) GetRevokedTokenIds() ([]string,error) {
	args := store.Called()
	return args.Get(0).([]string), args.Error(1)
}

func (store *MockedStore) Close() {
	store.Called()
	return
}


