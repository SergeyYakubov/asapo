package server

import (
	"asapo_common/logger"
	"asapo_common/utils"
	"errors"
	"net/http"
	"strconv"
)

func writeAuthAnswer(w http.ResponseWriter, requestOp string, db_name string, err error) {
	logger.WithFields(map[string]interface{}{"operation": requestOp, "cause": err.Error()}).Error("cannot authorize request")

	switch er := err.(type) {
	case AuthorizationError:
		w.WriteHeader(er.statusCode)
	default:
		w.WriteHeader(http.StatusServiceUnavailable)
	}
	w.Write([]byte(err.Error()))
}

func valueTrue(r *http.Request, key string) bool {
	val := r.URL.Query().Get(key)
	if len(val) == 0 {
		return false
	}

	if val == "true" {
		return true
	}
	return false
}

func valueInt(r *http.Request, key string) int {
	val := r.URL.Query().Get(key)
	if len(val) == 0 {
		return 0
	}

	i, err := strconv.Atoi(val)
	if err != nil {
		return 0
	}
	return i
}

func datasetRequested(r *http.Request) (bool, int) {
	return valueTrue(r, "dataset"), valueInt(r, "minsize")
}

func authorize(r *http.Request, beamtime_id string, needWriteAccess bool) error {
	tokenJWT := r.URL.Query().Get("token")

	if len(tokenJWT) == 0 {
		return AuthorizationError{errors.New("cannot extract token from request"), http.StatusBadRequest}
	}

	token, err := auth.AuthorizeToken(tokenJWT)
	if err != nil {
		return err
	}

	err = checkSubject(token.Sub, beamtime_id)
	if err != nil {
		return err
	}

	return checkAccessType(token.AccessTypes, needWriteAccess)
}

func checkSubject(subject string, beamtime_id string) error {
	if subject != utils.SubjectFromBeamtime(beamtime_id) {
		return AuthorizationError{errors.New("wrong token subject"), http.StatusUnauthorized}
	}
	return nil
}

func checkAccessType(accessTypes []string, needWriteAccess bool) error {
	if needWriteAccess && !utils.StringInSlice("write", accessTypes) {
		return AuthorizationError{errors.New("wrong token access type"), http.StatusUnauthorized}
	}

	if !utils.StringInSlice("read", accessTypes) {
		return AuthorizationError{errors.New("wrong token access type"), http.StatusUnauthorized}
	}
	return nil
}
