// +build integration_tests

package database

import (
	"asapo_common/utils"
	"encoding/json"
	"fmt"
	"github.com/stretchr/testify/assert"
	"sync"
	"testing"
	"time"
)

type TestRecord struct {
	ID        int64             `bson:"_id" json:"_id"`
	Meta      map[string]string `bson:"meta" json:"meta"`
	Name      string            `bson:"name" json:"name"`
	Timestamp int64             `bson:"timestamp" json:"timestamp"`
}

type TestDataset struct {
	Timestamp int64        `bson:"timestamp" json:"timestamp"`
	ID        int64        `bson:"_id" json:"_id"`
	Size      int64        `bson:"size" json:"size"`
	Messages  []TestRecord `bson:"messages" json:"messages"`
}

var db Mongodb

const dbname = "12345"
const collection = "stream"
const collection2 = "stream2"
const dbaddress = "127.0.0.1:27017"
const groupId = "bid2a5auidddp1vl71d0"
const metaID = 0
const metaID_str = "0"

const badSymbolsDb = `/\."$`
const badSymbolsCol = `$`
const badSymbolsDbEncoded = "%2F%5C%2E%22%24"
const badSymbolsColEncoded ="%24"

var empty_next = map[string]string{"next_stream": ""}

var rec1 = TestRecord{1, empty_next, "aaa", 0}
var rec1_later = TestRecord{1, empty_next, "aaa", 1}
var rec_finished = TestRecord{2, map[string]string{"next_stream": "next1"}, finish_stream_keyword, 2}
var rec2 = TestRecord{2, empty_next, "bbb", 1}
var rec3 = TestRecord{3, empty_next, "ccc", 2}
var rec_finished3 = TestRecord{3, map[string]string{"next_stream": "next1"}, finish_stream_keyword, 2}
var rec_finished11 = TestRecord{11, map[string]string{"next_stream": "next1"}, finish_stream_keyword, 2}

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
	db.dropDatabase(dbname)
	db.Close()
}

func cleanupWithName(name string) {
	if db.client == nil {
		return
	}
	db.dropDatabase(name)
	db.Close()
}


// these are the integration tests. They assume mongo db is runnig on 127.0.0.1:27027
// test names should contain MongoDB*** so that go test could find them:
// go_integration_test(${TARGET_NAME}-connectdb "./..." "MongoDBConnect")
func TestMongoDBConnectFails(t *testing.T) {
	err := db.Connect("blabla")
	defer db.Close()
	assert.NotNil(t, err)
}

func TestMongoDBConnectOK(t *testing.T) {
	err := db.Connect(dbaddress)
	defer cleanup()
	assert.Nil(t, err)
}

func TestMongoDBGetNextErrorWhenNotConnected(t *testing.T) {
	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
	assert.Equal(t, utils.StatusServiceUnavailable, err.(*DBError).Code)
}

func TestMongoDBGetMetaErrorWhenNotConnected(t *testing.T) {
	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, Op: "meta", ExtraParam: "0"})
	assert.Equal(t, utils.StatusServiceUnavailable, err.(*DBError).Code)
}

func TestMongoDBQueryMessagesErrorWhenNotConnected(t *testing.T) {
	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, Op: "querymessages", ExtraParam: "0"})
	assert.Equal(t, utils.StatusServiceUnavailable, err.(*DBError).Code)
}

func TestMongoDBGetNextErrorWhenWrongDatabasename(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	_, err := db.ProcessRequest(Request{DbCollectionName: collection, GroupId: groupId, Op: "next"})
	assert.Equal(t, utils.StatusWrongInput, err.(*DBError).Code)
}

func TestMongoDBGetNextErrorWhenNonExistingDatacollectionname(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: "bla", GroupId: groupId, Op: "next"})
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":0,\"id_max\":0,\"next_stream\":\"\"}", err.Error())
}

func TestMongoDBGetLastErrorWhenNonExistingDatacollectionname(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: "bla", GroupId: groupId, Op: "last"})
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":0,\"id_max\":0,\"next_stream\":\"\"}", err.Error())
}

func TestMongoDBGetByIdErrorWhenNoData(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "id", ExtraParam: "2"})

	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":2,\"id_max\":0,\"next_stream\":\"\"}", err.Error())
}

func TestMongoDBGetNextErrorWhenRecordNotThereYet(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec2)
	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":2,\"next_stream\":\"\"}", err.Error())
}

