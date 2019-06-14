package server

import (
	"github.com/gorilla/mux"
	"net/http"
)

func extractRequestParametersID(r *http.Request) (string, bool) {
	vars := mux.Vars(r)
	id_str, ok := vars["id"]
	if !ok {
		return "0", ok
	}
	return id_str, true
}

func routeGetByID(w http.ResponseWriter, r *http.Request) {
	id, ok := extractRequestParametersID(r)
	if !ok {
		w.WriteHeader(http.StatusBadRequest)
		return
	}
	processRequest(w, r, "id", id, true)
}
