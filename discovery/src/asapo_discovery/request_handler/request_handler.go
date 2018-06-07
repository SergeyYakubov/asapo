package request_handler

import "asapo_discovery/utils"

type Agent interface {
	GetReceivers() ([]byte, error)
	GetBroker() ([]byte, error)
	Init(settings utils.Settings) error
}

