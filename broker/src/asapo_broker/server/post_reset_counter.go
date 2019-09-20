package server

import (
	"net/http"
)

func extractRequestParametersValue(r *http.Request) string {
	val := r.URL.Query().Get("value")
	if len(val) == 0 {
		return "0"
	}
	return val
}

func routeResetCounter(w http.ResponseWriter, r *http.Request) {
	val := extractRequestParametersValue(r)
	processRequest(w, r, "resetcounter", val, true)
}
