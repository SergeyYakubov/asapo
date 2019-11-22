// +build integration_tests

package database

import (
	"asapo_common/utils"
	"context"
	"encoding/json"
	"github.com/stretchr/testify/assert"
	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/mongo/options"
	"sync"
	"testing"
)

type TestRecord struct {
	ID    int    `bson:"_id" json:"_id"`
	FName string `bson:"fname" json:"fname"`
}

type TestDataset struct {
	ID     int          `bson:"_id" json:"_id"`
	Size   int          `bson:"size" json:"size"`
	Images []TestRecord `bson:"images" json:"images"`
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
	if db.client == nil {
		return
	}
	db.deleteAllRecords(dbname)
	db.db_pointers_created = nil
	db.Close()
}

// these are the integration tests. They assume mongo db is runnig on 127.0.0.1:27027
// test names should contain MongoDB*** so that go test could find them:
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
	_, err := db.ProcessRequest(dbname, groupId, "next", "")
	assert.Equal(t, utils.StatusServiceUnavailable, err.(*DBError).Code)
}

func TestMongoDBGetMetaErrorWhenNotConnected(t *testing.T) {
	_, err := db.ProcessRequest(dbname, "", "meta", "0")
	assert.Equal(t, utils.StatusServiceUnavailable, err.(*DBError).Code)
}

func TestMongoDBQueryImagesErrorWhenNotConnected(t *testing.T) {
	_, err := db.ProcessRequest(dbname, "", "queryimages", "0")
	assert.Equal(t, utils.StatusServiceUnavailable, err.(*DBError).Code)
}

func TestMongoDBGetNextErrorWhenWrongDatabasename(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	_, err := db.ProcessRequest("", groupId, "next", "")
	assert.Equal(t, utils.StatusWrongInput, err.(*DBError).Code)
}

func TestMongoDBGetNextErrorWhenEmptyCollection(t *testing.T) {
	db.Connect(dbaddress)
	db.databases = append(db.databases, dbname)
	defer cleanup()
	_, err := db.ProcessRequest(dbname, groupId, "next", "")
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
}

func TestMongoDBGetNextErrorWhenRecordNotThereYet(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, &rec2)
	_, err := db.ProcessRequest(dbname, groupId, "next", "")
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":2}", err.Error())
}

func TestMongoDBGetNextOK(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, &rec1)
	res, err := db.ProcessRequest(dbname, groupId, "next", "")
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBGetNextErrorOnNoMoreData(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, &rec1)
	db.ProcessRequest(dbname, groupId, "next", "")
	_, err := db.ProcessRequest(dbname, groupId, "next", "")

	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1}", err.(*DBError).Message)
}

func TestMongoDBGetNextCorrectOrder(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, &rec2)
	db.insertRecord(dbname, &rec1)
	res1, _ := db.ProcessRequest(dbname, groupId, "next", "")
	res2, _ := db.ProcessRequest(dbname, groupId, "next", "")
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
		db.insertRecord(dbname, &record)
	}

}

func getRecords(n int) []int {
	results := make([]int, n)
	var wg sync.WaitGroup
	wg.Add(n)
	for i := 0; i < n; i++ {
		go func() {
			defer wg.Done()
			res_bin, _ := db.ProcessRequest(dbname, groupId, "next", "")
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

func TestMongoDBgetRecordByID(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, &rec1)
	res, err := db.getRecordByID(dbname, "", "1", false)
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBgetRecordByIDFails(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, &rec1)
	_, err := db.getRecordByID(dbname, "", "2", false)
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":2,\"id_max\":1}", err.Error())
}

func TestMongoDBGetRecordNext(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, &rec1)
	res, err := db.ProcessRequest(dbname, groupId, "next", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBGetRecordID(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, &rec1)
	res, err := db.ProcessRequest(dbname, groupId, "id", "1")
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBWrongOp(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, &rec1)
	_, err := db.ProcessRequest(dbname, groupId, "bla", "0")
	assert.NotNil(t, err)
}

func TestMongoDBGetRecordLast(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, &rec1)
	db.insertRecord(dbname, &rec2)

	res, err := db.ProcessRequest(dbname, groupId, "last", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(rec2_expect), string(res))
}

func TestMongoDBGetNextAfterGetLastCorrect(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, &rec1)
	db.insertRecord(dbname, &rec2)

	res, err := db.ProcessRequest(dbname, groupId, "last", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(rec2_expect), string(res))

	db.insertRecord(dbname, &rec3)

	res, err = db.ProcessRequest(dbname, groupId, "next", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(rec3_expect), string(res))

}

