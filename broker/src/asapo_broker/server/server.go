package server

import (
	"asapo_broker/database"
	"asapo_common/utils"
)

var db database.Agent

type serverSettings struct {
	BrokerDbAddress  string
	MonitorDbAddress string
	MonitorDbName    string
	SecretFile       string
	Port             int
	LogLevel         string
}

var settings serverSettings
var statistics serverStatistics
var auth utils.Auth

func InitDB(dbAgent database.Agent) error {
	db = dbAgent
	err := db.Connect(settings.BrokerDbAddress)
	return err
}

func CleanupDB() {
	if db != nil {
		db.Close()
	}
}
