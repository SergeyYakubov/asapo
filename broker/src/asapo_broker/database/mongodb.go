//+build !test

package database

import (
	"asapo_common/logger"
	"asapo_common/utils"
	"context"
	"encoding/json"
	"errors"
	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
	"sort"
	"strconv"
	"strings"
	"sync"
	"time"
)

type ID struct {
	ID int `bson:"_id"`
}

type ServiceRecord struct {
	ID   int                    `json:"_id"`
	Name string                 `json:"name"`
	Meta map[string]interface{} `json:"meta"`
}

type SubstreamsRecord struct {
	Substreams []string `bson:"substreams" json:"substreams"`
}

type LocationPointer struct {
	GroupID string `bson:"_id"`
	Value   int    `bson:"current_pointer"`
}

const data_collection_name_prefix = "data_"
const meta_collection_name = "meta"
const pointer_collection_name = "current_location"
const pointer_field_name = "current_pointer"
const no_session_msg = "database client not created"
const wrong_id_type = "wrong id type"
const already_connected_msg = "already connected"

const finish_substream_keyword = "asapo_finish_substream"
const no_next_substream_keyword = "asapo_no_next"

var dbListLock sync.RWMutex
var dbPointersLock sync.RWMutex
var dbSessionLock sync.RWMutex

type SizeRecord struct {
	Size int `bson:"size" json:"size"`
}

type Mongodb struct {
	client              *mongo.Client
	timeout             time.Duration
	databases           []string
	parent_db           *Mongodb
	db_pointers_created map[string]bool
}

func (db *Mongodb) databaseInList(dbname string) bool {
	dbListLock.RLock()
	defer dbListLock.RUnlock()
	return utils.StringInSlice(dbname, db.databases)
}

func (db *Mongodb) updateDatabaseList() (err error) {
	dbListLock.Lock()
	ctx, cancel := context.WithTimeout(context.Background(), 2*time.Second)
	defer cancel()
	db.databases, err = db.client.ListDatabaseNames(ctx, bson.M{})
	dbListLock.Unlock()
	return err
}

func (db *Mongodb) Ping() (err error) {
	if db.client == nil {
		return &DBError{utils.StatusServiceUnavailable, no_session_msg}
	}
	ctx, cancel := context.WithTimeout(context.Background(), 2*time.Second)
	defer cancel()
	return db.client.Ping(ctx, nil)
}

func (db *Mongodb) dataBaseExist(dbname string) (err error) {
	if db.databaseInList(dbname) {
		return nil
	}

	if err := db.updateDatabaseList(); err != nil {
		return err
	}

	if !db.databaseInList(dbname) {
		return &DBError{utils.StatusWrongInput, "stream not found: " + dbname}
	}

	return nil
}

func (db *Mongodb) Connect(address string) (err error) {
	if db.client != nil {
		return &DBError{utils.StatusServiceUnavailable, already_connected_msg}
	}

	db.client, err = mongo.NewClient(options.Client().SetConnectTimeout(20 * time.Second).ApplyURI("mongodb://" + address))
	if err != nil {
		return err
	}
	ctx, cancel := context.WithTimeout(context.Background(), 20*time.Second)
	defer cancel()
	err = db.client.Connect(ctx)
	if err != nil {
		db.client = nil
		return err
	}

	//	db.client.SetSafe(&mgo.Safe{J: true})

	if err := db.updateDatabaseList(); err != nil {
		db.Close()
		return err
	}

	return
}

func (db *Mongodb) Close() {
	if db.client != nil {
		dbSessionLock.Lock()
		ctx, cancel := context.WithTimeout(context.Background(), 2*time.Second)
		defer cancel()
		db.client.Disconnect(ctx)
		db.client = nil
		dbSessionLock.Unlock()
	}
}

func (db *Mongodb) deleteAllRecords(dbname string) (err error) {
	if db.client == nil {
		return &DBError{utils.StatusServiceUnavailable, no_session_msg}
	}
	return db.client.Database(dbname).Drop(context.TODO())
}

func (db *Mongodb) insertRecord(dbname string, collection_name string, s interface{}) error {
	if db.client == nil {
		return &DBError{utils.StatusServiceUnavailable, no_session_msg}
	}

	c := db.client.Database(dbname).Collection(data_collection_name_prefix + collection_name)

	_, err := c.InsertOne(context.TODO(), s)
	return err
}

