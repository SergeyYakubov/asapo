package server

import (
	"asapo_discovery/common"
	"asapo_discovery/request_handler"
	"github.com/prometheus/client_golang/prometheus"
)

var requestHandler request_handler.Agent


var settings common.Settings

var (
	nReqests = prometheus.NewCounter(
		prometheus.CounterOpts{
			Name: "http_requests",
			Help: "Number of discovery requests",
		},
	)
)

func SetHandler(rh request_handler.Agent) error {
	requestHandler = rh
	err := requestHandler.Init(settings)
	return err
}


func GetHandlerMode()string {
	return settings.Mode
}