func TestMongoDBGetSize(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, &rec1)
	db.insertRecord(dbname, &rec2)
	db.insertRecord(dbname, &rec3)

	res, err := db.ProcessRequest(dbname, "", "size", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(recs1_expect), string(res))
}

func TestMongoDBGetSizeNoRecords(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	// to have empty collection
	db.insertRecord(dbname, &rec1)
	db.client.Database(dbname).Collection(data_collection_name).DeleteOne(context.TODO(), bson.M{"_id": 1}, options.Delete())

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

func TestMongoPing(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	err := db.Ping()
	assert.Nil(t, err)
}

func TestMongoPingNotConected(t *testing.T) {
	err := db.Ping()
	assert.NotNil(t, err)
}

func TestMongoDBgetRecordByIDNotConnected(t *testing.T) {
	_, err := db.ProcessRequest(dbname, "", "id", "2")
	assert.Equal(t, utils.StatusServiceUnavailable, err.(*DBError).Code)
}

func TestMongoDBResetCounter(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, &rec1)
	db.insertRecord(dbname, &rec2)

	res1, err1 := db.ProcessRequest(dbname, groupId, "next", "0")

	assert.Nil(t, err1)
	assert.Equal(t, string(rec1_expect), string(res1))

	_, err_reset := db.ProcessRequest(dbname, groupId, "resetcounter", "1")
	assert.Nil(t, err_reset)

	res2, err2 := db.ProcessRequest(dbname, groupId, "next", "0")

	assert.Nil(t, err2)
	assert.Equal(t, string(rec2_expect), string(res2))
}

