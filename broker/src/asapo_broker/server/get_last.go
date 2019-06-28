package server

import (
	"net/http"
)

func routeGetLast(w http.ResponseWriter, r *http.Request) {
	processRequest(w, r, "last", "0", true)
}
