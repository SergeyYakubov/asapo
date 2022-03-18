package server

import (
	"asapo_broker/database"
	"asapo_common/logger"
	log "asapo_common/logger"
	"asapo_common/utils"
	"asapo_common/version"
	"github.com/gorilla/mux"
	"net/http"
	"net/url"
)

func readFromMapUnescaped(key string, vars map[string]string) (val string,ok bool) {
	if val, ok = vars[key];!ok {
		return
	}
	var err error
	if val, err = url.PathUnescape(val);err!=nil {
		return "",false
	}
	return
}

func extractRequestParameters(r *http.Request, needGroupID bool) (string, string, string, string, bool) {
	vars := mux.Vars(r)
	db_name, ok1 := vars["beamtime"]
	datasource, ok3 := readFromMapUnescaped("datasource",vars)
	stream, ok4 := readFromMapUnescaped("stream",vars)

	ok2 := true
	group_id := ""
	if needGroupID {
		group_id, ok2 = readFromMapUnescaped("groupid",vars)
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

func checkBrokerApiVersion(w http.ResponseWriter, r *http.Request) bool {
	_, ok := utils.PrecheckApiVersion(w, r, version.GetBrokerApiVersion())
	return ok
}

func needWriteAccess(op string) bool {
	return op=="delete_stream";
}

func processRequest(w http.ResponseWriter, r *http.Request, op string, extra_param string, needGroupID bool) {
	if ok := checkBrokerApiVersion(w, r); !ok {
		return
	}


	w.Header().Set("Access-Control-Allow-Origin", "*")
	beamtime, datasource, stream, group_id, ok := extractRequestParameters(r, needGroupID)
	if !ok {
		log.WithFields(map[string]interface{}{"request":r.RequestURI}).Error("cannot extract request parameters")
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	if err := authorize(r, beamtime, needWriteAccess(op)); err != nil {
		writeAuthAnswer(w, "get "+op, beamtime, err)
		return
	}

	request := database.Request{}
	request.Beamtime = beamtime
	request.DataSource = datasource
	request.Op = op
	request.ExtraParam = extra_param
	request.Stream = stream
	request.GroupId = group_id
	if yes, minSize := datasetRequested(r); yes {
		request.DatasetOp = true
		request.MinDatasetSize = minSize
	}

	rlog:=request.Logger()
	rlog.Debug("got request")
	answer, code := processRequestInDb(request,rlog)
	w.WriteHeader(code)
	w.Write(answer)
}

func returnError(err error, rlog logger.Logger) (answer []byte, code int) {
	code = database.GetStatusCodeFromError(err)
	if code != utils.StatusNoData && code != utils.StatusPartialData{
		rlog.WithFields(map[string]interface{}{"cause":err.Error()}).Error("cannot process request")
	} else {
		rlog.WithFields(map[string]interface{}{"cause":err.Error()}).Debug("no data or partial data")
	}
	return []byte(err.Error()), code
}

func reconnectIfNeeded(db_error error) {
	code := database.GetStatusCodeFromError(db_error)
	if code != utils.StatusServiceUnavailable {
		return
	}

	if err := ReconnectDb(); err != nil {
		log.WithFields(map[string]interface{}{"address":settings.GetDatabaseServer(),"cause": err.Error()}).Error("cannot reconnect to database")
	} else {
		log.WithFields(map[string]interface{}{"address":settings.GetDatabaseServer()}).Debug("reconnected to database")
	}
}



func processRequestInDb(request database.Request,rlog logger.Logger) (answer []byte, code int) {
	statistics.IncreaseCounter()
	answer, err := db.ProcessRequest(request)
	if err != nil {
		go reconnectIfNeeded(err)
		return returnError(err, rlog)
	}
	return answer, utils.StatusOK
}
