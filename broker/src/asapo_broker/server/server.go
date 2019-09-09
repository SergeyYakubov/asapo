package server

import (
	"asapo_broker/database"
	log "asapo_common/logger"
	"asapo_common/utils"
	"errors"
	"io/ioutil"
	"net/http"
)

var db database.Agent

type serverSettings struct {
	DiscoveryServer  string
	DatabaseServer  string
	PerformanceDbServer string
	PerformanceDbName    string
	SecretFile       string
	Port             int
	LogLevel         string
}

var settings serverSettings
var statistics serverStatistics
var auth utils.Auth

type discoveryAPI struct {
	Client  *http.Client
	baseURL string
}

var discoveryService discoveryAPI

func (api *discoveryAPI) GetMongoDbAddress() (string, error) {
	resp, err := api.Client.Get(api.baseURL + "/mongo")
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	return string(body), err
}

func InitDB(dbAgent database.Agent) (err error) {
	db = dbAgent
	if settings.DatabaseServer == "auto" {
		settings.DatabaseServer, err = discoveryService.GetMongoDbAddress()
		if err != nil {
			return err
		}
		if settings.DatabaseServer == "" {
			return errors.New("no database servers found")
		}
		log.Debug("Got mongodb server: " + settings.DatabaseServer)
	}

	err = db.Connect(settings.DatabaseServer)
	return err
}

func CreateDiscoveryService() {
	discoveryService = discoveryAPI{&http.Client{}, "http://" + settings.DiscoveryServer}
}

func CleanupDB() {
	if db != nil {
		db.Close()
	}
}
