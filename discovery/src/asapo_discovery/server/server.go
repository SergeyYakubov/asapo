package server

import (
	"asapo_discovery/request_handler"
	"asapo_discovery/common"
)

var requestHandler request_handler.Agent


var settings common.Settings

func SetHandler(rh request_handler.Agent) error {
	requestHandler = rh
	err := requestHandler.Init(settings)
	return err
}


func GetHandlerMode()string {
	return settings.Mode
}

