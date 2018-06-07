package request_handler

import (
	"asapo_discovery/utils"
	"github.com/hashicorp/consul/api"
	"strconv"
	"errors"
	"sort"
	"sync"
)

type ConsulRequestHandler struct {
	MaxConnections int
	client         *api.Client
}

type Responce struct {
	MaxConnections int
	Uris           []string
}

type SafeCounter struct {
	counter   int
	mux sync.Mutex
}

func (c *SafeCounter) Next(size int) int {
	c.mux.Lock()
	defer c.mux.Unlock()
	val  := c.counter % size
	c.counter++
	return val
}

var counter SafeCounter

func (rh *ConsulRequestHandler) GetServices(name string) ([]string, error) {
	var result = make([]string, 0)
	services, _, err := rh.client.Health().Service(name, "", true, nil)
	if err != nil {
		return nil, err
	}
	for _, service := range (services) {
		result = append(result, service.Node.Address+":"+strconv.Itoa(service.Service.Port))
	}
	sort.Strings(result)
	return result, nil
}

func (rh *ConsulRequestHandler) GetReceivers() ([]byte, error) {
	if (rh.client == nil) {
		return nil, errors.New("consul client not connected")
	}
	var response Responce
	var err error
	response.Uris, err = rh.GetServices("receiver")
	if err != nil {
		return nil, err
	}
	response.MaxConnections = rh.MaxConnections
	return utils.MapToJson(&response)
}

func (rh *ConsulRequestHandler) GetBroker() ([]byte, error) {
	if (rh.client == nil) {
		return nil, errors.New("consul client not connected")
	}
	response, err := rh.GetServices("broker")
	if err != nil {
		return nil, err
	}
	size := len(response)
	if size ==0 {
		return []byte(""),nil
	}else {
		return []byte(response[counter.Next(size)]),nil
	}
	return nil, nil
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

func (rh *ConsulRequestHandler) Init(settings utils.Settings) (err error) {
	rh.MaxConnections = settings.Receiver.MaxConnections
	if len(settings.ConsulEndpoints) == 0 {
		rh.client, err = rh.connectClient("")
		return err
	}
	for _, uri := range (settings.ConsulEndpoints) {
		rh.client, err = rh.connectClient(uri)
		if err == nil {
			return nil
		}
	}
	return err
}
