package server

import (
	"net/http"
)

func extractRequestParameters(r *http.Request) (string, bool) {
	db_name := r.URL.Query().Get("database")
	return db_name, db_name != ""
}

func routeGetNext(w http.ResponseWriter, r *http.Request) {
	r.Header.Set("Content-type", "application/json")

	db_name, ok := extractRequestParameters(r)
	if !ok {
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	answer, ok := getNextRecord(db_name)
	if !ok {
		w.WriteHeader(http.StatusBadRequest)
	} else {
		w.WriteHeader(http.StatusOK)
	}
	w.Write(answer)
}

func getNextRecord(db_name string) (answer []byte, ok bool) {
	return db.GetNextRecord(db_name, "data")
}
