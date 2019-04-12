package server

import (
	"github.com/gorilla/mux"
	"net/http"
	"strconv"
)

func extractRequestParametersID(r *http.Request) (int, bool) {
	vars := mux.Vars(r)
	id_str, ok := vars["id"]
	if !ok {
		return 0, ok
	}
	id, err := strconv.Atoi(id_str)
	return id, err == nil
}

func routeGetByID(w http.ResponseWriter, r *http.Request) {
	id, ok := extractRequestParametersID(r)
	if !ok {
		w.WriteHeader(http.StatusBadRequest)
		return
	}
	processRequest(w, r, "id", id, true)
}
