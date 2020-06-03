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
	ID   int               `bson:"_id" json:"_id"`
	Meta map[string]string `bson:"meta" json:"meta"`
	Name string            `bson:"name" json:"name"`
}

type TestDataset struct {
	ID     int          `bson:"_id" json:"_id"`
	Size   int          `bson:"size" json:"size"`
	Images []TestRecord `bson:"images" json:"images"`
}

var db Mongodb

const dbname = "run1"
const collection = "substream"
const collection2 = "substream2"
const dbaddress = "127.0.0.1:27017"
const groupId = "bid2a5auidddp1vl71d0"
const metaID = 0
const metaID_str = "0"

var empty_next = map[string]string{"next_substream": ""}

var rec1 = TestRecord{1, empty_next, "aaa"}
var rec_finished = TestRecord{2, map[string]string{"next_substream": "next1"}, finish_substream_keyword}
var rec2 = TestRecord{2, empty_next, "bbb"}
var rec3 = TestRecord{3, empty_next, "ccc"}

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
	db.db_pointers_created = nil
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
	_, err := db.ProcessRequest(dbname, collection, groupId, "next", "")
	assert.Equal(t, utils.StatusServiceUnavailable, err.(*DBError).Code)
}

func TestMongoDBGetMetaErrorWhenNotConnected(t *testing.T) {
	_, err := db.ProcessRequest(dbname, collection, "", "meta", "0")
	assert.Equal(t, utils.StatusServiceUnavailable, err.(*DBError).Code)
}

func TestMongoDBQueryImagesErrorWhenNotConnected(t *testing.T) {
	_, err := db.ProcessRequest(dbname, collection, "", "queryimages", "0")
	assert.Equal(t, utils.StatusServiceUnavailable, err.(*DBError).Code)
}

func TestMongoDBGetNextErrorWhenWrongDatabasename(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	_, err := db.ProcessRequest("", collection, groupId, "next", "")
	assert.Equal(t, utils.StatusWrongInput, err.(*DBError).Code)
}

func TestMongoDBGetNextErrorWhenNonExistingDatacollectionname(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	_, err := db.ProcessRequest(dbname, "bla", groupId, "next", "")
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":0,\"id_max\":0,\"next_substream\":\"\"}", err.Error())
}

func TestMongoDBGetLastErrorWhenNonExistingDatacollectionname(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	_, err := db.ProcessRequest(dbname, "bla", groupId, "last", "")
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":0,\"id_max\":0,\"next_substream\":\"\"}", err.Error())
}

func TestMongoDBGetByIdErrorWhenNonExistingDatacollectionname(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	_, err := db.ProcessRequest(dbname, collection, groupId, "id", "2")

	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":2,\"id_max\":0,\"next_substream\":\"\"}", err.Error())
}


func TestMongoDBGetNextErrorWhenRecordNotThereYet(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec2)
	_, err := db.ProcessRequest(dbname, collection, groupId, "next", "")
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":2,\"next_substream\":\"\"}", err.Error())
}

func TestMongoDBGetNextOK(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	res, err := db.ProcessRequest(dbname, collection, groupId, "next", "")
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBGetNextErrorOnFinishedStream(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec_finished)

	db.ProcessRequest(dbname, collection, groupId, "next", "")
	_, err := db.ProcessRequest(dbname, collection, groupId, "next", "")

	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":2,\"id_max\":2,\"next_substream\":\"next1\"}", err.(*DBError).Message)
}

func TestMongoDBGetNextErrorOnNoMoreData(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.ProcessRequest(dbname, collection, groupId, "next", "")
	_, err := db.ProcessRequest(dbname, collection, groupId, "next", "")

	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":1,\"id_max\":1,\"next_substream\":\"\"}", err.(*DBError).Message)
}

func TestMongoDBGetNextCorrectOrder(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec2)
	db.insertRecord(dbname, collection, &rec1)
	res1, _ := db.ProcessRequest(dbname, collection, groupId, "next", "")
	res2, _ := db.ProcessRequest(dbname, collection, groupId, "next", "")
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
		record.Name = string(ind)
		db.insertRecord(dbname, collection, &record)
	}

}

