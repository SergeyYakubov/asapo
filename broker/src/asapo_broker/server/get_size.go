package server

import (
	"net/http"
)

func routeGetSize(w http.ResponseWriter, r *http.Request) {
	processRequest(w, r, "size", 0, false)
}