func TestMongoDBGetNextOK(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBGetNextErrorOnFinishedStream(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec_finished)

	db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})

	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_stream\":\"next1\"}", err.(*DBError).Message)
}

func TestMongoDBGetNextErrorOnFinishedStreamAlways(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec_finished)

	db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
	db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})

	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_stream\":\"next1\"}", err.(*DBError).Message)
}



func TestMongoDBGetByIdErrorOnFinishedStream(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec_finished)

	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "id", ExtraParam: "2"})

	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_stream\":\"next1\"}", err.(*DBError).Message)
}

func TestMongoDBGetLastErrorOnFinishedStream(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec_finished)

	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "last"})
	fmt.Println(string(res))
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_stream\":\"next1\"}", err.(*DBError).Message)
}

func TestMongoDBGetNextErrorOnNoMoreData(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})

	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_stream\":\"\"}", err.(*DBError).Message)
}

func TestMongoDBGetNextCorrectOrder(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec2)
	db.insertRecord(dbname, collection, &rec1)
	res1, _ := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
	res2, _ := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
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
		record.ID = int64(ind) + 1
		record.Name = fmt.Sprint(ind)
		if err := db.insertRecord(dbname, collection, &record); err != nil {
			fmt.Println("error at insert ", ind)
		}
	}
}

func getRecords(n int, resend bool) []int {
	results := make([]int, n)
	var wg sync.WaitGroup
	wg.Add(n)
	extra_param := ""
	if resend {
		extra_param = "0_1"
	}
	for i := 0; i < n; i++ {
		go func() {
			defer wg.Done()
			res_bin, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: extra_param})
			if err != nil {
				fmt.Println("error at read ", i)
			}
			var res TestRecord
			json.Unmarshal(res_bin, &res)
			if res.ID > 0 {
				results[res.ID-1] = 1
			}
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

	results := getRecords(n, false)

	assert.Equal(t, n, getNOnes(results))
}

func TestMongoDBGetNextInParallelWithResend(t *testing.T) {
	db.SetSettings(DBSettings{ReadFromInprocessPeriod: 100})
	db.Connect(dbaddress)
	defer cleanup()
	n := 100
	insertRecords(n)

	results := getRecords(n, true)
	results2 := getRecords(n, true)

	assert.Equal(t, n, getNOnes(results), "first")
	assert.Equal(t, n, getNOnes(results2), "second")
}

func TestMongoDBGetLastAfterErasingDatabase(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	insertRecords(10)
	db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
	db.dropDatabase(dbname)

	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec2)

	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "last", ExtraParam: "0"})
	assert.Nil(t, err)
	assert.Equal(t, string(rec2_expect), string(res))
}

func TestMongoDBGetNextAfterErasingDatabase(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	insertRecords(200)
	db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
	db.dropDatabase(dbname)

	n := 100
	insertRecords(n)
	results := getRecords(n, false)
	assert.Equal(t, n, getNOnes(results))
}

func TestMongoDBGetNextEmptyAfterErasingDatabase(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	insertRecords(10)
	db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
	db.dropDatabase(dbname)

	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":0,\"id_max\":0,\"next_stream\":\"\"}", err.Error())
}

func TestMongoDBgetRecordByID(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)

	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "id", ExtraParam: "1"})
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBgetRecordByIDFails(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "id", ExtraParam: "2"})
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":2,\"id_max\":1,\"next_stream\":\"\"}", err.Error())
}

func TestMongoDBGetRecordNext(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBGetRecordNextMultipleCollections(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection2, &rec_dataset1)

	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
	res_string, err2 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection2, GroupId: groupId, Op: "next", DatasetOp: true})
	var res_ds TestDataset
	json.Unmarshal(res_string, &res_ds)

	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))

	assert.Nil(t, err2)
	assert.Equal(t, rec_dataset1, res_ds)

}

func TestMongoDBGetRecordID(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "id", ExtraParam: "1"})
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBWrongOp(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "bla"})
	assert.NotNil(t, err)
}

func TestMongoDBGetRecordLast(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec2)

	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "last", ExtraParam: "0"})
	assert.Nil(t, err)
	assert.Equal(t, string(rec2_expect), string(res))
}

func TestMongoDBGetNextAfterGetLastCorrect(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec2)

	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "last", ExtraParam: "0"})
	assert.Nil(t, err)
	assert.Equal(t, string(rec2_expect), string(res))

	db.insertRecord(dbname, collection, &rec3)

	res, err = db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))

}