func getRecords(n int) []int {
	results := make([]int, n)
	var wg sync.WaitGroup
	wg.Add(n)
	for i := 0; i < n; i++ {
		go func() {
			defer wg.Done()
			res_bin, _ := db.ProcessRequest(dbname, collection, groupId, "next", "")
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

func TestMongoDBGetLastAfterErasingDatabase(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	insertRecords(10)
	db.ProcessRequest(dbname, collection, groupId, "next", "0")
	db.dropDatabase(dbname)

	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec2)

	res, err := db.ProcessRequest(dbname, collection, groupId, "last", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(rec2_expect), string(res))
}

func TestMongoDBGetNextAfterErasingDatabase(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	insertRecords(200)
	db.ProcessRequest(dbname, collection, groupId, "next", "0")
	db.dropDatabase(dbname)

	n := 100
	insertRecords(n)
	results := getRecords(n)
	assert.Equal(t, n, getNOnes(results))
}

func TestMongoDBGetNextEmptyAfterErasingDatabase(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	insertRecords(10)
	db.ProcessRequest(dbname, collection, groupId, "next", "0")
	db.dropDatabase(dbname)

	_, err := db.ProcessRequest(dbname, collection, groupId, "next", "0")
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":0,\"id_max\":0,\"next_substream\":\"\"}", err.Error())
}


func TestMongoDBgetRecordByID(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	res, err := db.ProcessRequest(dbname, collection, groupId, "id", "1")
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBgetRecordByIDFails(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	_, err := db.ProcessRequest(dbname, collection, groupId, "id", "2")
	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":2,\"id_max\":1,\"next_substream\":\"\"}", err.Error())
}

func TestMongoDBGetRecordNext(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	res, err := db.ProcessRequest(dbname, collection, groupId, "next", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBGetRecordNextMultipleCollections(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection2, &rec_dataset1)

	res, err := db.ProcessRequest(dbname, collection, groupId, "next", "0")
	res_string, err2 := db.ProcessRequest(dbname, collection2, groupId, "next_dataset", "0")
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
	res, err := db.ProcessRequest(dbname, collection, groupId, "id", "1")
	assert.Nil(t, err)
	assert.Equal(t, string(rec1_expect), string(res))
}

func TestMongoDBWrongOp(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	_, err := db.ProcessRequest(dbname, collection, groupId, "bla", "0")
	assert.NotNil(t, err)
}

func TestMongoDBGetRecordLast(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec2)

	res, err := db.ProcessRequest(dbname, collection, groupId, "last", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(rec2_expect), string(res))
}

func TestMongoDBGetNextAfterGetLastCorrect(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec2)

	res, err := db.ProcessRequest(dbname, collection, groupId, "last", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(rec2_expect), string(res))

	db.insertRecord(dbname, collection, &rec3)

	res, err = db.ProcessRequest(dbname, collection, groupId, "next", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(rec3_expect), string(res))

}

func TestMongoDBGetSize(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec2)
	db.insertRecord(dbname, collection, &rec3)

	res, err := db.ProcessRequest(dbname, collection, "", "size", "0")
	assert.Nil(t, err)
	assert.Equal(t, string(recs1_expect), string(res))
}

func TestMongoDBGetSizeNoRecords(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	res, err := db.ProcessRequest(dbname, collection, "", "size", "0")
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
	_, err := db.ProcessRequest(dbname, collection, "", "id", "2")
	assert.Equal(t, utils.StatusServiceUnavailable, err.(*DBError).Code)
}

func TestMongoDBResetCounter(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec2)

	res1, err1 := db.ProcessRequest(dbname, collection, groupId, "next", "0")

	assert.Nil(t, err1)
	assert.Equal(t, string(rec1_expect), string(res1))

	_, err_reset := db.ProcessRequest(dbname, collection, groupId, "resetcounter", "1")
	assert.Nil(t, err_reset)

	res2, err2 := db.ProcessRequest(dbname, collection, groupId, "next", "0")

	assert.Nil(t, err2)
	assert.Equal(t, string(rec2_expect), string(res2))
}

func TestMongoDBGetMetaOK(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	rec1.ID = metaID
	rec_expect, _ := json.Marshal(rec1)
	db.insertMeta(dbname, &rec1)

	res, err := db.ProcessRequest(dbname, collection, "", "meta", metaID_str)

	assert.Nil(t, err)
	assert.Equal(t, string(rec_expect), string(res))
}

