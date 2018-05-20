package request_handler

import (
	"asapo_discovery/utils"
)

type StaticRequestHandler struct {
	Responce
	}

func (rh *StaticRequestHandler) GetReceivers() ([]byte, error) {
	return utils.MapToJson(&rh)
}

func (rh *StaticRequestHandler) Init(maxCons int,uris []string) error {
	rh.MaxConnections = maxCons
	rh.Uris = uris
	return nil
}