func TestMongoDBGetSize(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec2)
	db.insertRecord(dbname, collection, &rec3)

	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, Op: "size"})
	assert.Nil(t, err)
	assert.Equal(t, string(recs1_expect), string(res))
}

func TestMongoDBGetSizeWithFinishedStream(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec_finished)

	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, Op: "size"})
	assert.Nil(t, err)
	var rec_expect, _ = json.Marshal(&SizeRecord{1})
	assert.Equal(t, string(rec_expect), string(res))
}

func TestMongoDBGetSizeForDatasets(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)

	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, Op: "size", ExtraParam: "false"})
	assert.Equal(t, utils.StatusWrongInput, err.(*DBError).Code)

	_, err1 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, Op: "size", ExtraParam: "true"})
	assert.Equal(t, utils.StatusWrongInput, err1.(*DBError).Code)
}

func TestMongoDBGetSizeForDatasetsWithFinishedStream(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec_dataset1_incomplete)
	db.insertRecord(dbname, collection, &rec_finished)

	res, _ := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, Op: "size", ExtraParam: "true"})

	var rec_expect, _ = json.Marshal(&SizeRecord{1})
	assert.Equal(t, string(rec_expect), string(res))
}

func TestMongoDBGetSizeDataset(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, collection, &rec_dataset1)
	db.insertRecord(dbname, collection, &rec_dataset2_incomplete)

	size2_expect, _ := json.Marshal(SizeRecord{2})

	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, Op: "size", ExtraParam: "true"})
	assert.Nil(t, err)
	assert.Equal(t, string(size2_expect), string(res))
}

func TestMongoDBGetSizeNoRecords(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, Op: "size"})
	assert.Nil(t, err)
	assert.Equal(t, string(recs2_expect), string(res))
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
	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "id", ExtraParam: "1"})
	assert.Equal(t, utils.StatusServiceUnavailable, err.(*DBError).Code)
}

func TestMongoDBResetCounter(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec2)

	res1, err1 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})

	assert.Nil(t, err1)
	assert.Equal(t, string(rec1_expect), string(res1))

	_, err_reset := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "resetcounter", ExtraParam: "1"})
	assert.Nil(t, err_reset)

	res2, err2 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})

	assert.Nil(t, err2)
	assert.Equal(t, string(rec2_expect), string(res2))
}

func TestMongoDBGetMetaOK(t *testing.T) {
	recm := rec1
	db.Connect(dbaddress)
	defer cleanup()
	recm.ID = metaID
	rec_expect, _ := json.Marshal(recm)
	db.insertMeta(dbname, &recm)

	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, Op: "meta", ExtraParam: metaID_str})

	assert.Nil(t, err)
	assert.Equal(t, string(rec_expect), string(res))
}

func TestMongoDBGetMetaErr(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, Op: "meta", ExtraParam: metaID_str})
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
	{"_id > 0", []TestRecordMeta{recq1, recq2, recq3, recq4}, true},
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

func TestMongoDBQueryMessagesOK(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	//	logger.SetLevel(logger.DebugLevel)
	db.insertRecord(dbname, collection, &recq1)
	db.insertRecord(dbname, collection, &recq2)
	db.insertRecord(dbname, collection, &recq3)
	db.insertRecord(dbname, collection, &recq4)

	for _, test := range tests {
		//		info, _ := db.client.BuildInfo()
		//		if strings.Contains(test.query, "NOT REGEXP") && !info.VersionAtLeast(4, 0, 7) {
		//			fmt.Println("Skipping NOT REGEXP test since it is not supported by this mongodb version")
		//			continue
		//		}

		res_string, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, Op: "querymessages", ExtraParam: test.query})
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

func TestMongoDBQueryMessagesOnEmptyDatabase(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	for _, test := range tests {
		res_string, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, Op: "querymessages", ExtraParam: test.query})
		var res []TestRecordMeta
		json.Unmarshal(res_string, &res)
		assert.Equal(t, 0, len(res))
		if test.ok {
			assert.Nil(t, err, test.query)
		} else {
			assert.NotNil(t, err, test.query)
		}
	}
}

var rec_dataset1 = TestDataset{0, 1, 3, []TestRecord{rec1, rec2, rec3}}
var rec_dataset1_incomplete = TestDataset{1, 1, 4, []TestRecord{rec1, rec2, rec3}}
var rec_dataset2_incomplete = TestDataset{2, 2, 4, []TestRecord{rec1, rec2, rec3}}
var rec_dataset2 = TestDataset{1, 2, 4, []TestRecord{rec1, rec2, rec3}}
var rec_dataset3 = TestDataset{2, 3, 3, []TestRecord{rec3, rec2, rec2}}