func (db *Mongodb) insertMeta(dbname string, s interface{}) error {
	if db.client == nil {
		return &DBError{utils.StatusServiceUnavailable, no_session_msg}
	}

	c := db.client.Database(dbname).Collection(meta_collection_name)

	_, err := c.InsertOne(context.TODO(), s)
	return err
}

func (db *Mongodb) getMaxIndex(dbname string, collection_name string, dataset bool) (max_id int, err error) {
	c := db.client.Database(dbname).Collection(data_collection_name_prefix + collection_name)
	var q bson.M
	if dataset {
		q = bson.M{"$expr": bson.M{"$eq": []interface{}{"$size", bson.M{"$size": "$images"}}}}
	} else {
		q = nil
	}
	opts := options.FindOne().SetSort(bson.M{"_id": -1}).SetReturnKey(true)
	var result ID
	err = c.FindOne(context.TODO(), q, opts).Decode(&result)
	if err == mongo.ErrNoDocuments {
		return 0, nil
	}

	return result.ID, err
}

func (db *Mongodb) createLocationPointers(dbname string, collection_name string, group_id string) (err error) {
	opts := options.Update().SetUpsert(true)
	update := bson.M{"$inc": bson.M{pointer_field_name: 0}}
	q := bson.M{"_id": group_id + "_" + collection_name}
	c := db.client.Database(dbname).Collection(pointer_collection_name)
	_, err = c.UpdateOne(context.TODO(), q, update, opts)
	return
}

func (db *Mongodb) setCounter(dbname string, collection_name string, group_id string, ind int) (err error) {
	update := bson.M{"$set": bson.M{pointer_field_name: ind}}
	c := db.client.Database(dbname).Collection(pointer_collection_name)
	q := bson.M{"_id": group_id + "_" + collection_name}
	_, err = c.UpdateOne(context.TODO(), q, update, options.Update())
	return
}

func (db *Mongodb) incrementField(dbname string, collection_name string, group_id string, max_ind int, res interface{}) (err error) {
	update := bson.M{"$inc": bson.M{pointer_field_name: 1}}
	opts := options.FindOneAndUpdate().SetUpsert(false).SetReturnDocument(options.After)
	q := bson.M{"_id": group_id + "_" + collection_name, pointer_field_name: bson.M{"$lt": max_ind}}
	c := db.client.Database(dbname).Collection(pointer_collection_name)

	err = c.FindOneAndUpdate(context.TODO(), q, update, opts).Decode(res)
	if err != nil {
		if err == mongo.ErrNoDocuments {
			return &DBError{utils.StatusNoData, encodeAnswer(max_ind, max_ind, "")}
		}
		return &DBError{utils.StatusTransactionInterrupted, err.Error()}
	}

	return nil
}

func encodeAnswer(id, id_max int, next_substream string) string {
	var r = struct {
		Op             string `json:"op"`
		Id             int    `json:"id"`
		Id_max         int    `json:"id_max"`
		Next_substream string `json:"next_substream"`
	}{"get_record_by_id", id, id_max, next_substream}
	answer, _ := json.Marshal(&r)
	return string(answer)
}

func (db *Mongodb) getRecordByIDRow(dbname string, collection_name string, id, id_max int, dataset bool) ([]byte, error) {
	var res map[string]interface{}
	var q bson.M
	if dataset {
		q = bson.M{"$and": []bson.M{bson.M{"_id": id}, bson.M{"$expr": bson.M{"$eq": []interface{}{"$size", bson.M{"$size": "$images"}}}}}}
	} else {
		q = bson.M{"_id": id}
	}

	c := db.client.Database(dbname).Collection(data_collection_name_prefix + collection_name)
	err := c.FindOne(context.TODO(), q, options.FindOne()).Decode(&res)
	if err != nil {
		answer := encodeAnswer(id, id_max, "")
		log_str := "error getting record id " + strconv.Itoa(id) + " for " + dbname + " : " + err.Error()
		logger.Debug(log_str)
		return nil, &DBError{utils.StatusNoData, answer}
	}
	log_str := "got record id " + strconv.Itoa(id) + " for " + dbname
	logger.Debug(log_str)
	return utils.MapToJson(&res)
}

func (db *Mongodb) getRecordByID(dbname string, collection_name string, group_id string, id_str string, dataset bool) ([]byte, error) {
	id, err := strconv.Atoi(id_str)
	if err != nil {
		return nil, &DBError{utils.StatusWrongInput, err.Error()}
	}

	max_ind, err := db.getMaxIndex(dbname, collection_name, dataset)
	if err != nil {
		return nil, err
	}

	return db.getRecordByIDRow(dbname, collection_name, id, max_ind, dataset)

}

