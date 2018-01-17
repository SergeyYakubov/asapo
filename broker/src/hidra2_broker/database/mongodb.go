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

func (db *Mongodb) Connect(address string) error {
	var err error
	db.main_session, err = mgo.DialWithTimeout(address, time.Second)
	return err
}

func (db *Mongodb) Close() {
	if db.main_session != nil {
		db.main_session.Close()
	}

}

func (db *Mongodb) GetNextRecord(db_name string, collection_name string) (answer []byte, ok bool) {
	return nil, true
}