func TestMongoDBGetMetaOK(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	rec1.ID = metaID
	rec_expect, _ := json.Marshal(rec1)
	db.insertMeta(dbname, &rec1)

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

type MetaData struct {
	Temp    float64 `bson:"temp" json:"temp"`
	Counter int     `bson:"counter" json:"counter"`
	Text    string  `bson:"text" json:"text"`
}

type TestRecordMeta struct {
	ID    int      `bson:"_id" json:"_id"`
	FName string   `bson:"fname" json:"fname"`
	Meta  MetaData `bson:"meta" json:"meta"`
}

var recq1 = TestRecordMeta{1, "aaa", MetaData{10.2, 10, "aaa"}}
var recq2 = TestRecordMeta{2, "bbb", MetaData{11.2, 11, "bbb"}}
var recq3 = TestRecordMeta{3, "ccc", MetaData{10.2, 10, "ccc"}}
var recq4 = TestRecordMeta{4, "ddd", MetaData{13.2, 13, ""}}

var tests = []struct {
	query string
	res   []TestRecordMeta
	ok    bool
}{
	{"meta.counter = 10", []TestRecordMeta{recq1, recq3}, true},
	{"meta.counter = 10 ORDER BY _id DESC", []TestRecordMeta{recq3, recq1}, true},
	{"meta.counter > 10 ORDER BY meta.counter DESC", []TestRecordMeta{recq4, recq2}, true},
	{"meta.counter = 18", []TestRecordMeta{}, true},
	{"meta.counter = 11", []TestRecordMeta{recq2}, true},
	{"meta.counter > 11", []TestRecordMeta{recq4}, true},
	{"meta.counter < 11", []TestRecordMeta{recq1, recq3}, true},
	{"meta.counter <= 11", []TestRecordMeta{recq1, recq2, recq3}, true},
	{"meta.counter >= 11", []TestRecordMeta{recq2, recq4}, true},
	{"meta.temp = 11.2", []TestRecordMeta{recq2}, true},
	{"meta.text = 'ccc'", []TestRecordMeta{recq3}, true},
	{"meta.text = ''", []TestRecordMeta{recq4}, true},
	{"meta.text = ccc", []TestRecordMeta{}, false},
	{"meta.text != 'ccc'", []TestRecordMeta{recq1, recq2, recq4}, true},
	{"meta.temp BETWEEN 11 AND 13", []TestRecordMeta{recq2}, true},
	{"meta.temp not BETWEEN 11 and 13", []TestRecordMeta{recq1, recq3, recq4}, true},
	{"meta.counter IN (10,13)", []TestRecordMeta{recq1, recq3, recq4}, true},
	{"meta.counter NOT IN (10,13)", []TestRecordMeta{recq2}, true},
	{"meta.text IN ('aaa','ccc')", []TestRecordMeta{recq1, recq3}, true},
	{"_id = 1", []TestRecordMeta{recq1}, true},
	{"meta.text REGEXP '^c+'", []TestRecordMeta{recq3}, true},
	{"meta.text REGEXP '^a|b'", []TestRecordMeta{recq1, recq2}, true},
	// mongo 4.07+ is needed for NOT REXEXP
	{"meta.text NOT REGEXP '^c+'", []TestRecordMeta{recq1, recq2, recq4}, true},
	{"give_error", []TestRecordMeta{}, false},
	{"meta.counter = 10 AND meta.text = 'ccc'", []TestRecordMeta{recq3}, true},
	{"meta.counter = 10 OR meta.text = 'bbb'", []TestRecordMeta{recq1, recq2, recq3}, true},
	{"(meta.counter = 10 OR meta.counter = 11) AND (meta.text = 'bbb' OR meta.text = 'ccc')", []TestRecordMeta{recq2, recq3}, true},
	{"(meta.counter = 10 OR meta.counter = 11 AND (meta.text = 'bbb' OR meta.text = 'ccc')", []TestRecordMeta{}, false},
}

func TestMongoDBQueryImagesOK(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	//	logger.SetLevel(logger.DebugLevel)
	db.insertRecord(dbname, &recq1)
	db.insertRecord(dbname, &recq2)
	db.insertRecord(dbname, &recq3)
	db.insertRecord(dbname, &recq4)

	for _, test := range tests {
		//		info, _ := db.client.BuildInfo()
		//		if strings.Contains(test.query, "NOT REGEXP") && !info.VersionAtLeast(4, 0, 7) {
		//			fmt.Println("Skipping NOT REGEXP test since it is not supported by this mongodb version")
		//			continue
		//		}

		res_string, err := db.ProcessRequest(dbname, "", "queryimages", test.query)
		var res []TestRecordMeta
		json.Unmarshal(res_string, &res)
		//		fmt.Println(string(res_string))
		if test.ok {
			assert.Nil(t, err, test.query)
			assert.Equal(t, test.res, res)
		} else {
			assert.NotNil(t, err, test.query)
			assert.Equal(t, 0, len(res))
		}
	}

}

var rec_dataset1 = TestDataset{1, 3, []TestRecord{rec1, rec2, rec3}}
var rec_dataset1_incomplete = TestDataset{1, 4, []TestRecord{rec1, rec2, rec3}}
var rec_dataset2 = TestDataset{2, 4, []TestRecord{rec1, rec2, rec3}}
var rec_dataset3 = TestDataset{3, 3, []TestRecord{rec3, rec2, rec2}}

func TestMongoDBGetDataset(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, &rec_dataset1)

	res_string, err := db.ProcessRequest(dbname, groupId, "next_dataset", "0")

	assert.Nil(t, err)

	var res TestDataset
	json.Unmarshal(res_string, &res)

	assert.Equal(t, rec_dataset1, res)
}

func TestMongoDBNoDataOnNotCompletedFirstDataset(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, &rec_dataset1_incomplete)

	res_string, err := db.ProcessRequest(dbname, groupId, "next_dataset", "0")

	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":0,\"id_max\":0}", err.(*DBError).Message)

	assert.Equal(t, "", string(res_string))
}

func TestMongoDBGetRecordLastDataSetSkipsIncompleteSets(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, &rec_dataset1)
	db.insertRecord(dbname, &rec_dataset2)

	res_string, err := db.ProcessRequest(dbname, groupId, "last_dataset", "0")

	assert.Nil(t, err)

	var res TestDataset
	json.Unmarshal(res_string, &res)

	assert.Equal(t, rec_dataset1, res)
}

func TestMongoDBGetRecordLastDataSetOK(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, &rec_dataset1)
	db.insertRecord(dbname, &rec_dataset3)

	res_string, err := db.ProcessRequest(dbname, groupId, "last_dataset", "0")

	assert.Nil(t, err)

	var res TestDataset
	json.Unmarshal(res_string, &res)

	assert.Equal(t, rec_dataset3, res)
}

func TestMongoDBGetDatasetID(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, &rec_dataset1)

	res_string, err := db.ProcessRequest(dbname, groupId, "id_dataset", "1")

	assert.Nil(t, err)

	var res TestDataset
	json.Unmarshal(res_string, &res)

	assert.Equal(t, rec_dataset1, res)

}
