//+build !test

package database

import (
	"asapo_common/utils"
	"context"
	"errors"
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

func (db *Mongodb) ProcessRequest(request Request) (answer []byte, err error) {
	dbClientLock.RLock()
	defer dbClientLock.RUnlock()

	if err := db.checkDatabaseOperationPrerequisites(request); err != nil {
		return nil, err
	}

	switch request.Op {
	}

	return nil, errors.New("Wrong db operation: " + request.Op)
}
