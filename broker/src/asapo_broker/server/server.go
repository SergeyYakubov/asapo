package server

import (
	"asapo_broker/database"
	log "asapo_common/logger"
	"asapo_common/utils"
	"errors"
	"io/ioutil"
	"net/http"
)

const  kDefaultresendInterval = 10
const  kDefaultStreamCacheUpdateIntervalMs = 100

var db database.Agent

type serverSettings struct {
	DiscoveryServer     string
	DatabaseServer      string
	PerformanceDbServer string
	PerformanceDbName   string
	SecretFile          string
	Port                int
	LogLevel            string
	discoveredDbAddress string
	CheckResendInterval *int
	StreamCacheUpdateIntervalMs *int
}

func (s *serverSettings) GetResendInterval() int {
	if s.CheckResendInterval==nil {
		return kDefaultresendInterval
	}
	return *s.CheckResendInterval
}

func (s *serverSettings) GetStreamCacheUpdateInterval() int {
	if s.StreamCacheUpdateIntervalMs==nil {
		return kDefaultStreamCacheUpdateIntervalMs
	}
	return *s.StreamCacheUpdateIntervalMs
}

func (s *serverSettings) GetDatabaseServer() string {
	if s.DatabaseServer == "auto" {
		return s.discoveredDbAddress
	} else {
		return s.DatabaseServer
	}
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
	resp, err := api.Client.Get(api.baseURL + "/asapo-mongodb")
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	return string(body), err
}

func ReconnectDb() (err error) {
	if db == nil {
		return errors.New("database not initialized")
	}
	db.Close()
	return InitDB(db)
}

func InitDB(dbAgent database.Agent) (err error) {
	db = dbAgent
	if settings.DatabaseServer == "auto" {
		settings.discoveredDbAddress, err = discoveryService.GetMongoDbAddress()
		if err != nil {
			return err
		}
		if settings.discoveredDbAddress == "" {
			return errors.New("no database servers found")
		}
		log.Debug("Got mongodb server: " + settings.discoveredDbAddress)
	}

	db.SetSettings(database.DBSettings{ReadFromInprocessPeriod: settings.GetResendInterval(),UpdateStreamCachePeriodMs: settings.GetStreamCacheUpdateInterval()})

	return db.Connect(settings.GetDatabaseServer())
}

func CreateDiscoveryService() {
	discoveryService = discoveryAPI{&http.Client{}, "http://" + settings.DiscoveryServer}
}

func CleanupDB() {
	if db != nil {
		db.Close()
	}
}
