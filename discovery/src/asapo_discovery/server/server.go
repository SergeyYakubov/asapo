package server

import (
	"asapo_discovery/request_handler"
	"errors"
)

var requestHandler request_handler.Agent

type serverSettings struct {
	Endpoints		 []string
	Mode			 string
	Port             int
	MaxConnections   int
	LogLevel         string
}

var settings serverSettings

func SetHandler(rh request_handler.Agent) error {
	requestHandler = rh
	err := requestHandler.Init(settings.MaxConnections,settings.Endpoints)
	return err
}

func (settings *serverSettings) Validate() error {
	if len(settings.Endpoints) == 0 &&  settings.Mode != "consul"{
		return errors.New("Endpoints not set")
	}

	if settings.MaxConnections == 0 {
		return errors.New("Max connections not set")
	}

	if settings.Port == 0 {
		return errors.New("Server port not set")
	}

	if settings.Mode == "" {
		return errors.New("Mode not set")
	}

	if settings.Mode != "static" && settings.Mode != "consul" {
		return errors.New("wrong mode: "  + settings.Mode+ ", (allowed static|consul)")
	}

	return nil
}

func GetHandlerMode()string {
	return settings.Mode
}
