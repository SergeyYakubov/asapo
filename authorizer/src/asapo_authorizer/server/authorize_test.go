package server

import (
	"asapo_common/utils"
	"github.com/stretchr/testify/assert"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
	"io/ioutil"
)

type request struct {
	path    string
	cmd     string
	answer  int
	message string
}

func containsMatcher(substr string) func(str string) bool {
	return func(str string) bool { return strings.Contains(str, substr) }
}

func makeRequest(request authorizationRequest) string {
	buf, _ := utils.MapToJson(request)
	return string(buf)
}

func doAuthorizeRequest(path string,buf string) *httptest.ResponseRecorder {
	mux := utils.NewRouter(listRoutes)
	req, _ := http.NewRequest("POST", path, strings.NewReader(buf))
	w := httptest.NewRecorder()
	mux.ServeHTTP(w, req)
	return w
}

func TestAuthorizeOK(t *testing.T) {
	request :=  makeRequest(authorizationRequest{"asapo_test","host"})
	w := doAuthorizeRequest("/authorize",request)

	body, _ := ioutil.ReadAll(w.Body)

	assert.Equal(t, string(body), "asapo_test", "")
	assert.Equal(t, http.StatusOK, w.Code, "")
}

func TestNotAuthorized(t *testing.T) {
	request :=  makeRequest(authorizationRequest{"any_id","host"})
	w := doAuthorizeRequest("/authorize",request)
	assert.Equal(t, http.StatusUnauthorized, w.Code, "")
}


func TestAuthorizeWrongRequest(t *testing.T) {
	w := doAuthorizeRequest("/authorize","babla")
	assert.Equal(t, http.StatusBadRequest, w.Code, "")
}


func TestAuthorizeWrongPath(t *testing.T) {
	w := doAuthorizeRequest("/authorized","")
	assert.Equal(t, http.StatusNotFound, w.Code, "")
}



