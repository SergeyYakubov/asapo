package server

import (
	"net/http"
)

func routeGetSize(w http.ResponseWriter, r *http.Request) {
	keys := r.URL.Query()
	incomplete := keys.Get("incomplete")
	processRequest(w, r, "size", incomplete, false)
}
