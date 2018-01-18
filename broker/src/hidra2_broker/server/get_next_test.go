package server

import (
	"github.com/stretchr/testify/assert"
	"hidra2_broker/database"
	"hidra2_broker/utils"
	"net/http"
	"net/http/httptest"
	"testing"
)

type request struct {
	path    string
	cmd     string
	answer  int
	message string
}

func doRequest(path string) *httptest.ResponseRecorder {
	mux := utils.NewRouter(listRoutes)
	req, _ := http.NewRequest("GET", path, nil)
	w := httptest.NewRecorder()
	mux.ServeHTTP(w, req)
	return w
}

func TestGetNextWithoutDatabaseName(t *testing.T) {
	w := doRequest("/next")
	assert.Equal(t, http.StatusBadRequest, w.Code, "no database name")
}

func TestGetNextWithWrongDatabaseName(t *testing.T) {
	mock_db := new(database.MockedDatabase)
	db = mock_db
	defer func() { db = nil }()
	mock_db.On("GetNextRecord", "foo").Return([]byte(""), utils.StatusWrongInput)

	w := doRequest("/next?database=foo")
	assert.Equal(t, http.StatusBadRequest, w.Code, "no database name")
	assertExpectations(t, mock_db)
}

func TestGetNextWithGoodDatabaseName(t *testing.T) {
	mock_db := new(database.MockedDatabase)
	db = mock_db
	defer func() { db = nil }()
	mock_db.On("GetNextRecord", "database").Return([]byte("Hello"), utils.StatusOK)

	w := doRequest("/next?database=database")
	assert.Equal(t, http.StatusOK, w.Code, "GetNext OK")
	assert.Equal(t, "Hello", string(w.Body.Bytes()), "GetNext sends data")
	assertExpectations(t, mock_db)
}