var rec_dataset2_incomplete3 = TestDataset{1, 2, 3, []TestRecord{rec1, rec2}}

func TestMongoDBGetDataset(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, collection, &rec_dataset1)

	res_string, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", DatasetOp: true})

	assert.Nil(t, err)

	var res TestDataset
	json.Unmarshal(res_string, &res)

	assert.Equal(t, rec_dataset1, res)
}

func TestMongoDBNoDataOnNotCompletedFirstDataset(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, collection, &rec_dataset1_incomplete)

	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", DatasetOp: true})

	assert.Equal(t, utils.StatusPartialData, err.(*DBError).Code)
	var res TestDataset
	json.Unmarshal([]byte(err.(*DBError).Message), &res)
	assert.Equal(t, rec_dataset1_incomplete, res)
}

func TestMongoDBNoDataOnNotCompletedNextDataset(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, collection, &rec_dataset1_incomplete)
	db.insertRecord(dbname, collection, &rec_dataset2_incomplete)

	_, err1 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", DatasetOp: true})
	_, err2 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", DatasetOp: true})

	assert.Equal(t, utils.StatusPartialData, err1.(*DBError).Code)
	assert.Equal(t, utils.StatusPartialData, err2.(*DBError).Code)
	var res TestDataset
	json.Unmarshal([]byte(err2.(*DBError).Message), &res)
	assert.Equal(t, rec_dataset2_incomplete, res)
}

func TestMongoDBGetRecordLastDataSetSkipsIncompleteSets(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, collection, &rec_dataset1)
	db.insertRecord(dbname, collection, &rec_dataset2)

	res_string, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "last", DatasetOp: true, ExtraParam: "0"})

	assert.Nil(t, err)

	var res TestDataset
	json.Unmarshal(res_string, &res)

	assert.Equal(t, rec_dataset1, res)
}

func TestMongoDBGetRecordLastDataSetReturnsIncompleteSets(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, collection, &rec_dataset1)
	db.insertRecord(dbname, collection, &rec_dataset2)

	res_string, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "last",
		DatasetOp: true, MinDatasetSize: 3, ExtraParam: "0"})

	assert.Nil(t, err)

	var res TestDataset
	json.Unmarshal(res_string, &res)

	assert.Equal(t, rec_dataset2, res)
}

func TestMongoDBGetRecordLastDataSetSkipsIncompleteSetsWithMinSize(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, collection, &rec_dataset1)
	db.insertRecord(dbname, collection, &rec_dataset2_incomplete3)

	res_string, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "last",
		DatasetOp: true, MinDatasetSize: 3, ExtraParam: "0"})

	assert.Nil(t, err)

	var res TestDataset
	json.Unmarshal(res_string, &res)
	assert.Equal(t, rec_dataset1, res)
}

func TestMongoDBGetRecordLastDataSetWithFinishedStream(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, collection, &rec_dataset1)
	db.insertRecord(dbname, collection, &rec_finished)

	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "last",
		DatasetOp: true, ExtraParam: "0"})

	assert.NotNil(t, err)
	if err != nil {
		assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
		assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_stream\":\"next1\"}", err.Error())
	}
}

func TestMongoDBGetRecordLastDataSetWithIncompleteDatasetsAndFinishedStreamReturnsEndofStream(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, collection, &rec_dataset1_incomplete)
	db.insertRecord(dbname, collection, &rec_finished)

	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "last",
		DatasetOp: true, MinDatasetSize: 2, ExtraParam: "0"})

	assert.NotNil(t, err)
	if err != nil {
		assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
		assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_stream\":\"next1\"}", err.Error())
	}
}

func TestMongoDBGetRecordLastDataSetOK(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, collection, &rec_dataset1)
	db.insertRecord(dbname, collection, &rec_dataset3)

	res_string, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "last", DatasetOp: true, ExtraParam: "0"})

	assert.Nil(t, err)

	var res TestDataset
	json.Unmarshal(res_string, &res)

	assert.Equal(t, rec_dataset3, res)
}

func TestMongoDBGetDatasetID(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec_dataset1)

	res_string, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "id", DatasetOp: true, ExtraParam: "1"})

	assert.Nil(t, err)

	var res TestDataset
	json.Unmarshal(res_string, &res)

	assert.Equal(t, rec_dataset1, res)

}

