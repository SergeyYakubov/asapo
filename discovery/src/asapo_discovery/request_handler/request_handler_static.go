package request_handler

import (
	"asapo_common/utils"
	"errors"
	"asapo_discovery/common"
)

type StaticRequestHandler struct {
	receiverResponce Responce
	singleServices map[string]string
}

func (rh *StaticRequestHandler) GetSingleService(service string) ([]byte, error) {
	uri,ok := rh.singleServices[service]
	if !ok {
		return nil, errors.New("wrong service: " + service)
	}
	return  []byte(uri),nil
}

func (rh *StaticRequestHandler) GetReceivers(bool) ([]byte, error) {
	return utils.MapToJson(&rh.receiverResponce)
}

func (rh *StaticRequestHandler) Init(settings common.Settings) error {
	rh.receiverResponce.MaxConnections = settings.Receiver.MaxConnections
	rh.receiverResponce.Uris = settings.Receiver.StaticEndpoints
	rh.singleServices = make(map[string]string)
	rh.singleServices[common.NameBrokerService] = settings.Broker.StaticEndpoint
	rh.singleServices[common.NameMongoService] = settings.Mongo.StaticEndpoint
	rh.singleServices[common.NameFtsService] = settings.FileTransferService.StaticEndpoint
	return nil
}
