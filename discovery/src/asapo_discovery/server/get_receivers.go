package server

import (
	"net/http"
	"asapo_common/logger"
	"asapo_discovery/common"
)

func getService(service string) (answer []byte, code int) {
	var err error
	if (service == "asapo-receiver") {
		answer, err = requestHandler.GetReceivers(settings.Receiver.UseIBAddress)
	} else {
		answer, err = requestHandler.GetSingleService(service)

	}
	log_str := "processing get "+service
	if err != nil {
		logger.Error(log_str + " - " + err.Error())
		return []byte(err.Error()),http.StatusInternalServerError
	}
	logger.Debug(log_str + " -  got " + string(answer))
	return answer, http.StatusOK
}


func routeGetReceivers(w http.ResponseWriter, r *http.Request) {
	r.Header.Set("Content-type", "application/json")
	answer,code := getService(common.NameReceiverService)
	w.WriteHeader(code)
	w.Write(answer)
}

func routeGetBroker(w http.ResponseWriter, r *http.Request) {
	r.Header.Set("Content-type", "application/json")
	answer,code := getService(common.NameBrokerService)
	w.WriteHeader(code)
	w.Write(answer)
}

func routeGetMongo(w http.ResponseWriter, r *http.Request) {
	r.Header.Set("Content-type", "application/json")
	answer,code := getService(common.NameMongoService)
	w.WriteHeader(code)
	w.Write(answer)
}

func routeGetFileTransferService(w http.ResponseWriter, r *http.Request) {
	r.Header.Set("Content-type", "application/json")
	answer,code := getService(common.NameFtsService)
	w.WriteHeader(code)
	w.Write(answer)
}