func (db *Mongodb) needCreateLocationPointersInDb(collection_name string, group_id string) bool {
	dbPointersLock.RLock()
	needCreate := !db.db_pointers_created[group_id+"_"+collection_name]
	dbPointersLock.RUnlock()
	return needCreate
}

func (db *Mongodb) setLocationPointersCreateFlag(collection_name string, group_id string) {
	dbPointersLock.Lock()
	if db.db_pointers_created == nil {
		db.db_pointers_created = make(map[string]bool)
	}
	db.db_pointers_created[group_id+"_"+collection_name] = true
	dbPointersLock.Unlock()
}

func (db *Mongodb) generateLocationPointersInDbIfNeeded(db_name string, collection_name string, group_id string) {
	if db.needCreateLocationPointersInDb(collection_name, group_id) {
		db.createLocationPointers(db_name, collection_name, group_id)
		db.setLocationPointersCreateFlag(collection_name, group_id)
	}
}

func (db *Mongodb) getParentDB() *Mongodb {
	if db.parent_db == nil {
		return db
	} else {
		return db.parent_db
	}
}

func (db *Mongodb) checkDatabaseOperationPrerequisites(db_name string, collection_name string, group_id string) error {
	if db.client == nil {
		return &DBError{utils.StatusServiceUnavailable, no_session_msg}
	}

	if err := db.getParentDB().dataBaseExist(db_name); err != nil {
		return err
	}

	if len(group_id) > 0 {
		db.getParentDB().generateLocationPointersInDbIfNeeded(db_name, collection_name, group_id)
	}
	return nil
}

func (db *Mongodb) getCurrentPointer(db_name string, collection_name string, group_id string, dataset bool) (LocationPointer, int, error) {
	max_ind, err := db.getMaxIndex(db_name, collection_name, dataset)
	if err != nil {
		return LocationPointer{}, 0, err
	}

	var curPointer LocationPointer
	err = db.incrementField(db_name, collection_name, group_id, max_ind, &curPointer)
	if err != nil {
		return LocationPointer{}, 0, err
	}

	return curPointer, max_ind, nil
}

func processLastRecord(data []byte, collection_name string, err error) ([]byte, error) {
	var r ServiceRecord
	err = json.Unmarshal(data, &r)
	if err != nil || r.Name != finish_substream_keyword {
		return data, err
	}
	var next_substream string
	next_substream, ok := r.Meta["next_substream"].(string)
	if !ok {
		next_substream = no_next_substream_keyword
	}

	answer := encodeAnswer(r.ID, r.ID, next_substream)
	log_str := "reached end of substream " + collection_name + " , next_substream: " + next_substream
	logger.Debug(log_str)
	return nil, &DBError{utils.StatusNoData, answer}
}

func (db *Mongodb) getNextRecord(db_name string, collection_name string, group_id string, dataset bool) ([]byte, error) {
	curPointer, max_ind, err := db.getCurrentPointer(db_name, collection_name, group_id, dataset)
	if err != nil {
		log_str := "error getting next pointer for " + db_name + ", groupid: " + group_id + ":" + err.Error()
		logger.Debug(log_str)
		return nil, err
	}
	log_str := "got next pointer " + strconv.Itoa(curPointer.Value) + " for " + db_name + ", groupid: " + group_id
	logger.Debug(log_str)
	data, err := db.getRecordByIDRow(db_name, collection_name, curPointer.Value, max_ind, dataset)
	if curPointer.Value != max_ind {
		return data, err
	}
	return processLastRecord(data, collection_name, err)
}

func (db *Mongodb) getLastRecord(db_name string, collection_name string, group_id string, dataset bool) ([]byte, error) {
	max_ind, err := db.getMaxIndex(db_name, collection_name, dataset)
	if err != nil {
		return nil, err
	}
	res, err := db.getRecordByIDRow(db_name, collection_name, max_ind, max_ind, dataset)

	db.setCounter(db_name, collection_name, group_id, max_ind)

	return res, err
}

func (db *Mongodb) getSize(db_name string, collection_name string) ([]byte, error) {
	c := db.client.Database(db_name).Collection(data_collection_name_prefix + collection_name)
	var rec SizeRecord
	var err error

	size, err := c.CountDocuments(context.TODO(), bson.M{}, options.Count())
	if err != nil {
		return nil, err
	}
	rec.Size = int(size)
	return json.Marshal(&rec)
}

