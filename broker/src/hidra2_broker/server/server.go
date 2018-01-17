package server

import (
	"hidra2_broker/database"
)

var db database.Agent

func InitDB(dbAgent database.Agent) error {
	db = dbAgent
	return db.Connect("127.0.0.1:27017")
}

func CleanupDB() {
	if db != nil {
		db.Close()
	}
}
