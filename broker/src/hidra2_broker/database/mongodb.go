//+build !test

package database

import (
	"gopkg.in/mgo.v2"
	"time"
)

type Mongodb struct {
	main_session *mgo.Session
	name         string
	timeout      time.Duration
}

func (db *Mongodb) Connect(string) error {
	var err error
	return err
}

func (db *Mongodb) Close() {
	if db.main_session != nil {
		db.main_session.Close()
	}

}
