package server

import (
	"github.com/gorilla/mux"
	"asapo_broker/database"
	"asapo_broker/logger"
	"asapo_broker/utils"
	"net/http"
)

func extractRequestParameters(r *http.Request) (string, bool) {
	vars := mux.Vars(r)
	db_name, ok := vars["dbname"]
	return db_name, ok
}

func routeGetNext(w http.ResponseWriter, r *http.Request) {
	r.Header.Set("Content-type", "application/json")
	db_name, ok := extractRequestParameters(r)
	if !ok {
		w.WriteHeader(http.StatusBadRequest)
		return
	}
	answer, code := getNextRecord(db_name)
	w.WriteHeader(code)
	w.Write(answer)
}

func returnError(err error, log_str string) (answer []byte, code int) {
	err_db, ok := err.(*database.DBError)
	code = utils.StatusError
	if ok {
		code = err_db.Code
	}
	if code != utils.StatusNoData {
		logger.Error(log_str + " - " + err.Error())
	} else {
		logger.Debug(log_str + " - " + err.Error())
	}
	return []byte(err.Error()), code
}

func getNextRecord(db_name string) (answer []byte, code int) {
	db_new := db.Copy()
	defer db_new.Close()
	statistics.IncreaseCounter()
	answer, err := db_new.GetNextRecord(db_name)
	log_str := "processing get next request in " + db_name + " at " + settings.BrokerDbAddress
	if err != nil {
		return returnError(err, log_str)
	}
	logger.Debug(log_str)
	return answer, utils.StatusOK
}
