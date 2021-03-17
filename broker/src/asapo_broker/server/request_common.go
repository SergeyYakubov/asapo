package server

import (
	"asapo_common/logger"
	"asapo_common/utils"
	"errors"
	"net/http"
	"strconv"
)

func writeAuthAnswer(w http.ResponseWriter, requestName string, db_name string, err error) {
	log_str := "processing " + requestName + " request in " + db_name + " at " + settings.GetDatabaseServer()
	logger.Error(log_str + " - " + err.Error())

	httpError, ok := err.(*HttpError)
	if ok && httpError.statusCode != http.StatusUnauthorized {
		w.WriteHeader(http.StatusInternalServerError)
	} else {
		w.WriteHeader(http.StatusUnauthorized)
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

func authorize(r *http.Request, beamtime_id string) error {
	tokenJWT := r.URL.Query().Get("token")

	if len(tokenJWT) == 0 {
		return errors.New("cannot extract token from request")
	}

	token, err := auth.AuthorizeToken(tokenJWT)
	if err != nil {
		return err
	}

	err = checkSubject(token.Sub, beamtime_id)
	if err != nil {
		return err
	}

	return checkAccessType(token.AccessTypes)
}

func checkSubject(subject string, beamtime_id string) error {
	if subject != utils.SubjectFromBeamtime(beamtime_id) {
		return errors.New("wrong token subject")
	}
	return nil
}

func checkAccessType(accessTypes []string) error {
	if !utils.StringInSlice("read",accessTypes) {
		return errors.New("wrong token access type")
	}
	return nil
}
