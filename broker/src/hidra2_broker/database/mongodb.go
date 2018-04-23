//+build !test

package database

import (
	"errors"
	"gopkg.in/mgo.v2"
	"gopkg.in/mgo.v2/bson"
	"hidra2_broker/utils"
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

type Mongodb struct {
	main_session        *mgo.Session
	timeout             time.Duration
	databases           []string
	db_pointers_created map[string]bool
}

func (db *Mongodb) Copy() Agent {
	new_db := new(Mongodb)
	new_db.main_session = db.main_session.Copy()
	new_db.databases = make([]string, len(db.databases))
	copy(new_db.databases, db.databases)
	return new_db
}

func (db *Mongodb) databaseInList(dbname string) bool {
	return utils.StringInSlice(dbname, db.databases)
}

func (db *Mongodb) updateDatabaseList() (err error) {
	db.databases, err = db.main_session.DatabaseNames()
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
	if db.main_session != nil {
		return errors.New(already_connected_msg)
	}

	db.main_session, err = mgo.DialWithTimeout(address, time.Second)
	if err != nil {
		return err
	}

	if err := db.updateDatabaseList(); err != nil {
		return err
	}

	return
}

func (db *Mongodb) Close() {
	if db.main_session != nil {
		db.main_session.Close()
		db.main_session = nil
	}

}

func (db *Mongodb) DeleteAllRecords(dbname string) (err error) {
	if db.main_session == nil {
		return errors.New(no_session_msg)
	}
	return db.main_session.DB(dbname).DropDatabase()
}

func (db *Mongodb) InsertRecord(dbname string, s interface{}) error {
	if db.main_session == nil {
		return errors.New(no_session_msg)
	}

	c := db.main_session.DB(dbname).C(data_collection_name)

	return c.Insert(s)
}

func (db *Mongodb) getMaxIndex(dbname string) (max_id int, err error) {
	c := db.main_session.DB(dbname).C(data_collection_name)
	var id Pointer
	err = c.Find(nil).Sort("-_id").Select(bson.M{"_id": 1}).One(&id)
	if err != nil {
		return 0, nil
	}
	return id.ID, nil
}

func (db *Mongodb) createField(dbname string) (err error) {
	change := mgo.Change{
		Update: bson.M{"$inc": bson.M{pointer_field_name: 0}},
		Upsert: true,
	}
	q := bson.M{"_id": 0}
	c := db.main_session.DB(dbname).C(pointer_collection_name)
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
	q := bson.M{"$and": []bson.M{{"_id": 0}, {pointer_field_name: bson.M{"$lt": max_ind}}}}
	c := db.main_session.DB(dbname).C(pointer_collection_name)
	_, err = c.Find(q).Apply(change, res)
	if err == mgo.ErrNotFound {
		return &DBError{utils.StatusNoData, err.Error()}
	}
	return err
}

func (db *Mongodb) getRecordByID(dbname string, id int) (interface{}, error) {
	var res map[string]interface{}
	q := bson.M{"_id": id}
	c := db.main_session.DB(dbname).C(data_collection_name)
	err := c.Find(q).One(&res)
	if err == mgo.ErrNotFound {
		return nil, &DBError{utils.StatusWrongInput, err.Error()}
	}
	return &res, err
}

func (db *Mongodb) checkDatabaseOperationPrerequisites(db_name string) error {
	if db.main_session == nil {
		return &DBError{utils.StatusError, no_session_msg}
	}

	if err := db.dataBaseExist(db_name); err != nil {
		return &DBError{utils.StatusWrongInput, err.Error()}
	}

	if !db.db_pointers_created[db_name] {
		if db.db_pointers_created == nil {
			db.db_pointers_created = make(map[string]bool)
		}
		db.db_pointers_created[db_name] = true
		db.createField(db_name)
	}

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
		return nil, err
	}

	res, err := db.getRecordByID(db_name, curPointer.Value)
	if err != nil {
		return nil, err
	}

	return utils.MapToJson(&res)
}
