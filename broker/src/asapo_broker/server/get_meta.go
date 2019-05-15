package server

import (
	"net/http"
)

func routeGetMeta(w http.ResponseWriter, r *http.Request) {
	id, ok := extractRequestParametersID(r)
	if !ok {
		w.WriteHeader(http.StatusBadRequest)
		return
	}
	processRequest(w, r, "meta", id, false)
}
