package request_handler

import (
	"asapo_discovery/common"
)

type Agent interface {
	GetReceivers(bool) ([]byte, error)
	GetSingleService(service string) ([]byte, error)
	Init(settings common.Settings) error
}

type Responce struct {
	MaxConnections int
	Uris           []string
}