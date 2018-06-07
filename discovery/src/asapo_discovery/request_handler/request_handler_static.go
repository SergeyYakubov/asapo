package request_handler

import (
	"asapo_discovery/utils"
)

type StaticRequestHandler struct {
	receiverResponce Responce
	broker string
}


func (rh *StaticRequestHandler) GetReceivers() ([]byte, error) {
	return utils.MapToJson(&rh.receiverResponce)
}

func (rh *StaticRequestHandler) GetBroker() ([]byte, error) {
	return []byte(rh.broker),nil
}


func (rh *StaticRequestHandler) Init(settings utils.Settings) error {
	rh.receiverResponce.MaxConnections = settings.Receiver.MaxConnections
	rh.receiverResponce.Uris = settings.Receiver.StaticEndpoints
	rh.broker = settings.Broker.StaticEndpoint
	return nil
}
