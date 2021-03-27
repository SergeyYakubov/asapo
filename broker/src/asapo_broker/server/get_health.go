package server

import (
	"net/http"
)

func routeGetHealth(w http.ResponseWriter, r *http.Request) {
	err := db.Ping()
	if err != nil {
		ReconnectDb()
	}
	w.WriteHeader(http.StatusNoContent)
}
