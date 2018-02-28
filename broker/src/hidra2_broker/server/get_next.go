package server

import (
	"github.com/gorilla/mux"
	"hidra2_broker/database"
	"hidra2_broker/utils"
	"net/http"
)

func extractRequestParameters(r *http.Request) (string, bool) {
	vars := mux.Vars(r)
	db_name, ok := vars["dbname"]
	return db_name, ok
}

func routeGetNext(w http.ResponseWriter, r *http.Request) {
	r.Header.Set("Content-type", "application/json")
	// w.Write([]byte("Hello"))
	// return
	db_name, ok := extractRequestParameters(r)
	if !ok {
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	answer, code := getNextRecord(db_name)
	w.WriteHeader(code)
	w.Write(answer)
}

func getNextRecord(db_name string) (answer []byte, code int) {
	db_new := db.Copy()
	defer db_new.Close()
	statistics.IncreaseCounter()
	answer, err := db_new.GetNextRecord(db_name)
	if err != nil {
		err_db, ok := err.(*database.DBError)
		code = utils.StatusError
		if ok {
			code = err_db.Code
		}
		return []byte(err.Error()), code
	}

	return answer, utils.StatusOK
}
