// +build integration_tests

package database

import (
	"github.com/stretchr/testify/suite"
	"testing"
	"time"
)

type SubstreamsTestSuite struct {
	suite.Suite
}

func (suite *SubstreamsTestSuite) SetupTest() {
	db.Connect(dbaddress)
}

func (suite *SubstreamsTestSuite) TearDownTest() {
	cleanup()
	substreams.records= map[string]SubstreamsRecord{}
}

func TestSubstreamsTestSuite(t *testing.T) {
	suite.Run(t, new(SubstreamsTestSuite))
}

func (suite *SubstreamsTestSuite) TestSubstreamsEmpty() {
	rec, err := substreams.getSubstreams(&db, "test", "")
	suite.Nil(err)
	suite.Empty(rec.Substreams, 0)
}

func (suite *SubstreamsTestSuite) TestSubstreamsNotUsesCacheWhenEmpty() {
	db.settings.UpdateSubstreamCachePeriodMs = 1000
	substreams.getSubstreams(&db, dbname, "")
	db.insertRecord(dbname, collection, &rec1)
	rec, err := substreams.getSubstreams(&db, dbname, "")
	suite.Nil(err)
	suite.Equal(1, len(rec.Substreams))
}

func (suite *SubstreamsTestSuite) TestSubstreamsUsesCache() {
	db.settings.UpdateSubstreamCachePeriodMs = 1000
	db.insertRecord(dbname, collection, &rec2)
	substreams.getSubstreams(&db, dbname, "")
	db.insertRecord(dbname, collection, &rec1)
	rec, err := substreams.getSubstreams(&db, dbname, "")
	suite.Nil(err)
	suite.Equal(int64(1), rec.Substreams[0].Timestamp)
}

func (suite *SubstreamsTestSuite) TestSubstreamsNotUsesCacheWhenExpired() {
	db.settings.UpdateSubstreamCachePeriodMs = 10
	db.insertRecord(dbname, collection, &rec2)
	substreams.getSubstreams(&db, dbname, "")
	db.insertRecord(dbname, collection, &rec1)
	time.Sleep(time.Millisecond * 100)
	rec, err := substreams.getSubstreams(&db, dbname, "")
	suite.Nil(err)
	suite.Equal(int64(1), rec.Substreams[0].Timestamp)
}

func (suite *SubstreamsTestSuite) TestSubstreamRemovesDatabase() {
	db.settings.UpdateSubstreamCachePeriodMs = 0
	db.insertRecord(dbname, collection, &rec1)
	substreams.getSubstreams(&db, dbname, "")
	db.dropDatabase(dbname)
	rec, err := substreams.getSubstreams(&db, dbname, "")
	suite.Nil(err)
	suite.Empty(rec.Substreams, 0)
}
