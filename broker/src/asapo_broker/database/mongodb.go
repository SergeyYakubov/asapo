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
const meta_collection_name = "meta"
const pointer_collection_name = "current_location"
const pointer_field_name = "current_pointer"
const no_session_msg = "database session not created"
const wrong_id_type = "wrong id type"
const already_connected_msg = "already connected"

var dbListLock sync.RWMutex
var dbPointersLock sync.RWMutex

type SizeRecord struct {
	Size int `bson:"size" json:"size"`
}

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

func (db *Mongodb) InsertMeta(dbname string, s interface{}) error {
	if db.session == nil {
		return errors.New(no_session_msg)
	}

	c := db.session.DB(dbname).C(meta_collection_name)

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

func (db *Mongodb) createLocationPointers(dbname string, group_id string) (err error) {
	change := mgo.Change{
		Update: bson.M{"$inc": bson.M{pointer_field_name: 0}},
		Upsert: true,
	}
	q := bson.M{"_id": group_id}
	c := db.session.DB(dbname).C(pointer_collection_name)
	var res map[string]interface{}
	_, err = c.Find(q).Apply(change, &res)
	return err
}

func (db *Mongodb) setCounter(dbname string, group_id string, ind int) (err error) {
	update := bson.M{"$set": bson.M{pointer_field_name: ind}}
	c := db.session.DB(dbname).C(pointer_collection_name)
	return c.UpdateId(group_id, update)
}

func (db *Mongodb) incrementField(dbname string, group_id string, max_ind int, res interface{}) (err error) {
	update := bson.M{"$inc": bson.M{pointer_field_name: 1}}
	change := mgo.Change{
		Update:    update,
		Upsert:    false,
		ReturnNew: true,
	}
	q := bson.M{"_id": group_id, pointer_field_name: bson.M{"$lt": max_ind}}
	c := db.session.DB(dbname).C(pointer_collection_name)
	_, err = c.Find(q).Apply(change, res)
	if err == mgo.ErrNotFound {
		return &DBError{utils.StatusNoData, err.Error()}
	}
	return err
}

func (db *Mongodb) GetRecordByIDRow(dbname string, id int, returnID bool) ([]byte, error) {
	var res map[string]interface{}
	q := bson.M{"_id": id}
	c := db.session.DB(dbname).C(data_collection_name)
	err := c.Find(q).One(&res)
	if err != nil {
		var r = struct {
			Id int `json:"id""`
		}{id}
		answer, _ := json.Marshal(&r)
		log_str := "error getting record id " + strconv.Itoa(id) + " for " + dbname + " : " + err.Error()
		logger.Debug(log_str)
		if returnID {
			return nil, &DBError{utils.StatusNoData, string(answer)}
		} else {
			return nil, &DBError{utils.StatusNoData, err.Error()}
		}

	}
	log_str := "got record id " + strconv.Itoa(id) + " for " + dbname
	logger.Debug(log_str)
	return utils.MapToJson(&res)
}

func (db *Mongodb) GetRecordByID(dbname string, group_id string, id_str string, returnID bool, reset bool) ([]byte, error) {
	id, err := strconv.Atoi(id_str)
	if err != nil {
		return nil, err
	}

	if err := db.checkDatabaseOperationPrerequisites(dbname, group_id); err != nil {
		return nil, err
	}
	res, err := db.GetRecordByIDRow(dbname, id, returnID)

	if reset {
		db.setCounter(dbname, group_id, id)
	}

	return res, err
}

func (db *Mongodb) needCreateLocationPointersInDb(group_id string) bool {
	dbPointersLock.RLock()
	needCreate := !db.db_pointers_created[group_id]
	dbPointersLock.RUnlock()
	return needCreate
}

func (db *Mongodb) SetLocationPointersCreateFlag(group_id string) {
	dbPointersLock.Lock()
	if db.db_pointers_created == nil {
		db.db_pointers_created = make(map[string]bool)
	}
	db.db_pointers_created[group_id] = true
	dbPointersLock.Unlock()
}

func (db *Mongodb) generateLocationPointersInDbIfNeeded(db_name string, group_id string) {
	if db.needCreateLocationPointersInDb(group_id) {
		db.createLocationPointers(db_name, group_id)
		db.SetLocationPointersCreateFlag(group_id)
	}
}

func (db *Mongodb) getParentDB() *Mongodb {
	if db.parent_db == nil {
		return db
	} else {
		return db.parent_db
	}
}

func (db *Mongodb) checkDatabaseOperationPrerequisites(db_name string, group_id string) error {
	if db.session == nil {
		return &DBError{utils.StatusError, no_session_msg}
	}

	if err := db.getParentDB().dataBaseExist(db_name); err != nil {
		return &DBError{utils.StatusWrongInput, err.Error()}
	}

	if len(group_id) > 0 {
		db.getParentDB().generateLocationPointersInDbIfNeeded(db_name, group_id)
	}
	return nil
}

func (db *Mongodb) getCurrentPointer(db_name string, group_id string) (Pointer, error) {
	max_ind, err := db.getMaxIndex(db_name)
	if err != nil {
		return Pointer{}, err
	}
	var curPointer Pointer
	err = db.incrementField(db_name, group_id, max_ind, &curPointer)
	if err != nil {
		return Pointer{}, err
	}

	return curPointer, nil
}

func (db *Mongodb) GetNextRecord(db_name string, group_id string) ([]byte, error) {

	if err := db.checkDatabaseOperationPrerequisites(db_name, group_id); err != nil {
		return nil, err
	}

	curPointer, err := db.getCurrentPointer(db_name, group_id)
	if err != nil {
		log_str := "error getting next pointer for " + db_name + ", groupid: " + group_id + ":" + err.Error()
		logger.Debug(log_str)
		return nil, err
	}
	log_str := "got next pointer " + strconv.Itoa(curPointer.Value) + " for " + db_name + ", groupid: " + group_id
	logger.Debug(log_str)
	return db.GetRecordByIDRow(db_name, curPointer.Value, true)

}

func (db *Mongodb) GetLastRecord(db_name string, group_id string) ([]byte, error) {

	if err := db.checkDatabaseOperationPrerequisites(db_name, group_id); err != nil {
		return nil, err
	}

	max_ind, err := db.getMaxIndex(db_name)
	if err != nil {
		log_str := "error getting last pointer for " + db_name + ", groupid: " + group_id + ":" + err.Error()
		logger.Debug(log_str)
		return nil, err
	}
	res, err := db.GetRecordByIDRow(db_name, max_ind, false)

	db.setCounter(db_name, group_id, max_ind)

	return res, err
}

func (db *Mongodb) GetSize(db_name string) ([]byte, error) {

	if err := db.checkDatabaseOperationPrerequisites(db_name, ""); err != nil {
		return nil, err
	}

	c := db.session.DB(db_name).C(data_collection_name)
	var rec SizeRecord
	var err error
	rec.Size, err = c.Count()
	if err != nil {
		return nil, err
	}
	return json.Marshal(&rec)
}

func (db *Mongodb) ResetCounter(db_name string, group_id string) ([]byte, error) {

	if err := db.checkDatabaseOperationPrerequisites(db_name, group_id); err != nil {
		return nil, err
	}

	err := db.setCounter(db_name, group_id, 0)

	return []byte(""), err
}

func (db *Mongodb) getMeta(dbname string, id_str string) ([]byte, error) {
	id, err := strconv.Atoi(id_str)
	if err != nil {
		return nil, err
	}

	var res map[string]interface{}
	q := bson.M{"_id": id}
	c := db.session.DB(dbname).C(meta_collection_name)
	err = c.Find(q).One(&res)
	if err != nil {
		log_str := "error getting meta with id " + strconv.Itoa(id) + " for " + dbname + " : " + err.Error()
		logger.Debug(log_str)
		return nil, &DBError{utils.StatusNoData, err.Error()}
	}

	log_str := "got record id " + strconv.Itoa(id) + " for " + dbname
	logger.Debug(log_str)
	return utils.MapToJson(&res)
}

func (db *Mongodb) ProcessRequest(db_name string, group_id string, op string, extra_param string) (answer []byte, err error) {
	switch op {
	case "next":
		return db.GetNextRecord(db_name, group_id)
	case "id":
		return db.GetRecordByID(db_name, group_id, extra_param, true, false)
	case "idreset":
		return db.GetRecordByID(db_name, group_id, extra_param, true, true)
	case "last":
		return db.GetLastRecord(db_name, group_id)
	case "resetcounter":
		return db.ResetCounter(db_name, group_id)
	case "size":
		return db.GetSize(db_name)
	case "meta":
		return db.getMeta(db_name, extra_param)

	}

	return nil, errors.New("Wrong db operation: " + op)
}
