package server

import (
	log "asapo_common/logger"
	"asapo_common/utils"
	"encoding/json"
	"net/http"
)

func routeRevoke(w http.ResponseWriter, r *http.Request) {
	Auth.AdminAuth().ProcessAuth(processRevoke, "")(w, r)
}

func processRevoke(w http.ResponseWriter, r *http.Request) {
	if checkRole(w, r, "revoke") != nil {
		return
	}
	revokeToken(w, r)
}

func revokeToken(w http.ResponseWriter, r *http.Request) {
	token, err := extractToken(r)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusBadRequest)
		return
	}

	rec, err := store.RevokeToken(token, "")
	if err != nil {
		log.Error("could not revoke token "+ token+": "+ err.Error())
		utils.WriteServerError(w, err, http.StatusBadRequest)
		return
	}

	log.Debug("revoked token " + rec.Token)
	answer, _ := json.Marshal(&rec)
	w.WriteHeader(http.StatusOK)
	w.Write(answer)
}
