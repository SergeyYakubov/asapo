package common

import "errors"

type ReceiverInfo struct {
	StaticEndpoints	 []string
	MaxConnections   int
	UseIBAddress bool
}

type BrokerInfo struct {
	StaticEndpoint		 string
}

type MongoInfo struct {
	StaticEndpoint		 string
}

type FtsInfo struct {
	StaticEndpoint		 string
}

type Settings struct {
	Receiver 		ReceiverInfo
	Broker 		    BrokerInfo
	Mongo 			MongoInfo
	FileTransferService  FtsInfo
	ConsulEndpoints []string
	Mode			string
	Kubernetes struct {
		Mode string
		ConfigFile string
		Namespace string
	}
	Port            int
	LogLevel        string
}

func (settings *Settings) Validate() error {
	if settings.Mode == "static"{
		if len(settings.Receiver.StaticEndpoints) == 0 || len(settings.Broker.StaticEndpoint) == 0 || len(settings.Mongo.StaticEndpoint) == 0{
			return errors.New("static endpoints not set")
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

	if settings.Mode != "static" && settings.Mode != "consul" && settings.Mode != "kubernetes" {
		return errors.New("wrong mode: "  + settings.Mode+ ", (allowed static|consul|kubernetes)")
	}

	return nil
}
