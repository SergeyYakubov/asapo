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
	LastId        int64  `json:"lastId"`
	Name          string `json:"name"`
	Timestamp     int64  `json:"timestampCreated"`
	TimestampLast int64  `json:"timestampLast"`
	Finished      bool   `json:"finished"`
	NextStream    string `json:"nextStream"`
}

type StreamsRecord struct {
	Streams []StreamInfo `json:"streams"`
}

type Streams struct {
	records     map[string]StreamsRecord
	lastUpdated map[string]time.Time
	lastSynced  map[string]time.Time
}

var streams = Streams{lastSynced: make(map[string]time.Time, 0),lastUpdated: make(map[string]time.Time, 0), records: make(map[string]StreamsRecord, 0)}
var streamsLock sync.Mutex

func (ss *Streams) tryGetFromCache(db_name string, updatePeriodMs int) (StreamsRecord, error) {
	if time.Now().Sub(ss.lastUpdated[db_name]).Milliseconds() > int64(updatePeriodMs) {
		return StreamsRecord{}, errors.New("cache expired")
	}
	rec, ok := ss.records[db_name]
	if !ok {
		return StreamsRecord{}, errors.New("no records for " + db_name)
	}
	res :=StreamsRecord{}
	utils.DeepCopy(rec,&res)
	return res, nil
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
			sNameEncoded:= strings.TrimPrefix(coll, data_collection_name_prefix)
			si := StreamInfo{Name: decodeString(sNameEncoded)}
			rec.Streams = append(rec.Streams, si)
		}
	}
	return rec, nil
}

func getCurrentStreams(db_name string) []StreamInfo {
	ss, dbFound := streams.records[db_name]
	currentStreams := []StreamInfo{}
	if dbFound {
		// sort streams by name
		currentStreams = ss.Streams
		sort.Slice(currentStreams, func(i, j int) bool {
			return currentStreams[i].Name >= currentStreams[j].Name
		})
	}
	return currentStreams
}

func findStreamAmongCurrent(currentStreams []StreamInfo, record StreamInfo) (int, bool) {
	ind := sort.Search(len(currentStreams), func(i int) bool {
		return currentStreams[i].Name >= record.Name
	})
	if ind < len(currentStreams) && currentStreams[ind].Name == record.Name {
		return ind, true
	}
	return -1, false
}

func fillInfoFromEarliestRecord(db *Mongodb, db_name string, rec *StreamsRecord, record StreamInfo, i int) error {
	res, err := db.getEarliestRawRecord(db_name, encodeStringForColName(record.Name))
	if err != nil {
		return err
	}
	ts, ok := utils.GetInt64FromMap(res, "timestamp")
	if ok {
		rec.Streams[i].Timestamp = ts
	} else {
		return errors.New("fillInfoFromEarliestRecord: cannot extact timestamp")
	}
	return nil
}

func fillInfoFromLastRecord(db *Mongodb, db_name string, rec *StreamsRecord, record StreamInfo, i int) error {
	res, err := db.getLastRawRecord(db_name, encodeStringForColName(record.Name))
	if err != nil {
		return err
	}
	mrec, ok := ExtractMessageRecord(res)
	if !ok {
		return errors.New("fillInfoFromLastRecord: cannot extract record")
	}

	rec.Streams[i].LastId = int64(mrec.ID)
	rec.Streams[i].TimestampLast = int64(mrec.Timestamp)
	rec.Streams[i].Finished = mrec.FinishedStream
	if mrec.FinishedStream {
		rec.Streams[i].LastId = rec.Streams[i].LastId - 1
		if mrec.NextStream != no_next_stream_keyword {
			rec.Streams[i].NextStream = mrec.NextStream
		}
	}
	return nil
}

func updateStreamInfofromCurrent(currentStreams []StreamInfo, record StreamInfo, rec *StreamInfo) (found bool, updateFinished bool) {
	ind, found := findStreamAmongCurrent(currentStreams, record)
	if found {
		*rec = currentStreams[ind]
		if currentStreams[ind].Finished {
			return found, true
		}
	}
	return found, false
}

