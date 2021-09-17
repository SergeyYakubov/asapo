package server

import (
	"asapo_authorizer/authorization"
	log "asapo_common/logger"
	"asapo_common/structs"
	"asapo_common/utils"
	"errors"
	"net/http"
)

func extractUserTokenrequest(r *http.Request) (request structs.IssueTokenRequest, err error) {
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

	if request.DaysValid <= 0 {
		return request, errors.New("set token valid period")
	}

	for _, ar := range request.AccessTypes {
		if ar != "read" && ar != "write" && !(ar== "writeraw" && request.Subject["beamline"]!="") {
			return request, errors.New("wrong requested access rights: "+ar)
		}
	}

	return request, nil
}

func routeAuthorisedTokenIssue(w http.ResponseWriter, r *http.Request) {
	Auth.AdminAuth().ProcessAuth(checkAccessToken, "admin")(w, r)
}
func checkAccessToken(w http.ResponseWriter, r *http.Request) {
	var extraClaim structs.AccessTokenExtraClaim
	var claims *utils.CustomClaims
	if err := utils.JobClaimFromContext(r, &claims, &extraClaim); err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(err.Error()))
	}
	if claims.Subject != "admin" || !utils.StringInSlice("create",extraClaim.AccessTypes) {
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

	token, err := Auth.PrepareAccessToken(request, true)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusInternalServerError)
		return
	}

	log.Debug("generated user token ")

	answer := authorization.UserTokenResponce(request, token)
	w.WriteHeader(http.StatusOK)
	w.Write(answer)
}
