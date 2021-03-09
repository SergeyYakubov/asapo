package server

import (
	"asapo_authorizer/authorization"
	log "asapo_common/logger"
	"asapo_common/utils"
	"errors"
	"net/http"
)


func extractUserTokenrequest(r *http.Request) (request authorization.TokenRequest, err error) {
	err = utils.ExtractRequest(r, &request)
	if err != nil {
		return request, err
	}

	if request.Subject["beamtimeId"] == "" && request.Subject["beamline"] == "" {
		return request, errors.New("missing beamtime/beamline")
	}

	if request.Subject["beamtimeId"] != "" && request.Subject["beamline"] != "" {
		return request, errors.New("set only one of beamtime/beamline")
	}

	if request.AccessType != "read" && request.AccessType != "write" {
		return request, errors.New("wrong access type " + request.AccessType)
	}

	return request, nil
}


func routeAuthorisedTokenIssue(w http.ResponseWriter, r *http.Request) {
	Auth.AdminAuth().ProcessAuth(issueUserToken, "admin")(w, r)
}

func issueUserToken(w http.ResponseWriter, r *http.Request) {
	request, err := extractUserTokenrequest(r)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusBadRequest)
		return
	}

	token, err := Auth.PrepareUserJWTToken(request)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusInternalServerError)
		return
	}

	log.Debug("generated user token ")

	answer := authorization.UserTokenResponce(request, token)
	w.WriteHeader(http.StatusOK)
	w.Write(answer)
}
