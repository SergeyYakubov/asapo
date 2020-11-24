//+build !test

package database

import (
	"asapo_common/utils"
	"context"
	"errors"
	"go.mongodb.org/mongo-driver/bson"
	"sort"
	"strings"
	"sync"
	"time"
)

type SubstreamInfo struct {
	Name      string `json:"name"`
	Timestamp int64  `json:"timestampCreated"`
}

type SubstreamsRecord struct {
	Substreams []SubstreamInfo `json:"substreams"`
}

type Substreams struct {
	records     map[string]SubstreamsRecord
	lastUpdated int64
}

var substreams = Substreams{lastUpdated: 0, records: make(map[string]SubstreamsRecord, 0)}
var substreamsLock sync.Mutex

func (ss *Substreams) tryGetFromCache(db_name string, updatePeriodMs int) (SubstreamsRecord, error) {
	if ss.lastUpdated < time.Now().UnixNano()-int64(updatePeriodMs*1000000) {
		return SubstreamsRecord{}, errors.New("cache expired")
	}
	rec, ok := ss.records[db_name]
	if !ok {
		return SubstreamsRecord{}, errors.New("no records for " + db_name)
	}
	return rec, nil
}

func readSubstreams(db *Mongodb, db_name string) (SubstreamsRecord, error) {
	database := db.client.Database(db_name)
	result, err := database.ListCollectionNames(context.TODO(), bson.D{})
	if err != nil {
		return SubstreamsRecord{}, err
	}
	var rec = SubstreamsRecord{[]SubstreamInfo{}}
	for _, coll := range result {
		if strings.HasPrefix(coll, data_collection_name_prefix) {
			si := SubstreamInfo{Name: strings.TrimPrefix(coll, data_collection_name_prefix)}
			rec.Substreams = append(rec.Substreams, si)
		}
	}
	return rec, nil
}

func updateTimestamps(db *Mongodb, db_name string, rec *SubstreamsRecord) {
	ss,dbFound :=substreams.records[db_name]
	currentSubstreams := []SubstreamInfo{}
	if dbFound {
		// sort substreams by name
		currentSubstreams=ss.Substreams
		sort.Slice(currentSubstreams,func(i, j int) bool {
			return currentSubstreams[i].Name>=currentSubstreams[j].Name
		})
	}
	for i, record := range rec.Substreams {
		ind := sort.Search(len(currentSubstreams),func(i int) bool {
			return currentSubstreams[i].Name>=record.Name
		})
		if ind < len(currentSubstreams) && currentSubstreams[ind].Name == record.Name { // record found, just skip it
			rec.Substreams[i].Timestamp = currentSubstreams[ind].Timestamp
			continue
		}
		res, err := db.getEarliestRecord(db_name, record.Name)
		if err == nil {
			ts,ok:=utils.InterfaceToInt64(res["timestamp"])
			if ok {
				rec.Substreams[i].Timestamp = ts
			}
		}
	}
}

func sortRecords(rec *SubstreamsRecord) {
	sort.Slice(rec.Substreams[:], func(i, j int) bool {
		return rec.Substreams[i].Timestamp < rec.Substreams[j].Timestamp
	})
}

func (ss *Substreams) updateFromDb(db *Mongodb, db_name string) (SubstreamsRecord, error) {
	rec, err := readSubstreams(db, db_name)
	if err != nil {
		return SubstreamsRecord{}, err
	}
	updateTimestamps(db, db_name, &rec)
	sortRecords(&rec)
	if len(rec.Substreams)>0 {
		ss.records[db_name] = rec
		ss.lastUpdated = time.Now().UnixNano()
	}
	return rec, nil
}

func (ss *Substreams) getSubstreams(db *Mongodb, db_name string, from string) (SubstreamsRecord, error) {
	substreamsLock.Lock()
	rec, err := ss.tryGetFromCache(db_name,db.settings.UpdateSubstreamCachePeriodMs)
	if err != nil {
		rec, err = ss.updateFromDb(db, db_name)
	}
	substreamsLock.Unlock()
	if err != nil {
		return SubstreamsRecord{}, err
	}

	if from != "" {
		ind := len(rec.Substreams)
		for i, rec := range rec.Substreams {
			if rec.Name == from {
				ind = i
				break
			}
		}
		rec.Substreams = rec.Substreams[ind:]
	}
	return rec, nil
}
