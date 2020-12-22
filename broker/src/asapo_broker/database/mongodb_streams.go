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

type StreamInfo struct {
	Name      string `json:"name"`
	Timestamp int64  `json:"timestampCreated"`
}

type StreamsRecord struct {
	Streams []StreamInfo `json:"streams"`
}

type Streams struct {
	records     map[string]StreamsRecord
	lastUpdated int64
}

var streams = Streams{lastUpdated: 0, records: make(map[string]StreamsRecord, 0)}
var streamsLock sync.Mutex

func (ss *Streams) tryGetFromCache(db_name string, updatePeriodMs int) (StreamsRecord, error) {
	if ss.lastUpdated < time.Now().UnixNano()-int64(updatePeriodMs*1000000) {
		return StreamsRecord{}, errors.New("cache expired")
	}
	rec, ok := ss.records[db_name]
	if !ok {
		return StreamsRecord{}, errors.New("no records for " + db_name)
	}
	return rec, nil
}

func readStreams(db *Mongodb, db_name string) (StreamsRecord, error) {
	database := db.client.Database(db_name)
	result, err := database.ListCollectionNames(context.TODO(), bson.D{})
	if err != nil {
		return StreamsRecord{}, err
	}
	var rec = StreamsRecord{[]StreamInfo{}}
	for _, coll := range result {
		if strings.HasPrefix(coll, data_collection_name_prefix) {
			si := StreamInfo{Name: strings.TrimPrefix(coll, data_collection_name_prefix)}
			rec.Streams = append(rec.Streams, si)
		}
	}
	return rec, nil
}

func updateTimestamps(db *Mongodb, db_name string, rec *StreamsRecord) {
	ss,dbFound :=streams.records[db_name]
	currentStreams := []StreamInfo{}
	if dbFound {
		// sort streams by name
		currentStreams=ss.Streams
		sort.Slice(currentStreams,func(i, j int) bool {
			return currentStreams[i].Name>=currentStreams[j].Name
		})
	}
	for i, record := range rec.Streams {
		ind := sort.Search(len(currentStreams),func(i int) bool {
			return currentStreams[i].Name>=record.Name
		})
		if ind < len(currentStreams) && currentStreams[ind].Name == record.Name { // record found, just skip it
			rec.Streams[i].Timestamp = currentStreams[ind].Timestamp
			continue
		}
		res, err := db.getEarliestRecord(db_name, record.Name)
		if err == nil {
			ts,ok:=utils.InterfaceToInt64(res["timestamp"])
			if ok {
				rec.Streams[i].Timestamp = ts
			}
		}
	}
}

func sortRecords(rec *StreamsRecord) {
	sort.Slice(rec.Streams[:], func(i, j int) bool {
		return rec.Streams[i].Timestamp < rec.Streams[j].Timestamp
	})
}

func (ss *Streams) updateFromDb(db *Mongodb, db_name string) (StreamsRecord, error) {
	rec, err := readStreams(db, db_name)
	if err != nil {
		return StreamsRecord{}, err
	}
	updateTimestamps(db, db_name, &rec)
	sortRecords(&rec)
	if len(rec.Streams)>0 {
		ss.records[db_name] = rec
		ss.lastUpdated = time.Now().UnixNano()
	}
	return rec, nil
}

func (ss *Streams) getStreams(db *Mongodb, db_name string, from string) (StreamsRecord, error) {
	streamsLock.Lock()
	rec, err := ss.tryGetFromCache(db_name,db.settings.UpdateStreamCachePeriodMs)
	if err != nil {
		rec, err = ss.updateFromDb(db, db_name)
	}
	streamsLock.Unlock()
	if err != nil {
		return StreamsRecord{}, err
	}

	if from != "" {
		ind := len(rec.Streams)
		for i, rec := range rec.Streams {
			if rec.Name == from {
				ind = i
				break
			}
		}
		rec.Streams = rec.Streams[ind:]
	}
	return rec, nil
}
