package server

import (
    "asapo_common/utils"
	"io/ioutil"
	"net/http"
)

type serverSettings struct {
	Port                    int
	LogLevel                string

	DiscoveryServer         string
	MonitorPerformance      bool
	MonitoringServerUrl     string

	SecretFile              string
	key 					string
}

var settings serverSettings
var monitoring brokerMonitoring
var authJWT utils.Auth

type discoveryAPI struct {
	Client  *http.Client
	baseURL string
}

var discoveryService discoveryAPI

func (api *discoveryAPI) GetMonitoringServerUrl() (string, error) {
	resp, err := api.Client.Get(api.baseURL + "/asapo-monitoring")
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	return string(body), err
}

func CreateDiscoveryService() {
	discoveryService = discoveryAPI{&http.Client{}, "http://" + settings.DiscoveryServer}
}
