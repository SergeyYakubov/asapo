package server

import (
	"net/http"
)

func routeGetHealth(w http.ResponseWriter, r *http.Request) {
	db_new := db.Copy()
	defer db_new.Close()
	err := db_new.Ping()
	if err != nil {
		ReconnectDb()
	}
	r.Header.Set("Content-type", "application/json")
	w.WriteHeader(http.StatusNoContent)
}