func TestMongoDBErrorOnIncompleteDatasetID(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec_dataset1_incomplete)

	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "id", DatasetOp: true, ExtraParam: "1"})

	assert.Equal(t, utils.StatusPartialData, err.(*DBError).Code)

	var res TestDataset
	json.Unmarshal([]byte(err.(*DBError).Message), &res)

	assert.Equal(t, rec_dataset1_incomplete, res)

}

func TestMongoDBOkOnIncompleteDatasetID(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec_dataset1_incomplete)

	res_string, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "id", DatasetOp: true, MinDatasetSize: 3, ExtraParam: "1"})

	assert.Nil(t, err)

	var res TestDataset
	json.Unmarshal(res_string, &res)

	assert.Equal(t, rec_dataset1_incomplete, res)

}

type Stream struct {
	name    string
	records []TestRecord
}

var testsStreams = []struct {
	from            string
	streams         []Stream
	expectedStreams StreamsRecord
	test            string
	ok              bool
}{
	{"", []Stream{}, StreamsRecord{[]StreamInfo{}}, "no streams", true},
	{"", []Stream{{"ss1", []TestRecord{rec2, rec1}}},
		StreamsRecord{[]StreamInfo{StreamInfo{Name: "ss1", Timestamp: 0, LastId: 2, TimestampLast: 1}}}, "one stream", true},
	{"", []Stream{{"ss1", []TestRecord{rec2, rec1}},
		{"ss2", []TestRecord{rec2, rec3}}},
		StreamsRecord{[]StreamInfo{StreamInfo{Name: "ss1", Timestamp: 0, LastId: 2, TimestampLast: 1},
			StreamInfo{Name: "ss2", Timestamp: 1, LastId: 3, TimestampLast: 2}}}, "two streams", true},
	{"ss2", []Stream{{"ss1", []TestRecord{rec1, rec2}}, {"ss2", []TestRecord{rec2, rec3}}}, StreamsRecord{[]StreamInfo{StreamInfo{Name: "ss2", Timestamp: 1, LastId: 3, TimestampLast: 2}}}, "with from", true},
	{"", []Stream{{"ss1$", []TestRecord{rec2, rec1}}},
		StreamsRecord{[]StreamInfo{StreamInfo{Name: "ss1$", Timestamp: 0, LastId: 2, TimestampLast: 1}}}, "one stream encoded", true},
	{"ss2$", []Stream{{"ss1$", []TestRecord{rec1, rec2}}, {"ss2$", []TestRecord{rec2, rec3}}}, StreamsRecord{[]StreamInfo{StreamInfo{Name: "ss2$", Timestamp: 1, LastId: 3, TimestampLast: 2}}}, "with from encoded", true},

}

func TestMongoDBListStreams(t *testing.T) {
	for _, test := range testsStreams {
		db.Connect(dbaddress)
		for _, stream := range test.streams {
			for _, rec := range stream.records {
				db.insertRecord(dbname, encodeStringForColName(stream.name), &rec)
			}
		}
		var rec_streams_expect, _ = json.Marshal(test.expectedStreams)

		res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: "0", Op: "streams", ExtraParam: test.from})
		if test.ok {
			assert.Nil(t, err, test.test)
			assert.Equal(t, string(rec_streams_expect), string(res), test.test)
		} else {
			assert.NotNil(t, err, test.test)
		}
		cleanup()
	}
}

func TestMongoDBAckMessage(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec_finished)

	query_str := "{\"Id\":1,\"Op\":\"ackmessage\"}"

	request := Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "ackmessage", ExtraParam: query_str}
	res, err := db.ProcessRequest(request)
	nacks, _ := db.getNacks(request, 0, 0)
	assert.Nil(t, err)
	assert.Equal(t, "", string(res))
	assert.Equal(t, 0, len(nacks))
}

var testsNacs = []struct {
	rangeString   string
	resString     string
	insertRecords bool
	ackRecords    bool
	ok            bool
	test          string
}{
	{"0_0", "{\"unacknowledged\":[1,2,3,4,5,6,7,8,9,10]}", true, false, true, "whole range"},
	{"", "{\"unacknowledged\":[1,2,3,4,5,6,7,8,9,10]}", true, false, false, "empty string range"},
	{"0_5", "{\"unacknowledged\":[1,2,3,4,5]}", true, false, true, "to given"},
	{"5_0", "{\"unacknowledged\":[5,6,7,8,9,10]}", true, false, true, "from given"},
	{"3_7", "{\"unacknowledged\":[3,4,5,6,7]}", true, false, true, "range given"},
	{"1_1", "{\"unacknowledged\":[1]}", true, false, true, "single record"},
	{"3_1", "{\"unacknowledged\":[]}", true, false, false, "to lt from"},
	{"0_0", "{\"unacknowledged\":[]}", false, false, true, "no records"},
	{"0_0", "{\"unacknowledged\":[1,5,6,7,8,9,10]}", true, true, true, "skip acks"},
	{"2_4", "{\"unacknowledged\":[]}", true, true, true, "all acknowledged"},
	{"1_4", "{\"unacknowledged\":[1]}", true, true, true, "some acknowledged"},
}

