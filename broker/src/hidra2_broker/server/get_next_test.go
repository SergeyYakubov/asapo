package server

import (
	"errors"
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
	w := doRequest("/database/next")
	assert.Equal(t, http.StatusNotFound, w.Code, "no database name")
}

func TestGetNextWithWrongDatabaseName(t *testing.T) {
	mock_db := new(database.MockedDatabase)
	db = mock_db
	defer func() { db = nil }()
	mock_db.On("GetNextRecord", "foo").Return([]byte(""),
		&database.DBError{utils.StatusWrongInput, ""})

	w := doRequest("/database/foo/next")
	assert.Equal(t, http.StatusBadRequest, w.Code, "wrong database name")
	assertExpectations(t, mock_db)
}

func TestGetNextWithInternalDBError(t *testing.T) {
	mock_db := new(database.MockedDatabase)
	db = mock_db
	defer func() { db = nil }()
	mock_db.On("GetNextRecord", "foo").Return([]byte(""), errors.New(""))

	w := doRequest("/database/foo/next")
	assert.Equal(t, http.StatusInternalServerError, w.Code, "internal error")
	assertExpectations(t, mock_db)
}

func TestGetNextWithGoodDatabaseName(t *testing.T) {
	mock_db := new(database.MockedDatabase)
	db = mock_db
	defer func() { db = nil }()
	mock_db.On("GetNextRecord", "dbname").Return([]byte("Hello"), nil)

	w := doRequest("/database/dbname/next")
	assert.Equal(t, http.StatusOK, w.Code, "GetNext OK")
	assert.Equal(t, "Hello", string(w.Body.Bytes()), "GetNext sends data")
	assertExpectations(t, mock_db)
}
