package server

import (
	"net/http"
)

func routeGetNext(w http.ResponseWriter, r *http.Request) {
	processRequest(w, r, "next", "0", true)
}
