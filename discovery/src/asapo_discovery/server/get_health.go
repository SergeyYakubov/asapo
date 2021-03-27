package server

import (
	"net/http"
)

func routeGetHealth(w http.ResponseWriter, r *http.Request) {
	w.WriteHeader(http.StatusNoContent)
}