func TestMongoDBNacks(t *testing.T) {
	for _, test := range testsNacs {
		db.Connect(dbaddress)
		if test.insertRecords {
			insertRecords(10)
			db.insertRecord(dbname, collection, &rec_finished11)
		}
		if test.ackRecords {
			db.ackRecord(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, ExtraParam: "{\"Id\":2,\"Op\":\"ackmessage\"}"})
			db.ackRecord(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, ExtraParam: "{\"Id\":3,\"Op\":\"ackmessage\"}"})
			db.ackRecord(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, ExtraParam: "{\"Id\":4,\"Op\":\"ackmessage\"}"})
		}

		res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "nacks", ExtraParam: test.rangeString})
		if test.ok {
			assert.Nil(t, err, test.test)
			assert.Equal(t, test.resString, string(res), test.test)
		} else {
			assert.NotNil(t, err, test.test)
		}
		cleanup()
	}
}

var testsLastAcs = []struct {
	insertRecords bool
	ackRecords    bool
	resString     string
	test          string
}{
	{false, false, "{\"lastAckId\":0}", "empty db"},
	{true, false, "{\"lastAckId\":0}", "no acks"},
	{true, true, "{\"lastAckId\":4}", "last ack 4"},
}

func TestMongoDBLastAcks(t *testing.T) {
	for _, test := range testsLastAcs {
		db.Connect(dbaddress)
		if test.insertRecords {
			insertRecords(10)
			db.insertRecord(dbname, collection, &rec_finished11)
		}
		if test.ackRecords {
			db.ackRecord(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, ExtraParam: "{\"Id\":2,\"Op\":\"ackmessage\"}"})
			db.ackRecord(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, ExtraParam: "{\"Id\":3,\"Op\":\"ackmessage\"}"})
			db.ackRecord(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, ExtraParam: "{\"Id\":4,\"Op\":\"ackmessage\"}"})
		}

		res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "lastack"})
		assert.Nil(t, err, test.test)
		assert.Equal(t, test.resString, string(res), test.test)
		cleanup()
	}
}

func TestMongoDBGetNextUsesInprocessedImmedeatly(t *testing.T) {
	db.SetSettings(DBSettings{ReadFromInprocessPeriod: 0})
	db.Connect(dbaddress)
	defer cleanup()
	err := db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec2)

	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "0_3"})
	res1, err1 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "0_3"})

	assert.Nil(t, err)
	assert.Nil(t, err1)
	assert.Equal(t, string(rec1_expect), string(res))
	assert.Equal(t, string(rec1_expect), string(res1))
}

func TestMongoDBGetNextUsesInprocessedNumRetry(t *testing.T) {
	db.SetSettings(DBSettings{ReadFromInprocessPeriod: 0})
	db.Connect(dbaddress)
	defer cleanup()
	err := db.insertRecord(dbname, collection, &rec1)
	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "0_1"})
	res1, err1 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "0_1"})
	_, err2 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "0_1"})

	assert.Nil(t, err)
	assert.Nil(t, err1)
	assert.NotNil(t, err2)
	if err2 != nil {
		assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_stream\":\"\"}", err2.Error())
	}
	assert.Equal(t, string(rec1_expect), string(res))
	assert.Equal(t, string(rec1_expect), string(res1))
}

func TestMongoDBGetNextUsesInprocessedAfterTimeout(t *testing.T) {
	db.SetSettings(DBSettings{ReadFromInprocessPeriod: 0})
	db.Connect(dbaddress)
	defer cleanup()
	err := db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec2)
	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "1000_3"})
	res1, err1 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "1000_3"})
	time.Sleep(time.Second)
	res2, err2 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "1000_3"})
	assert.Nil(t, err)
	assert.Nil(t, err1)
	assert.Nil(t, err2)
	assert.Equal(t, string(rec1_expect), string(res))
	assert.Equal(t, string(rec2_expect), string(res1))
	assert.Equal(t, string(rec1_expect), string(res2))
}

