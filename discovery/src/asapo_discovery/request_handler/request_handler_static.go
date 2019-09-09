package request_handler

import (
	"asapo_common/utils"
)

type StaticRequestHandler struct {
	receiverResponce Responce
	broker string
	mongo string
}


func (rh *StaticRequestHandler) GetReceivers() ([]byte, error) {
	return utils.MapToJson(&rh.receiverResponce)
}

func (rh *StaticRequestHandler) GetBroker() ([]byte, error) {
	return []byte(rh.broker),nil
}

func (rh *StaticRequestHandler) GetMongo() ([]byte, error) {
	return []byte(rh.mongo),nil
}



func (rh *StaticRequestHandler) Init(settings utils.Settings) error {
	rh.receiverResponce.MaxConnections = settings.Receiver.MaxConnections
	rh.receiverResponce.Uris = settings.Receiver.StaticEndpoints
	rh.broker = settings.Broker.StaticEndpoint
	rh.mongo = settings.Mongo.StaticEndpoint

	return nil
}