func TestMongoDBGetMetaErr(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	_, err := db.ProcessRequest(dbname, collection, "", "meta", metaID_str)
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

		res_string, err := db.ProcessRequest(dbname, collection, "", "queryimages", test.query)
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

func TestMongoDBQueryImagesOnEmptyDatabase(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	for _, test := range tests {
		res_string, err := db.ProcessRequest(dbname, collection, "", "queryimages", test.query)
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

var rec_dataset1 = TestDataset{1, 3, []TestRecord{rec1, rec2, rec3}}
var rec_dataset1_incomplete = TestDataset{1, 4, []TestRecord{rec1, rec2, rec3}}
var rec_dataset2 = TestDataset{2, 4, []TestRecord{rec1, rec2, rec3}}
var rec_dataset3 = TestDataset{3, 3, []TestRecord{rec3, rec2, rec2}}

func TestMongoDBGetDataset(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, collection, &rec_dataset1)

	res_string, err := db.ProcessRequest(dbname, collection, groupId, "next_dataset", "0")

	assert.Nil(t, err)

	var res TestDataset
	json.Unmarshal(res_string, &res)

	assert.Equal(t, rec_dataset1, res)
}

func TestMongoDBNoDataOnNotCompletedFirstDataset(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, collection, &rec_dataset1_incomplete)

	res_string, err := db.ProcessRequest(dbname, collection, groupId, "next_dataset", "0")

	assert.Equal(t, utils.StatusNoData, err.(*DBError).Code)
	assert.Equal(t, "{\"op\":\"get_record_by_id\",\"id\":0,\"id_max\":0,\"next_substream\":\"\"}", err.(*DBError).Message)

	assert.Equal(t, "", string(res_string))
}

func TestMongoDBGetRecordLastDataSetSkipsIncompleteSets(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, collection, &rec_dataset1)
	db.insertRecord(dbname, collection, &rec_dataset2)

	res_string, err := db.ProcessRequest(dbname, collection, groupId, "last_dataset", "0")

	assert.Nil(t, err)

	var res TestDataset
	json.Unmarshal(res_string, &res)

	assert.Equal(t, rec_dataset1, res)
}

func TestMongoDBGetRecordLastDataSetOK(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()

	db.insertRecord(dbname, collection, &rec_dataset1)
	db.insertRecord(dbname, collection, &rec_dataset3)

	res_string, err := db.ProcessRequest(dbname, collection, groupId, "last_dataset", "0")

	assert.Nil(t, err)

	var res TestDataset
	json.Unmarshal(res_string, &res)

	assert.Equal(t, rec_dataset3, res)
}

func TestMongoDBGetDatasetID(t *testing.T) {
	db.Connect(dbaddress)
	defer cleanup()
	db.insertRecord(dbname, collection, &rec_dataset1)

	res_string, err := db.ProcessRequest(dbname, collection, groupId, "id_dataset", "1")

	assert.Nil(t, err)

	var res TestDataset
	json.Unmarshal(res_string, &res)

	assert.Equal(t, rec_dataset1, res)

}

var testsSubstreams = []struct {
	substreams SubstreamsRecord
	test       string
	ok         bool
}{
	{SubstreamsRecord{[]string{}}, "no substreams", true},
	{SubstreamsRecord{[]string{"ss1"}}, "one substream", true},
	{SubstreamsRecord{[]string{"ss1", "ss2"}}, "two substreams", true},
}

func TestMongoDBListSubstreams(t *testing.T) {
	for _, test := range testsSubstreams {
		db.Connect(dbaddress)
		for _, stream := range test.substreams.Substreams {
			db.insertRecord(dbname, stream, &rec1)
		}
		var rec_substreams_expect, _ = json.Marshal(test.substreams)
		res, err := db.ProcessRequest(dbname, "0", "0", "substreams", "0")
		if test.ok {
			assert.Nil(t, err, test.test)
			assert.Equal(t, string(rec_substreams_expect), string(res), test.test)
		} else {
			assert.NotNil(t, err, test.test)
		}
		cleanup()
	}

}
