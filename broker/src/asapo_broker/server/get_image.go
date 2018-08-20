package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"asapo_common/utils"
	"github.com/gorilla/mux"
	"net/http"
)

func extractRequestParameters(r *http.Request) (string, bool) {
	vars := mux.Vars(r)
	db_name, ok := vars["dbname"]
	return db_name, ok
}

func getImage(w http.ResponseWriter, r *http.Request, op string, id int) {
	r.Header.Set("Content-type", "application/json")
	db_name, ok := extractRequestParameters(r)
	if !ok {
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	if err := testAuth(r, db_name); err != nil {
		writeAuthAnswer(w, "get "+op, db_name, err.Error())
		return
	}

	answer, code := getRecord(db_name, op, id)
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

func getRecord(db_name string, op string, id int) (answer []byte, code int) {
	db_new := db.Copy()
	defer db_new.Close()
	statistics.IncreaseCounter()
	answer, err := db_new.GetRecordFromDb(db_name, op, id)
	log_str := "processing get " + op + " request in " + db_name + " at " + settings.BrokerDbAddress
	if err != nil {
		return returnError(err, log_str)
	}
	logger.Debug(log_str)
	return answer, utils.StatusOK
}
