package cli

import (
	"asapo_authorizer/token_store"
	"github.com/stretchr/testify/assert"
	"testing"
)

func TestRevokeTokenToken(t *testing.T) {
	mock_store := new(token_store.MockedStore)
	store = mock_store

	mock_store.On("RevokeToken", "123","").Return(token_store.TokenRecord{}, nil)
	c := command{"revoke-token", []string{"-token","123"}}
	err := c.CommandRevoke_token()
	assert.Nil(t, err)
}

func TestRevokeTokenTokenId(t *testing.T) {
	mock_store := new(token_store.MockedStore)
	store = mock_store

	mock_store.On("RevokeToken", "","123").Return(token_store.TokenRecord{}, nil)
	c := command{"revoke-token", []string{"-token-id","123"}}
	err := c.CommandRevoke_token()
	assert.Nil(t, err)
}