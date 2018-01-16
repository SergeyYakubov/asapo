package server

import (
	"hidra2_broker/database"
)

type Server struct {
	db database.Agent
}

func (srv *Server) InitDB(db database.Agent) error {
	srv.db = db
	return srv.db.Connect("127.0.0.1:27017")
}

func (srv *Server) CleanupDB() {
	if srv.db != nil {
		srv.db.Close()
	}
}
