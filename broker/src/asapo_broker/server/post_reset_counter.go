package server

import (
	"net/http"
)

func routeResetCounter(w http.ResponseWriter, r *http.Request) {
	processRequest(w, r, "resetcounter", "0", true)
}
