package server

import (
	"asapo_broker/logger"
	"asapo_broker/utils"
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
	r.Header.Set("Content-type", "application/json")
	db_name, ok := extractRequestParameters(r)
	if !ok {
		w.WriteHeader(http.StatusBadRequest)
		return
	}
	id, ok := extractRequestParametersID(r)
	if !ok {
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	answer, code := getRecordByID(db_name, id)
	w.WriteHeader(code)
	w.Write(answer)
}

func getRecordByID(db_name string, id int) (answer []byte, code int) {
	db_new := db.Copy()
	defer db_new.Close()
	statistics.IncreaseCounter()
	answer, err := db_new.GetRecordByID(db_name, id)
	log_str := "processing get id request in " + db_name + " at " + settings.BrokerDbAddress
	if err != nil {
		return returnError(err, log_str)
	}
	logger.Debug(log_str)
	return answer, utils.StatusOK
}
