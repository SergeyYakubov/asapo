package server

import (
	log "asapo_common/logger"
	"asapo_common/structs"
	"asapo_common/utils"
	"encoding/json"
	"net/http"
)

func extractToken(r *http.Request) (string, error) {
	var request structs.IntrospectTokenRequest
	err := utils.ExtractRequest(r, &request)
	if err != nil {
		return "", err
	}
	return request.Token, nil
}

func verifyUserToken(token string) (response structs.IntrospectTokenResponse, err error) {
	var extra_claim structs.AccessTokenExtraClaim
	claim,err := Auth.UserAuth().CheckAndGetContent(token,&extra_claim)
	if err!=nil {
		return
	}
	err = checkTokenRevoked(claim.Id)
	if err != nil {
		return
	}

	response.Sub = claim.Subject
	response.AccessTypes = extra_claim.AccessTypes
	return
}

func routeIntrospect(w http.ResponseWriter, r *http.Request) {
	token, err := extractToken(r)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusBadRequest)
		return
	}

	response,err := verifyUserToken(token)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusUnauthorized)
		return
	}

	log.WithFields(map[string]interface{}{
		"subject":response.Sub,
	}).Debug("verified user token")


	answer,_ := json.Marshal(&response)
	w.WriteHeader(http.StatusOK)
	w.Write(answer)
}
