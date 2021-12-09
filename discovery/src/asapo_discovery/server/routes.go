package server

import (
	"asapo_common/logger"
	"asapo_discovery/common"
	"asapo_discovery/protocols"
	"net/http"
)

func getService(service string) (answer []byte, code int) {
	var err error
	if service == "asapo-receiver" {
		answer, err = requestHandler.GetReceivers(settings.Receiver.UseIBAddress)
	} else {
		answer, err = requestHandler.GetSingleService(service)

	}
	log_str := "processing get " + service + " request"
	if err != nil {
		logger.Error(log_str + " - " + err.Error())
		return []byte(err.Error()), http.StatusInternalServerError
	}
	logger.WithFields(map[string]interface{}{
		"service": service,
		"answer":  string(answer),
	}).Debug("processing get service request")
	return answer, http.StatusOK
}

func validateProtocol(w http.ResponseWriter, r *http.Request, client string) bool {
	protocol, err := extractProtocol(r)
	log_str := "validating " + client + " protocol"
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		w.Write([]byte(err.Error()))
		logger.Error(log_str + " - " + err.Error())
		return false
	}
	if hint, ok := protocols.ValidateProtocol(client, protocol); !ok {
		w.WriteHeader(http.StatusUnsupportedMediaType)
		w.Write([]byte(hint))
		logger.Error(log_str + " - " + hint)
		return false
	}
	return true
}

func routeGetReceivers(w http.ResponseWriter, r *http.Request) {
	if ok := checkDiscoveryApiVersion(w, r); !ok {
		return
	}

	if !validateProtocol(w, r, "producer") {
		return
	}
	answer, code := getService(common.NameReceiverService)
	w.WriteHeader(code)
	w.Write(answer)
}

func routeGetMonitoringServers(w http.ResponseWriter, r *http.Request) {
	answer, code := getService(common.NameMonitoringServer)
	w.WriteHeader(code)
	w.Write(answer)
}

func routeGetBroker(w http.ResponseWriter, r *http.Request) {
	if ok := checkDiscoveryApiVersion(w, r); !ok {
		return
	}

	if !validateProtocol(w, r, "consumer") {
		return
	}

	answer, code := getService(common.NameBrokerService)
	w.WriteHeader(code)
	w.Write(answer)
}

func routeGetMongo(w http.ResponseWriter, r *http.Request) {
	answer, code := getService(common.NameMongoService)
	w.WriteHeader(code)
	w.Write(answer)
}

func routeGetFileTransferService(w http.ResponseWriter, r *http.Request) {
	if ok := checkDiscoveryApiVersion(w, r); !ok {
		return
	}
	if !validateProtocol(w, r, "consumer") {
		return
	}

	answer, code := getService(common.NameFtsService)
	w.WriteHeader(code)
	w.Write(answer)
}
