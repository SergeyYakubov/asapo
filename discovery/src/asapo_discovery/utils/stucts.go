package utils

import "errors"

type ReceiverInfo struct {
	ForceEndpoints	 []string
	MaxConnections   int
}

type BrokerInfo struct {
	ForceEndpoint		 string
}


type Settings struct {
	Receiver 		ReceiverInfo
	Broker 		    BrokerInfo
	ConsulEndpoints []string
	Mode			string
	Port            int
	LogLevel        string
}

func (settings *Settings) Validate() error {
	if settings.Mode != "consul"{
		if len(settings.Receiver.ForceEndpoints) == 0 || len(settings.Broker.ForceEndpoint) == 0 {
		return errors.New("Endpoints not set")
		}
	}

	if settings.Receiver.MaxConnections == 0 {
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
