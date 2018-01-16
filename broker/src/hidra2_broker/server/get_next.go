package server

import (
	"net/http"
)

func routeGetNext(w http.ResponseWriter, r *http.Request) {
	r.Header.Set("Content-type", "application/json")
	w.WriteHeader(http.StatusOK)
}
