package server

import (
	"asapo_common/logger"
	"errors"
	"net/http"
)

func writeAuthAnswer(w http.ResponseWriter, requestName string, db_name string, err string) {
	log_str := "processing " + requestName + " request in " + db_name + " at " + settings.BrokerDbAddress
	logger.Error(log_str + " - " + err)
	w.WriteHeader(http.StatusUnauthorized)
	w.Write([]byte(err))
}

func resetRequested(r *http.Request) bool {
	val := r.URL.Query().Get("reset")

	if len(val) == 0 {
		return false
	}

	if val == "true" {
		return true
	}

	return false
}

func testAuth(r *http.Request, beamtime_id string) error {
	token_got := r.URL.Query().Get("token")

	if len(token_got) == 0 {
		return errors.New("cannot extract token from request")
	}

	token_expect, _ := auth.GenerateToken(&beamtime_id)

	if token_got != token_expect {
		return errors.New("wrong token")
	}

	return nil
}
