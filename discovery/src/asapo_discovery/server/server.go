package server

import (
	"asapo_discovery/request_handler"
	"asapo_common/utils"
)

var requestHandler request_handler.Agent


var settings utils.Settings

func SetHandler(rh request_handler.Agent) error {
	requestHandler = rh
	err := requestHandler.Init(settings)
	return err
}


func GetHandlerMode()string {
	return settings.Mode
}
