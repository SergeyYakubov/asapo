package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	"asapo_common/utils"
	"fmt"
	"github.com/gorilla/mux"
	"github.com/rs/xid"
	"net/http"
)

func extractRequestParameters(r *http.Request, needGroupID bool) (string, string, bool) {
	vars := mux.Vars(r)
	db_name, ok1 := vars["dbname"]
	ok2 := true
	group_id := ""
	if needGroupID {
		group_id, ok2 = vars["groupid"]
	}
	return db_name, group_id, ok1 && ok2
}

func checkGroupID(w http.ResponseWriter, needGroupID bool, group_id string, db_name string, op string) bool {
	if !needGroupID {
		return true
	}
	if _, err := xid.FromString(group_id); err != nil {
		err_str := "wrong groupid " + group_id
		log_str := "processing get " + op + " request in " + db_name + " at " + settings.BrokerDbAddress + ": " + err_str
		logger.Error(log_str)
		fmt.Println(log_str)
		w.WriteHeader(http.StatusBadRequest)
		w.Write([]byte(err_str))
		return false
	}
	return true
}

func processRequest(w http.ResponseWriter, r *http.Request, op string, extra_param string, needGroupID bool) {
	r.Header.Set("Content-type", "application/json")
	w.Header().Set("Access-Control-Allow-Origin", "*")
	db_name, group_id, ok := extractRequestParameters(r, needGroupID)
	if !ok {
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	if err := testAuth(r, db_name); err != nil {
		writeAuthAnswer(w, "get "+op, db_name, err.Error())
		return
	}

	if !checkGroupID(w, needGroupID, group_id, db_name, op) {
		return
	}

	if op == "id" && resetRequested(r) {
		op = "idreset"
	}

	answer, code := processRequestInDb(db_name, group_id, op, extra_param)
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

func processRequestInDb(db_name string, group_id string, op string, extra_param string) (answer []byte, code int) {
	db_new := db.Copy()
	defer db_new.Close()
	statistics.IncreaseCounter()
	answer, err := db_new.ProcessRequest(db_name, group_id, op, extra_param)
	log_str := "processing request " + op + " in " + db_name + " at " + settings.BrokerDbAddress
	if err != nil {
		return returnError(err, log_str)
	}
	logger.Debug(log_str)
	return answer, utils.StatusOK
}
