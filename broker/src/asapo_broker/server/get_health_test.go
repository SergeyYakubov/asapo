package server

import (
	"github.com/stretchr/testify/assert"
	"net/http"
	"testing"
)


func TestGetNext(t *testing.T) {
	w := doRequest("/health")
	assert.Equal(t, http.StatusNoContent, w.Code)
}
