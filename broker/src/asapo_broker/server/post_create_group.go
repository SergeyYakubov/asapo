package server

import (
	"asapo_common/logger"
	"github.com/rs/xid"
	"net/http"
)

func routeCreateGroupID(w http.ResponseWriter, r *http.Request) {
	if ok := checkBrokerApiVersion(w, r); !ok {
		return
	}


	guid := xid.New()
	w.Write([]byte(guid.String()))
	logger.Debug("generated new group: " + guid.String())
	statistics.IncreaseCounter()
}
