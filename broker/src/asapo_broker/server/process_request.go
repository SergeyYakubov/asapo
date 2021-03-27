package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	log "asapo_common/logger"
	"asapo_common/utils"
	"asapo_common/version"
	"github.com/gorilla/mux"
	"net/http"
)

func extractRequestParameters(r *http.Request, needGroupID bool) (string, string, string, string, bool) {
	vars := mux.Vars(r)
	db_name, ok1 := vars["beamtime"]

	datasource, ok3 := vars["datasource"]
	stream, ok4 := vars["stream"]

	ok2 := true
	group_id := ""
	if needGroupID {
		group_id, ok2 = vars["groupid"]
	}
	return db_name, datasource, stream, group_id, ok1 && ok2 && ok3 && ok4
}

func IsLetterOrNumbers(s string) bool {
	for _, r := range s {
		if (r < 'a' || r > 'z') && (r < 'A' || r > 'Z') && (r<'0' || r>'9') {
			return false
		}
	}
	return true
}


func checkGroupID(w http.ResponseWriter, needGroupID bool, group_id string, db_name string, op string) bool {
	if !needGroupID {
		return true
	}
	if  len(group_id) > 0 && len (group_id) < 100 && IsLetterOrNumbers(group_id) {
		return true
	}
	err_str := "wrong groupid " + group_id
	log_str := "processing get " + op + " request in " + db_name + " at " + settings.GetDatabaseServer() + ": " + err_str
	logger.Error(log_str)
	w.WriteHeader(http.StatusBadRequest)
	w.Write([]byte(err_str))
	return false
}

func checkBrokerApiVersion(w http.ResponseWriter, r *http.Request) bool {
	_, ok := utils.PrecheckApiVersion(w, r, version.GetBrokerApiVersion())
	return ok
}

func processRequest(w http.ResponseWriter, r *http.Request, op string, extra_param string, needGroupID bool) {
	if ok := checkBrokerApiVersion(w, r); !ok {
		return
	}


	w.Header().Set("Access-Control-Allow-Origin", "*")
	db_name, datasource, stream, group_id, ok := extractRequestParameters(r, needGroupID)
	if !ok {
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	if err := authorize(r, db_name); err != nil {
		writeAuthAnswer(w, "get "+op, db_name, err)
		return
	}

	if !checkGroupID(w, needGroupID, group_id, db_name, op) {
		return
	}

	request := database.Request{}
	request.DbName = db_name+"_"+datasource
	request.Op = op
	request.ExtraParam = extra_param
	request.DbCollectionName = stream
	request.GroupId = group_id
	if yes, minSize := datasetRequested(r); yes {
		request.DatasetOp = true
		request.MinDatasetSize = minSize
	}

	answer, code := processRequestInDb(request)
	w.WriteHeader(code)
	w.Write(answer)
}

func returnError(err error, log_str string) (answer []byte, code int) {
	code = database.GetStatusCodeFromError(err)
	if code != utils.StatusNoData && code != utils.StatusPartialData{
		logger.Error(log_str + " - " + err.Error())
	} else {
		logger.Debug(log_str + " - " + err.Error())
	}
	return []byte(err.Error()), code
}

func reconnectIfNeeded(db_error error) {
	code := database.GetStatusCodeFromError(db_error)
	if code != utils.StatusServiceUnavailable {
		return
	}

	if err := ReconnectDb(); err != nil {
		log.Error("cannot reconnect to database at : " + settings.GetDatabaseServer() + " " + err.Error())
	} else {
		log.Debug("reconnected to database" + settings.GetDatabaseServer())
	}
}

func processRequestInDb(request database.Request) (answer []byte, code int) {
	statistics.IncreaseCounter()
	answer, err := db.ProcessRequest(request)
	log_str := "processing request " + request.Op + " in " + request.DbName + " at " + settings.GetDatabaseServer()
	if err != nil {
		go reconnectIfNeeded(err)
		return returnError(err, log_str)
	}
	logger.Debug(log_str)
	return answer, utils.StatusOK
}
