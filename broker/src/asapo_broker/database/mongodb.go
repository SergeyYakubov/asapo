//+build !test

package database

import (
	"asapo_common/logger"
	"asapo_common/utils"
	"encoding/json"
	"errors"
	"github.com/globalsign/mgo"
	"github.com/globalsign/mgo/bson"
	"strconv"
	"sync"
	"time"
)

type Pointer struct {
	ID    int `bson:"_id"`
	Value int `bson:"current_pointer"`
}

const data_collection_name = "data"
const pointer_collection_name = "current_location"
const pointer_field_name = "current_pointer"
const no_session_msg = "database session not created"
const wrong_id_type = "wrong id type"
const already_connected_msg = "already connected"

var dbListLock sync.RWMutex
var dbPointersLock sync.RWMutex

type Mongodb struct {
	session             *mgo.Session
	timeout             time.Duration
	databases           []string
	parent_db           *Mongodb
	db_pointers_created map[string]bool
}

func (db *Mongodb) Copy() Agent {
	new_db := new(Mongodb)
	new_db.session = db.session.Copy()
	new_db.parent_db = db
	return new_db
}

func (db *Mongodb) databaseInList(dbname string) bool {
	dbListLock.RLock()
	defer dbListLock.RUnlock()
	return utils.StringInSlice(dbname, db.databases)
}

func (db *Mongodb) updateDatabaseList() (err error) {
	dbListLock.Lock()
	db.databases, err = db.session.DatabaseNames()
	dbListLock.Unlock()
	return err
}

func (db *Mongodb) dataBaseExist(dbname string) (err error) {
	if db.databaseInList(dbname) {
		return nil
	}

	if err := db.updateDatabaseList(); err != nil {
		return err
	}

	if !db.databaseInList(dbname) {
		return errors.New("dataset not found: " + dbname)
	}

	return nil
}

func (db *Mongodb) Connect(address string) (err error) {
	if db.session != nil {
		return errors.New(already_connected_msg)
	}

	db.session, err = mgo.DialWithTimeout(address, time.Second)
	if err != nil {
		return err
	}

	//	db.session.SetSafe(&mgo.Safe{J: true})

	if err := db.updateDatabaseList(); err != nil {
		return err
	}

	return
}

func (db *Mongodb) Close() {
	if db.session != nil {
		db.session.Close()
		db.session = nil
	}

}

func (db *Mongodb) DeleteAllRecords(dbname string) (err error) {
	if db.session == nil {
		return errors.New(no_session_msg)
	}
	return db.session.DB(dbname).DropDatabase()
}

func (db *Mongodb) InsertRecord(dbname string, s interface{}) error {
	if db.session == nil {
		return errors.New(no_session_msg)
	}

	c := db.session.DB(dbname).C(data_collection_name)

	return c.Insert(s)
}

func (db *Mongodb) getMaxIndex(dbname string) (max_id int, err error) {
	c := db.session.DB(dbname).C(data_collection_name)
	var id Pointer
	err = c.Find(nil).Sort("-_id").Select(bson.M{"_id": 1}).One(&id)
	if err != nil {
		return 0, nil
	}
	return id.ID, nil
}

func (db *Mongodb) createLocationPointers(dbname string) (err error) {
	change := mgo.Change{
		Update: bson.M{"$inc": bson.M{pointer_field_name: 0}},
		Upsert: true,
	}
	q := bson.M{"_id": 0}
	c := db.session.DB(dbname).C(pointer_collection_name)
	var res map[string]interface{}
	_, err = c.Find(q).Apply(change, &res)
	return err
}

func (db *Mongodb) incrementField(dbname string, max_ind int, res interface{}) (err error) {
	update := bson.M{"$inc": bson.M{pointer_field_name: 1}}
	change := mgo.Change{
		Update:    update,
		Upsert:    false,
		ReturnNew: true,
	}
	q := bson.M{"_id": 0, pointer_field_name: bson.M{"$lt": max_ind}}
	c := db.session.DB(dbname).C(pointer_collection_name)
	_, err = c.Find(q).Apply(change, res)
	if err == mgo.ErrNotFound {
		return &DBError{utils.StatusNoData, err.Error()}
	}
	return err
}

func (db *Mongodb) GetRecordByID(dbname string, id int) ([]byte, error) {
	var res map[string]interface{}
	q := bson.M{"_id": id}
	c := db.session.DB(dbname).C(data_collection_name)
	err := c.Find(q).One(&res)
	if err != nil {
		var r = struct {
			Id int `json:"id""`
		}{id}
		res, _ := json.Marshal(&r)
		log_str := "error getting record id " + strconv.Itoa(id) + " for " + dbname + " : " + err.Error()
		logger.Debug(log_str)
		return nil, &DBError{utils.StatusNoData, string(res)}
	}

	log_str := "got record id " + strconv.Itoa(id) + " for " + dbname
	logger.Debug(log_str)
	return utils.MapToJson(&res)
}

func (db *Mongodb) needCreateLocationPointersInDb(db_name string) bool {
	dbPointersLock.RLock()
	needCreate := !db.db_pointers_created[db_name]
	dbPointersLock.RUnlock()
	return needCreate
}

func (db *Mongodb) SetLocationPointersCreateFlag(db_name string) {
	dbPointersLock.Lock()
	if db.db_pointers_created == nil {
		db.db_pointers_created = make(map[string]bool)
	}
	db.db_pointers_created[db_name] = true
	dbPointersLock.Unlock()
}

func (db *Mongodb) generateLocationPointersInDbIfNeeded(db_name string) {
	if db.needCreateLocationPointersInDb(db_name) {
		db.createLocationPointers(db_name)
		db.SetLocationPointersCreateFlag(db_name)
	}
}

func (db *Mongodb) getParentDB() *Mongodb {
	if db.parent_db == nil {
		return db
	} else {
		return db.parent_db
	}
}

func (db *Mongodb) checkDatabaseOperationPrerequisites(db_name string) error {
	if db.session == nil {
		return &DBError{utils.StatusError, no_session_msg}
	}

	if err := db.getParentDB().dataBaseExist(db_name); err != nil {
		return &DBError{utils.StatusWrongInput, err.Error()}
	}

	db.getParentDB().generateLocationPointersInDbIfNeeded(db_name)

	return nil
}

func (db *Mongodb) getCurrentPointer(db_name string) (Pointer, error) {
	max_ind, err := db.getMaxIndex(db_name)
	if err != nil {
		return Pointer{}, err
	}
	var curPointer Pointer
	err = db.incrementField(db_name, max_ind, &curPointer)
	if err != nil {
		return Pointer{}, err
	}

	return curPointer, nil
}

func (db *Mongodb) GetNextRecord(db_name string) ([]byte, error) {

	if err := db.checkDatabaseOperationPrerequisites(db_name); err != nil {
		return nil, err
	}

	curPointer, err := db.getCurrentPointer(db_name)
	if err != nil {
		log_str := "error getting next pointer for " + db_name + ":" + err.Error()
		logger.Debug(log_str)
		return nil, err
	}
	log_str := "got next pointer " + strconv.Itoa(curPointer.Value) + " for " + db_name
	logger.Debug(log_str)
	return db.GetRecordByID(db_name, curPointer.Value)

}
