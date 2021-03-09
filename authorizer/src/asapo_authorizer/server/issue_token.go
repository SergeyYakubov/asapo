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

	if request.DaysValid<=0 {
		return request, errors.New("set token valid period")
	}

	if request.AccessType != "read" && request.AccessType != "write" {
		return request, errors.New("wrong access type " + request.AccessType)
	}

	return request, nil
}

func routeAuthorisedTokenIssue(w http.ResponseWriter, r *http.Request) {
	Auth.AdminAuth().ProcessAuth(checkAccessToken, "admin")(w, r)
}
func checkAccessToken(w http.ResponseWriter, r *http.Request) {
	var extraClaim utils.AccessTokenExtraClaim
	var claims *utils.CustomClaims
	if err := utils.JobClaimFromContext(r, &claims, &extraClaim); err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(err.Error()))
	}
	if claims.Subject != "admin" || extraClaim.AccessType != "create" {
		err_txt := "wrong token claims"
		w.WriteHeader(http.StatusUnauthorized)
		w.Write([]byte(err_txt))
	}

	issueUserToken(w, r)
}

func issueUserToken(w http.ResponseWriter, r *http.Request) {
	request, err := extractUserTokenrequest(r)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusBadRequest)
		return
	}

	token, err := Auth.PrepareAccessToken(request)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusInternalServerError)
		return
	}

	log.Debug("generated user token ")

	answer := authorization.UserTokenResponce(request, token)
	w.WriteHeader(http.StatusOK)
	w.Write(answer)
}
