package cli

import (
	"asapo_authorizer/token_store"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"testing"
)

func TestListTokens(t *testing.T) {
	mock_store := new(token_store.MockedStore)
	store = mock_store

	mock_store.On("GetTokenList", mock.Anything).Return([]token_store.TokenRecord{}, nil)
	c := command{"list-tokens", []string{}}
	err := c.CommandList_tokens()
	assert.Nil(t, err)
}