func TestMongoDBGetNextReturnsToNormalAfterUsesInprocessed(t *testing.T) {
	db.SetSettings(DBSettings{ReadFromInprocessPeriod: 0})
	db.Connect(dbaddress)
	defer cleanup()
	err := db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec2)
	db.insertRecord(dbname, collection, &rec_finished3)
	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "1000_3"})
	time.Sleep(time.Second)
	res1, err1 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "1000_3"})
	res2, err2 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "1000_3"})
	assert.Nil(t, err)
	assert.Nil(t, err1)
	assert.Nil(t, err2)
	assert.Equal(t, string(rec1_expect), string(res))
	assert.Equal(t, string(rec1_expect), string(res1))
	assert.Equal(t, string(rec2_expect), string(res2))
}

func TestMongoDBGetNextUsesInprocessedImmedeatlyIfFinishedStream(t *testing.T) {
	db.SetSettings(DBSettings{ReadFromInprocessPeriod: 10})
	db.Connect(dbaddress)
	defer cleanup()
	err := db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec_finished)
	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "0_3"})
	res1, err1 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "0_3"})
	assert.Nil(t, err)
	assert.Nil(t, err1)
	assert.Equal(t, string(rec1_expect), string(res))
	assert.Equal(t, string(rec1_expect), string(res1))
}

func TestMongoDBGetNextUsesInprocessedImmedeatlyIfEndofStream(t *testing.T) {
	db.SetSettings(DBSettings{ReadFromInprocessPeriod: 10})
	db.Connect(dbaddress)
	defer cleanup()
	err := db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec2)
	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "0_3"})
	res1, err1 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "0_3"})
	res2, err2 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "0_3"})
	assert.Nil(t, err)
	assert.Nil(t, err1)
	assert.Nil(t, err2)
	assert.Equal(t, string(rec1_expect), string(res))
	assert.Equal(t, string(rec2_expect), string(res1))
	assert.Equal(t, string(rec1_expect), string(res2))
}

func TestMongoDBAckDeletesInprocessed(t *testing.T) {
	db.SetSettings(DBSettings{ReadFromInprocessPeriod: 0})
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "0_3"})
	query_str := "{\"Id\":1,\"Op\":\"ackmessage\"}"

	db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "ackmessage", ExtraParam: query_str})
	_, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "0_3"})
	assert.NotNil(t, err)
	if err != nil {
		assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
		assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_stream\":\"\"}", err.Error())
	}
}

func TestMongoDBAckTwiceErrors(t *testing.T) {
	db.SetSettings(DBSettings{ReadFromInprocessPeriod: 0})
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	query_str := "{\"Id\":1,\"Op\":\"ackmessage\"}"
	db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "ackmessage", ExtraParam: query_str})
	_,err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "ackmessage", ExtraParam: query_str})
	assert.Equal(t, utils.StatusWrongInput, err.(*DBError).Code)
}


func TestMongoDBNegAck(t *testing.T) {
	db.SetSettings(DBSettings{ReadFromInprocessPeriod: 0})
	db.Connect(dbaddress)
	defer cleanup()
	inputParams := struct {
		Id     int
		Params struct {
			DelayMs int
		}
	}{}
	inputParams.Id = 1
	inputParams.Params.DelayMs = 0

	db.insertRecord(dbname, collection, &rec1)
	db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})
	bparam, _ := json.Marshal(&inputParams)

	db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "negackmessage", ExtraParam: string(bparam)})
	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"}) // first time message from negack
	_, err1 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})  // second time nothing
	db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "negackmessage", ExtraParam: string(bparam)})
	_, err2 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next"})  // second time nothing

	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
	assert.NotNil(t, err1)
	assert.Nil(t, err2)

	if err1 != nil {
		assert.Equal(t, utils.StatusNoData, err1.(*DBError).Code)
		assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_stream\":\"\"}", err1.Error())
	}
}

func TestMongoDBGetNextClearsInprocessAfterReset(t *testing.T) {
	db.SetSettings(DBSettings{ReadFromInprocessPeriod: 0})
	db.Connect(dbaddress)
	defer cleanup()
	err := db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec2)
	res, err := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "0_1"})
	res1, err1 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "0_1"})

	db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "resetcounter", ExtraParam: "0"})
	res2, err2 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "0_1"})
	res3, err3 := db.ProcessRequest(Request{DbName: dbname, DbCollectionName: collection, GroupId: groupId, Op: "next", ExtraParam: "0_1"})

	assert.Nil(t, err)
	assert.Nil(t, err1)
	assert.Nil(t, err2)
	assert.Nil(t, err3)
	assert.Equal(t, string(rec1_expect), string(res))
	assert.Equal(t, string(rec1_expect), string(res1))
	assert.Equal(t, string(rec1_expect), string(res2))
	assert.Equal(t, string(rec1_expect), string(res3))
}

