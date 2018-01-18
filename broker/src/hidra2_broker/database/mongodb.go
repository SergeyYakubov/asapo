//+build !test

package database

import (
	"encoding/json"
	"errors"
	"fmt"
	"gopkg.in/mgo.v2"
	"gopkg.in/mgo.v2/bson"
	"hidra2_broker/utils"
	"time"
)

type Pointer struct {
	ID    int `bson:"_id"`
	Value int `bson:"current_pointer"`
}

type Mongodb struct {
	main_session *mgo.Session
	timeout      time.Duration
	databases    []string
}

func (db *Mongodb) dataBaseExist(dbname string) (err error) {
	if db.main_session == nil {
		return errors.New("database session not created")
	}

	if utils.StringInSlice(dbname, db.databases) {
		return nil
	}

	db.databases, err = db.main_session.DatabaseNames()
	if err != nil {
		return err
	}

	if !utils.StringInSlice(dbname, db.databases) {
		return errors.New(dbname + " not found")
	}

	return nil
}

func (db *Mongodb) Connect(address string) error {
	var err error
	if db.main_session != nil {
		return errors.New("already connected")
	}

	db.main_session, err = mgo.DialWithTimeout(address, time.Second)
	return err
}

func (db *Mongodb) Close() {
	if db.main_session != nil {
		db.main_session.Close()
		db.main_session = nil
	}

}

func (db *Mongodb) DeleteAllRecords(dbname string) (err error) {
	if db.main_session == nil {
		return errors.New("database session not created")
	}

	err1 := db.main_session.DB(dbname).C("data").DropCollection()
	err2 := db.main_session.DB(dbname).C("group_ids").DropCollection()
	if err1 == nil && err2 == nil {
		return nil
	}
	return fmt.Errorf("Combined error: %v %v", err1, err2)
}

func (db *Mongodb) InsertRecord(dbname string, s interface{}) error {
	if db.main_session == nil {
		return errors.New("database session not created")
	}

	c := db.main_session.DB(dbname).C("data")

	return c.Insert(s)
}

func (db *Mongodb) IncrementField(dbname string, res interface{}) (err error) {
	if db.main_session == nil {
		return errors.New("database session not created")
	}

	change := mgo.Change{
		Update:    bson.M{"$inc": bson.M{"current_pointer": 1}},
		Upsert:    true,
		ReturnNew: true,
	}
	q := bson.M{"_id": 0}
	c := db.main_session.DB(dbname).C("group_ids")
	_, err = c.Find(q).Apply(change, res)

	return err
}

func (db *Mongodb) getRecordByID(dbname string, id int, res interface{}) error {
	if db.main_session == nil {
		return errors.New("database session not created")
	}

	q := bson.M{"_id": id}

	c := db.main_session.DB(dbname).C("data")

	return c.Find(q).One(res)

}

func (db *Mongodb) GetNextRecord(db_name string) (answer []byte, code int) {
	if db.main_session == nil {
		return []byte("database session not created"), utils.StatusError
	}

	if err := db.dataBaseExist(db_name); err != nil {
		return []byte(err.Error()), utils.StatusWrongInput
	}

	var curPointer Pointer
	err := db.IncrementField(db_name, &curPointer)
	if err != nil {
		return []byte(err.Error()), utils.StatusError
	}

	var res map[string]interface{}

	err = db.getRecordByID(db_name, curPointer.Value, &res)
	if err == mgo.ErrNotFound {
		return []byte(err.Error()), utils.StatusNoData

	}

	if err == nil {
		answer, errm := json.Marshal(res)
		if errm == nil {
			return answer, utils.StatusOK
		} else {
			return []byte(err.Error()), utils.StatusError
		}
	}

	return []byte(err.Error()), utils.StatusError

}
