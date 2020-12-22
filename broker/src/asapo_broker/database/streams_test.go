// +build integration_tests

package database

import (
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
	streams.records= map[string]StreamsRecord{}
}

func TestStreamsTestSuite(t *testing.T) {
	suite.Run(t, new(StreamsTestSuite))
}

func (suite *StreamsTestSuite) TestStreamsEmpty() {
	rec, err := streams.getStreams(&db, "test", "")
	suite.Nil(err)
	suite.Empty(rec.Streams, 0)
}

func (suite *StreamsTestSuite) TestStreamsNotUsesCacheWhenEmpty() {
	db.settings.UpdateStreamCachePeriodMs = 1000
	streams.getStreams(&db, dbname, "")
	db.insertRecord(dbname, collection, &rec1)
	rec, err := streams.getStreams(&db, dbname, "")
	suite.Nil(err)
	suite.Equal(1, len(rec.Streams))
}

func (suite *StreamsTestSuite) TestStreamsUsesCache() {
	db.settings.UpdateStreamCachePeriodMs = 1000
	db.insertRecord(dbname, collection, &rec2)
	streams.getStreams(&db, dbname, "")
	db.insertRecord(dbname, collection, &rec1)
	rec, err := streams.getStreams(&db, dbname, "")
	suite.Nil(err)
	suite.Equal(int64(1), rec.Streams[0].Timestamp)
}

func (suite *StreamsTestSuite) TestStreamsNotUsesCacheWhenExpired() {
	db.settings.UpdateStreamCachePeriodMs = 10
	db.insertRecord(dbname, collection, &rec2)
	streams.getStreams(&db, dbname, "")
	db.insertRecord(dbname, collection, &rec1)
	time.Sleep(time.Millisecond * 100)
	rec, err := streams.getStreams(&db, dbname, "")
	suite.Nil(err)
	suite.Equal(int64(1), rec.Streams[0].Timestamp)
}

func (suite *StreamsTestSuite) TestStreamRemovesDatabase() {
	db.settings.UpdateStreamCachePeriodMs = 0
	db.insertRecord(dbname, collection, &rec1)
	streams.getStreams(&db, dbname, "")
	db.dropDatabase(dbname)
	rec, err := streams.getStreams(&db, dbname, "")
	suite.Nil(err)
	suite.Empty(rec.Streams, 0)
}
