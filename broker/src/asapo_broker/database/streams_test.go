// +build integration_tests

package database

import (
	"asapo_common/utils"
	"fmt"
	"github.com/stretchr/testify/suite"
	"testing"
	"time"
)

type StreamsTestSuite struct {
	suite.Suite
}

func (suite *StreamsTestSuite) SetupTest() {
	db.Connect(dbaddress)
}

func (suite *StreamsTestSuite) TearDownTest() {
	cleanup()
	streams.records = map[string]StreamsRecord{}
}

func TestStreamsTestSuite(t *testing.T) {
	suite.Run(t, new(StreamsTestSuite))
}

func (suite *StreamsTestSuite) TestStreamsEmpty() {
	rec, err := streams.getStreams(&db, Request{DbName: "test", ExtraParam: ""})
	suite.Nil(err)
	suite.Empty(rec.Streams, 0)
}

func (suite *StreamsTestSuite) TestStreamsNotUsesCacheWhenEmpty() {
	db.settings.UpdateStreamCachePeriodMs = 1000
	streams.getStreams(&db, Request{DbName: dbname, ExtraParam: ""})
	db.insertRecord(dbname, collection, &rec1)
	rec, err := streams.getStreams(&db, Request{DbName: dbname, ExtraParam: ""})
	suite.Nil(err)
	suite.Equal(1, len(rec.Streams))
}

func (suite *StreamsTestSuite) TestStreamsUsesCache() {
	db.settings.UpdateStreamCachePeriodMs = 1000
	db.insertRecord(dbname, collection, &rec2)
	streams.getStreams(&db, Request{DbName: dbname, ExtraParam: ""})
	db.insertRecord(dbname, collection, &rec1)
	rec, err := streams.getStreams(&db, Request{DbName: dbname, ExtraParam: ""})
	suite.Nil(err)
	suite.Equal(int64(1), rec.Streams[0].Timestamp)
	suite.Equal(false, rec.Streams[0].Finished)
	suite.Equal(int64(2), rec.Streams[0].LastId)
	suite.Equal(int64(1), rec.Streams[0].TimestampLast)
}

func (suite *StreamsTestSuite) TestStreamsCacheexpires() {
	db.settings.UpdateStreamCachePeriodMs = 100
	var res1 StreamsRecord
	go func() {
		db.insertRecord(dbname, collection, &rec1)
		streams.getStreams(&db, Request{DbName: dbname, ExtraParam: ""})
		db.insertRecord(dbname, collection, &rec_finished)
		res1,_ = streams.getStreams(&db, Request{DbName: dbname, ExtraParam: ""})
	}()
	db.insertRecord(dbname, collection+"1", &rec1_later)
	res2,_ := streams.getStreams(&db, Request{DbName: dbname, ExtraParam: ""})
	db.insertRecord(dbname, collection+"1", &rec_finished)
	time.Sleep(time.Second)
	res3, err := streams.getStreams(&db, Request{DbName: dbname, ExtraParam: ""})
	suite.Nil(err)
	suite.Equal(true, res3.Streams[0].Finished)
	fmt.Println(res1,res2)
//	suite.Equal(true, rec.Streams[1].Finished)
}


func (suite *StreamsTestSuite) TestStreamsGetFinishedInfo() {
	db.settings.UpdateStreamCachePeriodMs = 1000
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection, &rec_finished)
	rec, err := streams.getStreams(&db, Request{DbName: dbname, ExtraParam: ""})
	suite.Nil(err)
	suite.Equal(int64(0), rec.Streams[0].Timestamp)
	suite.Equal(true, rec.Streams[0].Finished)
	suite.Equal("next1", rec.Streams[0].NextStream)
}


func (suite *StreamsTestSuite) TestStreamsDataSetsGetFinishedInfo() {
	db.settings.UpdateStreamCachePeriodMs = 1000
	db.insertRecord(dbname, collection, &rec_dataset1_incomplete)
	db.insertRecord(dbname, collection, &rec_finished)
	rec, err := streams.getStreams(&db, Request{DbName: dbname, ExtraParam: ""})
	suite.Nil(err)
	suite.Equal(int64(1), rec.Streams[0].Timestamp)
	suite.Equal(int64(2), rec.Streams[0].TimestampLast)
	suite.Equal(true, rec.Streams[0].Finished)
	suite.Equal("next1", rec.Streams[0].NextStream)
	suite.Equal(int64(1), rec.Streams[0].LastId)
}

