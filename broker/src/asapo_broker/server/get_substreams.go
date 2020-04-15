package server

import (
	"net/http"
)

func routeGetSubstreams(w http.ResponseWriter, r *http.Request) {
	processRequest(w, r, "substreams", "", false)
}
