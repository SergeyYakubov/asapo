package server

import (
	"github.com/stretchr/testify/assert"
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

var getNextTests = []request{
	{"next", "GET", http.StatusOK, "get next job"},
}

func TestGetNext(t *testing.T) {
	mux := utils.NewRouter(listRoutes)

	for _, test := range getNextTests {

		req, err := http.NewRequest(test.cmd, "/"+test.path+"/", nil)

		assert.Nil(t, err, "Should not be error")

		w := httptest.NewRecorder()
		mux.ServeHTTP(w, req)
		assert.Equal(t, test.answer, w.Code, test.message)
	}
}
