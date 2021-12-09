//+build !test

package token_store

import (
	"asapo_common/utils"
	"context"
	"errors"
	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/bson/primitive"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
	"strings"
	"sync"
	"time"
)

const no_session_msg = "database client not created"
const already_connected_msg = "already connected"

var dbSessionLock sync.Mutex
var dbClientLock sync.RWMutex

type Mongodb struct {
	client                *mongo.Client
	timeout               time.Duration
	lastReadFromInprocess map[string]int64
}

func (db *Mongodb) Ping() (err error) {
	if db.client == nil {
		return &DBError{utils.StatusServiceUnavailable, no_session_msg}
	}
	ctx, cancel := context.WithTimeout(context.Background(), 2*time.Second)
	defer cancel()
	return db.client.Ping(ctx, nil)
}

func (db *Mongodb) Connect(address string) (err error) {
	dbClientLock.Lock()
	defer dbClientLock.Unlock()

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

	if db.lastReadFromInprocess == nil {
		db.lastReadFromInprocess = make(map[string]int64, 100)
	}

	return db.Ping()
}

func (db *Mongodb) Close() {
	dbClientLock.Lock()
	defer dbClientLock.Unlock()
	if db.client != nil {
		ctx, cancel := context.WithTimeout(context.Background(), 2*time.Second)
		defer cancel()
		db.client.Disconnect(ctx)
		db.client = nil
	}
}

func (db *Mongodb) insertRecord(dbname string, collection_name string, s interface{}) error {
	if db.client == nil {
		return &DBError{utils.StatusServiceUnavailable, no_session_msg}
	}

	c := db.client.Database(dbname).Collection(collection_name)

	_, err := c.InsertOne(context.TODO(), s)
	return err
}

func duplicateError(err error) bool {
	command_error, ok := err.(mongo.CommandError)
	if !ok {
		write_exception_error, ok1 := err.(mongo.WriteException)
		if !ok1 {
			return false
		}
		return strings.Contains(write_exception_error.Error(), "duplicate key")
	}
	return command_error.Name == "DuplicateKey"
}

func (db *Mongodb) checkDatabaseOperationPrerequisites(request Request) error {
	if db.client == nil {
		return &DBError{utils.StatusServiceUnavailable, no_session_msg}
	}

	if len(request.DbName) == 0 || len(request.Collection) == 0 {
		return &DBError{utils.StatusWrongInput, "database ans collection must be set"}
	}

	return nil
}

func (db *Mongodb) createRecord(request Request, extra_params ...interface{}) ([]byte, error) {
	if len(extra_params) != 1 {
		return nil, errors.New("wrong number of parameters")
	}
	record := extra_params[0]
	c := db.client.Database(request.DbName).Collection(request.Collection)
	res, err := c.InsertOne(context.TODO(), record, options.InsertOne())
	if err != nil {
		return nil, err
	}
	newId, ok := res.InsertedID.(primitive.ObjectID)
	if ok {
		return []byte(newId.Hex()), nil
	}
	return nil, nil
}

func (db *Mongodb) readRecords(request Request, extraParams ...interface{}) ([]byte, error) {
	c := db.client.Database(request.DbName).Collection(request.Collection)

	if len(extraParams) != 1 {
		return nil, errors.New("wrong number of parameters")
	}
	res := extraParams[0]

	opts := options.Find()

	cursor, err := c.Find(context.TODO(), bson.M{}, opts)
	if err != nil {
		return nil, err
	}

	err = cursor.All(context.TODO(), res)
	if err != nil {
		return nil, err
	}
	return nil, nil
}

func (db *Mongodb) readRecord(request Request, extraParams ...interface{}) ([]byte, error) {
	c := db.client.Database(request.DbName).Collection(request.Collection)

	if len(extraParams) != 2 {
		return nil, errors.New("wrong number of parameters")
	}
	q := extraParams[0]
	res := extraParams[1]
	err := c.FindOne(context.TODO(), q, options.FindOne()).Decode(res)
	if err != nil {
		return nil, err
	}
	return nil, nil
}

func (db *Mongodb) updateRecord(request Request, extra_params ...interface{}) ([]byte, error) {
	if len(extra_params) != 4 {
		return nil, errors.New("wrong number of parameters")
	}
	id, ok := extra_params[0].(string)
	if !ok {
		return nil, errors.New("id must be string")
	}
	input := extra_params[1]
	upsert, ok := extra_params[2].(bool)
	if !ok {
		return nil, errors.New("upsert must be string")
	}
	output := extra_params[3]

	opts := options.FindOneAndUpdate().SetUpsert(upsert).SetReturnDocument(options.After)
	filter := bson.D{{"_id", id}}

	update := bson.D{{"$set", input}}
	c := db.client.Database(request.DbName).Collection(request.Collection)
	err := c.FindOneAndUpdate(context.TODO(), filter, update, opts).Decode(output)
	return nil, err
}


func (db *Mongodb) ProcessRequest(request Request, extraParams ...interface{}) (answer []byte, err error) {
	dbClientLock.RLock()
	defer dbClientLock.RUnlock()

	if err := db.checkDatabaseOperationPrerequisites(request); err != nil {
		return nil, err
	}

	switch request.Op {
	case "create_record":
		return db.createRecord(request, extraParams...)
	case "read_records":
		return db.readRecords(request, extraParams...)
	case "read_record":
		return db.readRecord(request, extraParams...)
	case "update_record":
		return db.updateRecord(request, extraParams...)
	}

	return nil, errors.New("Wrong db operation: " + request.Op)
}
