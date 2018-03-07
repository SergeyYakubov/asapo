package server

import (
	"hidra2_broker/database"
)

var db database.Agent

type serverSettings struct {
	DbAddress string
	Port      int
}

var settings serverSettings

func InitDB(dbAgent database.Agent) error {
	db = dbAgent
	return db.Connect(settings.DbAddress)
}

func CleanupDB() {
	if db != nil {
		db.Close()
	}
}
