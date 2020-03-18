package request_handler

import "asapo_common/utils"

type Agent interface {
	GetReceivers(bool) ([]byte, error)
	GetBroker() ([]byte, error)
	GetMongo() ([]byte, error)
	GetFts() ([]byte, error)
	Init(settings utils.Settings) error
}

