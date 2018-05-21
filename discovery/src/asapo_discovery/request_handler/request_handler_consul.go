package request_handler

import (
	"asapo_discovery/utils"
	"github.com/hashicorp/consul/api"
	"strconv"
	"errors"
)

type ConsulRequestHandler struct {
	MaxConnections int
	client         *api.Client
}

type Responce struct {
	MaxConnections int
	Uris           []string
}

func (rh *ConsulRequestHandler) GetReceivers() ([]byte, error) {
	if (rh.client == nil){
		return nil,errors.New("consul client not connected")
	}
	var responce Responce
	services,_,err := rh.client.Health().Service("receiver","",true,nil)
	if err!=nil {
		return nil,err
	}
	for _,service := range (services) {
		responce.Uris = append(responce.Uris,service.Node.Address+":"+strconv.Itoa(service.Service.Port))
	}
	responce.MaxConnections = rh.MaxConnections
	return utils.MapToJson(&responce)
}

func (rh *ConsulRequestHandler) connectClient(uri string) (client *api.Client, err error) {
	config := api.DefaultConfig()
	if len(uri) > 0 {
		config.Address = uri
	}
	client, err = api.NewClient(config)
	if err == nil {
		_, err = client.Agent().Self()
	}
	if err != nil {
		client = nil
	}
	return
}

func (rh *ConsulRequestHandler) Init(maxCons int, uris []string) (err error) {
	rh.MaxConnections = maxCons
	if len(uris) == 0 {
		rh.client, err = rh.connectClient("")
		return err
	}
	for _, uri := range (uris) {
		rh.client, err = rh.connectClient(uri)
		if err == nil {
			return nil
		}
	}
	return err
}
