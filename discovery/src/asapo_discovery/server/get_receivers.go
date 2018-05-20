package server

import (
	"net/http"
	"asapo_discovery/logger"
)

func getReceivers() (answer []byte, code int) {
	answer, err := requestHandler.GetReceivers()
	log_str := "processing get receivers "
	if err != nil {
		logger.Error(log_str + " - " + err.Error())
		return []byte(err.Error()),http.StatusInternalServerError
	}
	logger.Debug(log_str + " -  got " + string(answer))
	return answer, http.StatusOK
}


func routeGetReceivers(w http.ResponseWriter, r *http.Request) {
	r.Header.Set("Content-type", "application/json")
	answer,code := getReceivers()
	w.WriteHeader(code)
	w.Write(answer)
}