var testsDeleteStream = []struct {
	stream  string
	params  string
	ok      bool
	ok2 bool
	message string
}{
	{"test", "{\"ErrorOnNotExist\":true,\"DeleteMeta\":true}", true,false, "delete stream"},
	{"test", "{\"ErrorOnNotExist\":false,\"DeleteMeta\":true}", true, true,"delete stream"},
	{`test$/\  .%&?*#'`, "{\"ErrorOnNotExist\":false,\"DeleteMeta\":true}", true, true,"delete stream"},

}

func TestDeleteStreams(t *testing.T) {
	defer cleanup()
	for _, test := range testsDeleteStream {
		db.Connect(dbaddress)
		db.insertRecord(dbname, encodeStringForColName(test.stream), &rec1)
		db.ProcessRequest(Request{DbName: dbname, DbCollectionName: test.stream, GroupId: "123", Op: "next"})
		query_str := "{\"Id\":1,\"Op\":\"ackmessage\"}"
		request := Request{DbName: dbname, DbCollectionName: test.stream, GroupId: groupId, Op: "ackmessage", ExtraParam: query_str}
		_, err := db.ProcessRequest(request)
		assert.Nil(t, err, test.message)
		_, err = db.ProcessRequest(Request{DbName: dbname, DbCollectionName: test.stream, GroupId: "", Op: "delete_stream", ExtraParam: test.params})
		if test.ok {
			rec, err := streams.getStreams(&db, Request{DbName: dbname, ExtraParam: ""})
			acks_exist,_:= db.collectionExist(Request{DbName: dbname, ExtraParam: ""},acks_collection_name_prefix+test.stream)
			inprocess_exist,_:= db.collectionExist(Request{DbName: dbname, ExtraParam: ""},inprocess_collection_name_prefix+test.stream)
			assert.Equal(t,0,len(rec.Streams),test.message)
			assert.Equal(t,false,acks_exist,test.message)
			assert.Equal(t,false,inprocess_exist,test.message)
			assert.Nil(t, err, test.message)
		} else {
			assert.NotNil(t, err, test.message)
		}
		_, err = db.ProcessRequest(Request{DbName: dbname, DbCollectionName: test.stream, GroupId: "", Op: "delete_stream", ExtraParam: test.params})
		if test.ok2 {
			assert.Nil(t, err, test.message+" 2")
		} else {
			assert.Equal(t, utils.StatusWrongInput, err.(*DBError).Code, test.message+" 2")
		}
	}
}


var testsEncodings = []struct {
	dbname          string
	collection      string
	group			string
	dbname_indb          string
	collection_indb      string
	group_indb			string
	message string
	ok              bool
}{
	{"dbname", "col", "group", "dbname","col","group", "no encoding",true},
	{"dbname"+badSymbolsDb, "col", "group", "dbname"+badSymbolsDbEncoded,"col","group", "symbols in db",true},
	{"dbname", "col"+badSymbolsCol, "group"+badSymbolsCol, "dbname","col"+badSymbolsColEncoded,"group"+badSymbolsColEncoded, "symbols in col",true},
	{"dbname"+badSymbolsDb, "col"+badSymbolsCol, "group"+badSymbolsCol, "dbname"+badSymbolsDbEncoded,"col"+badSymbolsColEncoded,"group"+badSymbolsColEncoded, "symbols in col and db",true},

}

func TestMongoDBEncodingOK(t *testing.T) {
	for _, test := range testsEncodings {
		db.Connect(dbaddress)
		db.insertRecord(test.dbname_indb, test.collection_indb, &rec1)
		res, err := db.ProcessRequest(Request{DbName: test.dbname, DbCollectionName: test.collection, GroupId: test.group, Op: "next"})
		if test.ok {
			assert.Nil(t, err, test.message)
			assert.Equal(t, string(rec1_expect), string(res), test.message)
		} else {
			assert.Equal(t, utils.StatusWrongInput, err.(*DBError).Code, test.message)
		}
		cleanupWithName(test.dbname_indb)
	}
}