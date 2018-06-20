package server

import (
	"net/http"
	"encoding/json"
)

type authorizationRequest struct {
	BeamtimeId string
	OriginHost string
}

func extractRequest(r *http.Request)(request authorizationRequest,err error) {
	decoder := json.NewDecoder(r.Body)
	err = decoder.Decode(&request)
	return
}

func authorize(request authorizationRequest) bool {
	// todo: implement real check
	if (request.BeamtimeId != "asapo_test") {
		return false
	}

	return true
}

func routeAuthorize(w http.ResponseWriter, r *http.Request) {
	request,err := extractRequest(r)
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		return
	}
	if (!authorize(request)) {
		w.WriteHeader(http.StatusUnauthorized)
		return
	}

	w.WriteHeader(http.StatusOK)
	w.Write([]byte(request.BeamtimeId))
}
