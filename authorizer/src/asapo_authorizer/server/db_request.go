package server

import (
	"asapo_authorizer/database"
	log "asapo_common/logger"
	"asapo_common/utils"
)

func reconnectIfNeeded(db_error error) {
	code := database.GetStatusCodeFromError(db_error)
	if code != utils.StatusServiceUnavailable {
		return
	}

	if err := ReconnectDb(); err != nil {
		log.Error("cannot reconnect to database at : " + settings.GetDatabaseServer() + " " + err.Error())
	} else {
		log.Debug("reconnected to database" + settings.GetDatabaseServer())
	}
}

func ProcessRequestInDb(request database.Request) ([]byte,error) {
	answer, err := db.ProcessRequest(request)
	if err != nil {
		go reconnectIfNeeded(err)
		return nil,err
	}
	return answer,nil
}