func (db *Mongodb) resetCounter(db_name string, collection_name string, group_id string, id_str string) ([]byte, error) {
	id, err := strconv.Atoi(id_str)
	if err != nil {
		return nil, err
	}

	err = db.setCounter(db_name, collection_name, group_id, id)

	return []byte(""), err
}

func (db *Mongodb) getMeta(dbname string, id_str string) ([]byte, error) {
	id, err := strconv.Atoi(id_str)
	if err != nil {
		return nil, err
	}

	var res map[string]interface{}
	q := bson.M{"_id": id}
	c := db.client.Database(dbname).Collection(meta_collection_name)
	err = c.FindOne(context.TODO(), q, options.FindOne()).Decode(&res)
	if err != nil {
		log_str := "error getting meta with id " + strconv.Itoa(id) + " for " + dbname + " : " + err.Error()
		logger.Debug(log_str)
		return nil, &DBError{utils.StatusNoData, err.Error()}
	}
	log_str := "got record id " + strconv.Itoa(id) + " for " + dbname
	logger.Debug(log_str)
	return utils.MapToJson(&res)
}

func (db *Mongodb) processQueryError(query, dbname string, err error) ([]byte, error) {
	log_str := "error processing query: " + query + " for " + dbname + " : " + err.Error()
	logger.Debug(log_str)
	return nil, &DBError{utils.StatusNoData, err.Error()}
}

func (db *Mongodb) queryImages(dbname string, collection_name string, query string) ([]byte, error) {
	var res []map[string]interface{}
	q, sort, err := db.BSONFromSQL(dbname, query)
	if err != nil {
		log_str := "error parsing query: " + query + " for " + dbname + " : " + err.Error()
		logger.Debug(log_str)
		return nil, &DBError{utils.StatusWrongInput, err.Error()}
	}

	c := db.client.Database(dbname).Collection(data_collection_name_prefix + collection_name)
	opts := options.Find()

	if len(sort) > 0 {
		opts = opts.SetSort(sort)
	} else {
	}

	cursor, err := c.Find(context.TODO(), q, opts)
	if err != nil {
		return db.processQueryError(query, dbname, err)
	}
	err = cursor.All(context.TODO(), &res)
	if err != nil {
		return db.processQueryError(query, dbname, err)
	}

	log_str := "processed query " + query + " for " + dbname + " ,found" + strconv.Itoa(len(res)) + " records"
	logger.Debug(log_str)
	if res != nil {
		return utils.MapToJson(&res)
	} else {
		return []byte("[]"), nil
	}
}

func (db *Mongodb) getSubstreams(db_name string) ([]byte, error) {
	database := db.client.Database(db_name)

	result, err := database.ListCollectionNames(context.TODO(), bson.D{})
	if err != nil {
		return db.processQueryError("get substreams", db_name, err)
	}

	var rec SubstreamsRecord
	for _, coll := range result {
		if strings.HasPrefix(coll, data_collection_name_prefix) {
			rec.Substreams = append(rec.Substreams, strings.TrimPrefix(coll, data_collection_name_prefix))
		}
	}
	sort.Strings(rec.Substreams)
	return json.Marshal(&rec)
}

func (db *Mongodb) ProcessRequest(db_name string, collection_name string, group_id string, op string, extra_param string) (answer []byte, err error) {
	dataset := false
	if strings.HasSuffix(op, "_dataset") {
		dataset = true
		op = op[:len(op)-8]
	}

	if err := db.checkDatabaseOperationPrerequisites(db_name, collection_name, group_id); err != nil {
		return nil, err
	}

	switch op {
	case "next":
		return db.getNextRecord(db_name, collection_name, group_id, dataset)
	case "id":
		return db.getRecordByID(db_name, collection_name, group_id, extra_param, dataset)
	case "last":
		return db.getLastRecord(db_name, collection_name, group_id, dataset)
	case "resetcounter":
		return db.resetCounter(db_name, collection_name, group_id, extra_param)
	case "size":
		return db.getSize(db_name, collection_name)
	case "meta":
		return db.getMeta(db_name, extra_param)
	case "queryimages":
		return db.queryImages(db_name, collection_name, extra_param)
	case "substreams":
		return db.getSubstreams(db_name)
	}

	return nil, errors.New("Wrong db operation: " + op)
}
