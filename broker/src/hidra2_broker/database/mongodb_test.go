// +build integration_tests

package database

import (
	"encoding/json"
	"github.com/stretchr/testify/assert"
	"hidra2_broker/utils"
	"sync"
	"testing"
)

type TestRecord struct {
	ID    int    `bson:"_id" json:"_id"`
	FName string `bson:"fname" json:"fname"`
}

var db Mongodb

const dbname = "run1"
const dbaddress = "127.0.0.1:27017"

var rec1 = TestRecord{1, "aaa"}
var rec2 = TestRecord{2, "bbb"}
var rec1_expect, _ = json.Marshal(rec1)
var rec2_expect, _ = json.Marshal(rec2)

func cleanup() {
	db.DeleteAllRecords(dbname)
	db.db_pointers_created = nil
	db.Close()
}

// these are tjhe integration tests. They assume mongo db is runnig on 127.0.0.1:27027
// test names shlud contain MongoDB*** so that go test could find them:
// go_integration_test(${TARGET_NAME}-connectdb "./..." "MongoDBConnect")
func TestMongoDBConnectFails(t *testing.T) {
	err := db.Connect("blabla")
	defer cleanup()
	assert.NotNil(t, err)
}

func TestMongoDBConnectOK(t *testing.T) {
	err := db.Connect(dbaddress)
	defer cleanup()
	assert.Nil(t, err)
}

func TestMongoDBGetNextErrorWhenNotConnected(t *testing.T) {
	_, err := db.GetNextRecord("")
	assert.Equal(t, utils.StatusError, err.(*DBError).Code)
}

func TestMongoDBGetNextErrorWhenWrongDatabasename(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	_, err := db.GetNextRecord("")
	assert.Equal(t, utils.StatusWrongInput, err.(*DBError).Code)
}

func TestMongoDBGetNextErrorWhenEmptyCollection(t *testing.T) {
	db.Connect(dbaddress)
	db.databases = append(db.databases, dbname)
	defer cleanup()
	_, err := db.GetNextRecord(dbname)
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
}

func TestMongoDBGetNextOK(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec1)
	res, err := db.GetNextRecord(dbname)
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBGetNextErrorOnNoMoreData(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec1)
	db.GetNextRecord(dbname)
	_, err := db.GetNextRecord(dbname)
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
}

func TestMongoDBGetNextCorrectOrder(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec2)
	db.InsertRecord(dbname, &rec1)
	res1, _ := db.GetNextRecord(dbname)
	res2, _ := db.GetNextRecord(dbname)
	assert.Equal(t, string(rec1_expect), string(res1))
	assert.Equal(t, string(rec2_expect), string(res2))
}

func getNOnes(array []int) int {
	sum1 := 0
	for _, result := range array {
		if result == 1 {
			sum1++
		}
	}
	return sum1
}

func insertRecords(n int) {
	records := make([]TestRecord, n)
	for ind, record := range records {
		record.ID = ind
		record.FName = string(ind)
		db.InsertRecord(dbname, &record)
	}

}

func getRecords(n int) []int {
	results := make([]int, n)
	var wg sync.WaitGroup
	wg.Add(n)
	for i := 0; i < n; i++ {
		go func() {
			defer wg.Done()
			res_bin, _ := db.GetNextRecord(dbname)
			var res TestRecord
			json.Unmarshal(res_bin, &res)
			results[res.ID] = 1
		}()
	}
	wg.Wait()

	return results
}

func TestMongoDBGetNextInParallel(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	n := 100
	insertRecords(n)

	results := getRecords(n)

	assert.Equal(t, n, getNOnes(results))
}
