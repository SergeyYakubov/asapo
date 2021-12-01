package server

import (
	"asapo_common/structs"
	"bytes"
	"encoding/json"
	"errors"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"io/ioutil"
	"net/http"
	"testing"
)

type MockClient struct {
	mock.Mock
}

const expectedAuthorizerUri="http://authorizer:8400/introspect"
const expectedToken="blabla"


func (m *MockClient) Do(req *http.Request) (*http.Response, error) {
	args := m.Called(req)
	return args.Get(0).(*http.Response), args.Error(1)
}

func  matchRequest(req *http.Request) bool {
	pathOk := req.URL.Scheme+"://"+req.URL.Host+req.URL.Path == expectedAuthorizerUri
	b,_:=ioutil.ReadAll(req.Body)
	token := struct {
		Token string
	}{}
	json.Unmarshal(b,&token)
	tokenOk:= token.Token == expectedToken
	return pathOk && tokenOk
}

func responseOk() (*http.Response, error) {
	token := Token{structs.IntrospectTokenResponse{AccessTypes: []string{"read"},Sub: "subject"}}
	b,_:=json.Marshal(&token)
	r := ioutil.NopCloser(bytes.NewReader(b))
	return &http.Response{
		StatusCode: http.StatusOK,
		Body:       r,
	}, nil
}

func responseUnauth() (*http.Response, error) {
	r := ioutil.NopCloser(bytes.NewReader([]byte("wrong or expired JWT token")))
	return &http.Response{
		StatusCode: http.StatusUnauthorized,
		Body:       r,
	}, nil
}

func responseErr() (*http.Response, error) {
	return &http.Response{}, errors.New("cannot connect")
}

var authTests = []struct {
	response func ()(*http.Response, error)
	twice bool
	ok bool
	message string
}{
	{responseOk,false,true,"ok"},
	{responseOk,true,true,"second time uses cache"},
	{responseErr,false,false,"not auth"},
	{responseUnauth,false,false,"request error"},
}

func TestAuthorize(t *testing.T) {
	settings.AuthorizationServer = "authorizer:8400"
	var client MockClient
	auth = &AsapoAuthorizer{
		serverUrl:  expectedAuthorizerUri,
		httpClient: &client,
	}
	for _,test := range authTests {
		client.On("Do",  mock.MatchedBy(matchRequest)).Once().Return(test.response())
		token, err := auth.AuthorizeToken(expectedToken)
		if test.twice {
			token, err = auth.AuthorizeToken(expectedToken)
		}
		client.AssertExpectations(t)
		client.ExpectedCalls = nil
		if test.ok {
			assert.Nil(t,err,test.message)
			assert.Equal(t,"subject",token.Sub,test.message)
			assert.Contains(t,token.AccessTypes,"read",test.message)
		} else {
			assert.NotNil(t,err,test.message)
		}
		delete(cachedTokens.tokens, expectedToken)
	}

}