func (suite *StreamsTestSuite) TestStreamsMultipleRequests() {
	db.settings.UpdateStreamCachePeriodMs = 1000
	db.insertRecord(dbname, collection, &rec_dataset1_incomplete)
	db.insertRecord(dbname, collection, &rec_finished)
	db.insertRecord(dbname, collection2, &rec_dataset1_incomplete)
	rec, err := streams.getStreams(&db, Request{DbName: dbname, ExtraParam: "0/unfinished"})
	rec2, err2 := streams.getStreams(&db, Request{DbName: dbname, ExtraParam: "0/finished"})
	suite.Nil(err)
	suite.Equal(collection2, rec.Streams[0].Name)
	suite.Equal(1, len(rec.Streams))
	suite.Nil(err2)
	suite.Equal(1, len(rec2.Streams))
	suite.Equal(collection, rec2.Streams[0].Name)
}

func (suite *StreamsTestSuite) TestStreamsNotUsesCacheWhenExpired() {
	db.settings.UpdateStreamCachePeriodMs = 10
	db.insertRecord(dbname, collection, &rec2)
	streams.getStreams(&db, Request{DbName: dbname, ExtraParam: ""})
	db.insertRecord(dbname, collection, &rec1)
	time.Sleep(time.Millisecond * 100)
	rec, err := streams.getStreams(&db, Request{DbName: dbname, ExtraParam: ""})
	suite.Nil(err)
	suite.Equal(int64(1), rec.Streams[0].Timestamp)
}

func (suite *StreamsTestSuite) TestStreamRemovesDatabase() {
	db.settings.UpdateStreamCachePeriodMs = 0
	db.insertRecord(dbname, collection, &rec1)
	streams.getStreams(&db, Request{DbName: dbname, ExtraParam: ""})
	db.dropDatabase(dbname)
	rec, err := streams.getStreams(&db, Request{DbName: dbname, ExtraParam: ""})
	suite.Nil(err)
	suite.Empty(rec.Streams, 0)
}

var streamFilterTests=[]struct{
	request Request
	error bool
	streams []string
	message string
}{
	{request: Request{DbName:dbname, ExtraParam:""},error: false,streams: []string{collection,collection2},message: "default all streams"},
	{request: Request{DbName:dbname, ExtraParam:"0/"},error: false,streams: []string{collection,collection2},message: "default 0/ all streams"},
	{request: Request{DbName:dbname, ExtraParam:utils.EncodeTwoStrings(collection,"")},error: false,streams: []string{collection,collection2},message: "first parameter only -  all streams"},
	{request: Request{DbName:dbname, ExtraParam:"0/all"},error: false,streams: []string{collection,collection2},message: "second parameter only -  all streams"},
	{request: Request{DbName:dbname, ExtraParam:"0/finished"},error: false,streams: []string{collection2},message: "second parameter only -  finished streams"},
	{request: Request{DbName:dbname, ExtraParam:"0/unfinished"},error: false,streams: []string{collection},message: "second parameter only -  unfinished streams"},
	{request: Request{DbName:dbname, ExtraParam:utils.EncodeTwoStrings(collection2,"all")},error: false,streams: []string{collection2},message: "from stream2"},
	{request: Request{DbName:dbname, ExtraParam:utils.EncodeTwoStrings(collection2,"unfinished")},error: false,streams: []string{},message: "from stream2 and filter"},
	{request: Request{DbName:dbname, ExtraParam:utils.EncodeTwoStrings(collection2,"bla")},error: true,streams: []string{},message: "wrong filter"},
	{request: Request{DbName:dbname, ExtraParam:utils.EncodeTwoStrings(collection2,"all_aaa")},error: true,streams: []string{},message: "wrong filter2"},
	{request: Request{DbName:dbname, ExtraParam:utils.EncodeTwoStrings("blabla","")},error: false,streams: []string{},message: "from unknown stream returns nothing"},
	{request: Request{DbName:dbname, ExtraParam:utils.EncodeTwoStrings(collection2,"")},error: false,streams: []string{collection2},message: "from stream2, first parameter only"},
}

func (suite *StreamsTestSuite) TestStreamFilters() {
	db.insertRecord(dbname, collection, &rec1)
	db.insertRecord(dbname, collection2, &rec1_later)
	db.insertRecord(dbname, collection2, &rec_finished)
	for _, test := range streamFilterTests {
		rec, err := streams.getStreams(&db, test.request)
		if test.error {
			suite.NotNil(err,test.message)
			continue
		}
		if err!=nil {
			fmt.Println(err.Error())
		}
		streams:=make([]string,0)
		for _,si:=range rec.Streams {
			streams=append(streams,si.Name)
		}
		suite.Equal(test.streams,streams,test.message)
	}
}