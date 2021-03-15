package server

import (
	"bytes"
	"encoding/json"
	"errors"
	"io"
	"net/http"
	"sync"
	"time"
)

type Token struct {
	Sub string
	AccessType string
}

type Authorizer interface {
	AuthorizeToken(tokenJWT string) (token Token, err error)
}

type HttpClient interface {
	Do(req *http.Request) (*http.Response, error)
}

type HttpError struct{
	err error
	statusCode int
}

func (m *HttpError) Error() string {
	return m.err.Error()
}


type AsapoAuthorizer struct {
	serverUrl string
	httpClient HttpClient
}

type cachedToken struct{
	Token
	lastUpdate time.Time
}

var cachedTokens  = struct {
	tokens map[string]cachedToken
	cachedTokensLock sync.RWMutex
}{tokens:make(map[string]cachedToken,0)}

func getCachedToken(tokenJWT string)(token Token, ok bool) {
	cachedTokens.cachedTokensLock.RLock()
	defer cachedTokens.cachedTokensLock.RUnlock()
	cachedToken,ok:=cachedTokens.tokens[tokenJWT]
	if !ok{
		return  token,false
	}
	if time.Now().Sub(cachedToken.lastUpdate) < 10000*time.Second {
		return cachedToken.Token, true
	}
	return token,false
}

func cacheToken(tokenJWT string,token Token) {
	cachedTokens.cachedTokensLock.Lock()
	defer cachedTokens.cachedTokensLock.Unlock()

	cachedTokens.tokens[tokenJWT] = cachedToken{
		Token:      token,
		lastUpdate: time.Now(),
	}
}

func (a * AsapoAuthorizer) doRequest(req *http.Request) (token Token, err error) {
	resp, err := a.httpClient.Do(req)
	if err != nil {
		return token, err
	}
	defer resp.Body.Close()

	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return token, err
	}

	if resp.StatusCode != http.StatusOK {
		return token, &HttpError{errors.New("returned " + resp.Status + ": " + string(body)),resp.StatusCode}
	}

	err = json.Unmarshal(body, &token)
	return
}
func createIntrospectTokenRequest(tokenJWT string) (*http.Request, error) {
	path := "http://"+settings.AuthorizationServer + "/introspect"
	request := struct {
		Token string
	}{tokenJWT}
	json_data, _ := json.Marshal(request)
	req, err := http.NewRequest("POST", path, bytes.NewBuffer(json_data))
	if err != nil {
		return nil, err
	}
	req.Header.Add("Content-Type", "application/json")
	return req, nil
}

func (a * AsapoAuthorizer) AuthorizeToken(tokenJWT string) (token Token, err error) {
	token,ok := getCachedToken(tokenJWT)
	if ok {
		return
	}

	req, err := createIntrospectTokenRequest(tokenJWT)
	if err != nil {
		return
	}

	token, err = a.doRequest(req)
	if err == nil {
		cacheToken(tokenJWT, token)
	}

	return
}

