// +build integration_tests

package database

import (
	"asapo_common/utils"
	"encoding/json"
	"github.com/stretchr/testify/assert"
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
const groupId = "bid2a5auidddp1vl71d0"
const metaID = 0
const metaID_str = "0"

var rec1 = TestRecord{1, "aaa"}
var rec2 = TestRecord{2, "bbb"}
var rec3 = TestRecord{3, "ccc"}
var rec1_expect, _ = json.Marshal(rec1)
var rec2_expect, _ = json.Marshal(rec2)
var rec3_expect, _ = json.Marshal(rec3)

var recs1 = SizeRecord{3}
var recs1_expect, _ = json.Marshal(recs1)
var recs2 = SizeRecord{0}
var recs2_expect, _ = json.Marshal(recs2)

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
	_, err := db.GetNextRecord("", groupId)
	assert.Equal(t, utils.StatusError, err.(*DBError).Code)
}

func TestMongoDBGetNextErrorWhenWrongDatabasename(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	_, err := db.GetNextRecord("", groupId)
	assert.Equal(t, utils.StatusWrongInput, err.(*DBError).Code)
}

func TestMongoDBGetNextErrorWhenEmptyCollection(t *testing.T) {
	db.Connect(dbaddress)
	db.databases = append(db.databases, dbname)
	defer cleanup()
	_, err := db.GetNextRecord(dbname, groupId)
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
}

func TestMongoDBGetNextErrorWhenRecordNotThereYet(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec2)
	_, err := db.GetNextRecord(dbname, groupId)
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"id\":1}", err.Error())
}

func TestMongoDBGetNextOK(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec1)
	res, err := db.GetNextRecord(dbname, groupId)
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBGetNextErrorOnNoMoreData(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec1)
	db.GetNextRecord(dbname, groupId)
	_, err := db.GetNextRecord(dbname, groupId)
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
}

func TestMongoDBGetNextCorrectOrder(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec2)
	db.InsertRecord(dbname, &rec1)
	res1, _ := db.GetNextRecord(dbname, groupId)
	res2, _ := db.GetNextRecord(dbname, groupId)
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
			res_bin, _ := db.GetNextRecord(dbname, groupId)
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

func TestMongoDBGetRecordByID(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec1)
	res, err := db.GetRecordByID(dbname, "", "1", true, false)
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBGetRecordByIDFails(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec1)
	_, err := db.GetRecordByID(dbname, "", "2", true, false)
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"id\":2}", err.Error())
}

func TestMongoDBGetRecordNext(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec1)
	res, err := db.ProcessRequest(dbname, groupId, "next", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBGetRecordID(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec1)
	res, err := db.ProcessRequest(dbname, groupId, "id", "1")
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBWrongOp(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec1)
	_, err := db.ProcessRequest(dbname, groupId, "bla", "0")
	assert.NotNil(t, err)
}

func TestMongoDBGetRecordLast(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec1)
	db.InsertRecord(dbname, &rec2)

	res, err := db.ProcessRequest(dbname, groupId, "last", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(rec2_expect), string(res))
}

func TestMongoDBGetNextAfterGetLastCorrect(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec1)
	db.InsertRecord(dbname, &rec2)

	res, err := db.ProcessRequest(dbname, groupId, "last", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(rec2_expect), string(res))

	db.InsertRecord(dbname, &rec3)

	res, err = db.ProcessRequest(dbname, groupId, "next", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(rec3_expect), string(res))

}

func TestMongoDBGetSize(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec1)
	db.InsertRecord(dbname, &rec2)
	db.InsertRecord(dbname, &rec3)

	res, err := db.ProcessRequest(dbname, "", "size", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(recs1_expect), string(res))
}

func TestMongoDBGetSizeNoRecords(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	// to have empty collection
	db.InsertRecord(dbname, &rec1)
	db.session.DB(dbname).C(data_collection_name).RemoveId(1)

	res, err := db.ProcessRequest(dbname, "", "size", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(recs2_expect), string(res))
}

func TestMongoDBGetSizeNoDatabase(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	_, err := db.ProcessRequest(dbname, "", "size", "0")
	assert.NotNil(t, err)
}

func TestMongoDBGetRecordIDWithReset(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec1)
	db.InsertRecord(dbname, &rec2)

	res1, err1 := db.ProcessRequest(dbname, groupId, "idreset", "1")
	res2, err2 := db.ProcessRequest(dbname, groupId, "next", "0")

	assert.Nil(t, err1)
	assert.Equal(t, string(rec1_expect), string(res1))
	assert.Nil(t, err2)
	assert.Equal(t, string(rec2_expect), string(res2))

}

func TestMongoDBGetRecordByIDNotConnected(t *testing.T) {
	_, err := db.GetRecordByID(dbname, "", "2", true, false)
	assert.Equal(t, utils.StatusError, err.(*DBError).Code)
}

func TestMongoDBResetCounter(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.InsertRecord(dbname, &rec1)
	db.InsertRecord(dbname, &rec2)

	res1, err1 := db.ProcessRequest(dbname, groupId, "next", "0")

	assert.Nil(t, err1)
	assert.Equal(t, string(rec1_expect), string(res1))

	_, err_reset := db.ProcessRequest(dbname, groupId, "resetcounter", "0")
	assert.Nil(t, err_reset)

	res2, err2 := db.ProcessRequest(dbname, groupId, "next", "0")

	assert.Nil(t, err2)
	assert.Equal(t, string(rec1_expect), string(res2))

}

func TestMongoDBGetMetaOK(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	rec1.ID = metaID
	rec_expect, _ := json.Marshal(rec1)
	db.InsertMeta(dbname, &rec1)

	res, err := db.ProcessRequest(dbname, "", "meta", metaID_str)

	assert.Nil(t, err)
	assert.Equal(t, string(rec_expect), string(res))
}

func TestMongoDBGetMetaErr(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	_, err := db.ProcessRequest(dbname, "", "meta", metaID_str)
	assert.NotNil(t, err)
}
