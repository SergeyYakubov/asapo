package server

import (
	"hidra2_broker/database"
)

var db database.Agent

type serverSettings struct {
	BrokerDbAddress  string
	MonitorDbAddress string
	MonitorDbName    string
	Port             int
}

var settings serverSettings
var statistics serverStatistics

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
