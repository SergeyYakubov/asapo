package server

import (
	"github.com/stretchr/testify/assert"
	"net/http"
	"testing"
	"net/http/httptest"
	"asapo_common/utils"
)


func TestGetNext(t *testing.T) {
		mux := utils.NewRouter(listRoutes)
		req, _ := http.NewRequest("GET", "/health-check", nil)
		w := httptest.NewRecorder()
		mux.ServeHTTP(w, req)
		assert.Equal(t, http.StatusNoContent, w.Code)
}