func updateStreamInfos(db *Mongodb, db_name string, rec *StreamsRecord,forceSync bool) error {
	currentStreams := getCurrentStreams(db_name)
	for i, record := range rec.Streams {
		found, mayContinue := updateStreamInfofromCurrent(currentStreams, record, &rec.Streams[i])
		if mayContinue && !forceSync {
			continue
		}
		if !found || forceSync { // set timestamp
			if err := fillInfoFromEarliestRecord(db, db_name, rec, record, i); err != nil {
				return err
			}
		}
		if err := fillInfoFromLastRecord(db, db_name, rec, record, i); err != nil { // update last record (timestamp, stream finished flag)
			return err
		}
	}
	return nil
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

	forceSync:= false
	if time.Now().Sub(ss.lastSynced[db_name]).Seconds() > 5 {
		forceSync = true
	}
	err = updateStreamInfos(db, db_name, &rec,forceSync)
	if err != nil {
		return StreamsRecord{}, err
	}

	if forceSync {
		ss.lastSynced[db_name] = time.Now()
	}

	sortRecords(&rec)
	if len(rec.Streams) > 0 {
		res :=StreamsRecord{}
		utils.DeepCopy(rec,&res)
		ss.records[db_name] = res
		ss.lastUpdated[db_name] = time.Now()
	}
	return rec, nil
}

func getFiltersFromString(filterString string) (string, string, error) {
	firstStream, streamStatus, err := utils.DecodeTwoStrings(filterString)
	if err!=nil {
		return "", "", errors.New("wrong format: " + filterString)
	}
	if streamStatus == "" {
		streamStatus = stream_filter_all
	}
	return firstStream, streamStatus, nil
}

func getStreamsParamsFromRequest(request Request) (string, string, error) {
	if request.ExtraParam == "" {
		return "", stream_filter_all, nil
	}

	firstStream, streamStatus, err := getFiltersFromString(request.ExtraParam)
	if err != nil {
		return "", "", err
	}

	err = checkStreamstreamStatus(streamStatus)
	if err != nil {
		return "", "", err
	}

	return firstStream, streamStatus, nil
}

func checkStreamstreamStatus(streamStatus string) error {
	if !utils.StringInSlice(streamStatus, []string{stream_filter_all, stream_filter_finished, stream_filter_unfinished}) {
		return errors.New("getStreamsParamsFromRequest: wrong streamStatus " + streamStatus)
	}
	return nil
}

func keepStream(rec StreamInfo, streamStatus string) bool {
	return (rec.Finished && streamStatus == stream_filter_finished) || (!rec.Finished && streamStatus == stream_filter_unfinished)
}

func filterStreams(rec StreamsRecord, firstStream string, streamStatus string) []StreamInfo {
	limitedStreams := limitStreams(rec, firstStream)

	if streamStatus == stream_filter_all {
		return limitedStreams
	}
	nextStreams := limitedStreams[:0]
	for _, rec := range limitedStreams {
		if keepStream(rec, streamStatus) {
			nextStreams = append(nextStreams, rec)
		}
	}
	return nextStreams
}

func limitStreams(rec StreamsRecord, firstStream string) []StreamInfo {
	if firstStream != "" {
		ind := len(rec.Streams)
		for i, rec := range rec.Streams {
			if rec.Name == firstStream {
				ind = i
				break
			}
		}
		rec.Streams = rec.Streams[ind:]
	}
	return rec.Streams
}

func (ss *Streams) getStreams(db *Mongodb, request Request) (StreamsRecord, error) {
	firstStream, streamStatus, err := getStreamsParamsFromRequest(request)
	if err != nil {
		return StreamsRecord{}, err
	}

	streamsLock.Lock()
	rec, err := ss.tryGetFromCache(request.DbName(), db.settings.UpdateStreamCachePeriodMs)
	if err != nil {
		rec, err = ss.updateFromDb(db, request.DbName())
	}
	streamsLock.Unlock()
	if err != nil {
		return StreamsRecord{}, err
	}

	rec.Streams = filterStreams(rec, firstStream, streamStatus)

	return rec, nil
}
