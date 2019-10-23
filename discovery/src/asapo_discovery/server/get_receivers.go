package server

import (
	"net/http"
	"asapo_common/logger"
	"errors"
)

func getService(service string) (answer []byte, code int) {
	var err error
	switch service {
	case "receivers":
		answer, err = requestHandler.GetReceivers(settings.Receiver.UseIBAddress)
		break
	case "broker":
		answer, err = requestHandler.GetBroker()
		break
	case "mongo":
		answer, err = requestHandler.GetMongo()
		break

	default:
		err = errors.New("wrong request: "+service)
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
	answer,code := getService("receivers")
	w.WriteHeader(code)
	w.Write(answer)
}

func routeGetBroker(w http.ResponseWriter, r *http.Request) {
	r.Header.Set("Content-type", "application/json")
	answer,code := getService("broker")
	w.WriteHeader(code)
	w.Write(answer)
}

func routeGetMongo(w http.ResponseWriter, r *http.Request) {
	r.Header.Set("Content-type", "application/json")
	answer,code := getService("mongo")
	w.WriteHeader(code)
	w.Write(answer)
}
