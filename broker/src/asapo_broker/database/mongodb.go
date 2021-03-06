//go:build !test
// +build !test

package database

import (
	log "asapo_common/logger"
	"asapo_common/utils"
	"context"
	"encoding/json"
	"errors"
	"go.mongodb.org/mongo-driver/bson"
	"go.mongodb.org/mongo-driver/bson/primitive"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
	"math"
	"strconv"
	"strings"
	"sync"
	"time"
)

type ID struct {
	ID int `bson:"_id"`
}

type MessageRecord struct {
	ID             int                    `json:"_id"`
	Timestamp      int                    `bson:"timestamp" json:"timestamp"`
	Name           string                 `json:"name"`
	Meta           map[string]interface{} `json:"meta"`
	NextStream     string
	FinishedStream bool
}

type InProcessingRecord struct {
	ID                int   `bson:"_id" json:"_id"`
	MaxResendAttempts int   `bson:"maxResendAttempts" json:"maxResendAttempts"`
	ResendAttempts    int   `bson:"resendAttempts" json:"resendAttempts"`
	DelayMs           int64 `bson:"delayMs" json:"delayMs"`
}

type NegAckParamsRecord struct {
	ID                int   `bson:"_id" json:"_id"`
	MaxResendAttempts int   `bson:"maxResendAttempts" json:"maxResendAttempts"`
	ResendAttempts    int   `bson:"resendAttempts" json:"resendAttempts"`
	DelayMs           int64 `bson:"delayMs" json:"delayMs"`
}

type Nacks struct {
	Unacknowledged []int `json:"unacknowledged"`
}

type LastAck struct {
	ID int `bson:"_id" json:"lastAckId"`
}

type LocationPointer struct {
	GroupID string `bson:"_id"`
	Value   int    `bson:"current_pointer"`
}

const data_collection_name_prefix = "data_"
const acks_collection_name_prefix = "acks_"
const inprocess_collection_name_prefix = "inprocess_"
const meta_collection_name = "meta"
const pointer_collection_name = "current_location"
const pointer_field_name = "current_pointer"
const last_message_collection_name = "last_messages"
const last_message_field_name = "last_message"

const no_session_msg = "database client not created"
const already_connected_msg = "already connected"

const finish_stream_keyword = "asapo_finish_stream"
const no_next_stream_keyword = "asapo_no_next"
const stream_filter_all = "all"
const stream_filter_finished = "finished"
const stream_filter_unfinished = "unfinished"

const (
	field_op_inc int = iota
	field_op_set
)

type fieldChangeRequest struct {
	collectionName string
	fieldName      string
	op             int
	max_ind        int
	val            int
}

var dbSessionLock sync.Mutex
var dbClientLock sync.RWMutex

type SizeRecord struct {
	Size int `bson:"size" json:"size"`
}

type Mongodb struct {
	client                *mongo.Client
	timeout               time.Duration
	settings              DBSettings
	lastReadFromInprocess map[string]int64
}

func (db *Mongodb) SetSettings(settings DBSettings) {
	db.settings = settings
}

func (db *Mongodb) Ping() (err error) {
	if db.client == nil {
		return &DBError{utils.StatusServiceUnavailable, no_session_msg}
	}
	ctx, cancel := context.WithTimeout(context.Background(), 2*time.Second)
	defer cancel()
	return db.client.Ping(ctx, nil)
}

func (db *Mongodb) Connect(address string) (err error) {
	dbClientLock.Lock()
	defer dbClientLock.Unlock()

	if db.client != nil {
		return &DBError{utils.StatusServiceUnavailable, already_connected_msg}
	}
	db.client, err = mongo.NewClient(options.Client().SetConnectTimeout(20 * time.Second).ApplyURI("mongodb://" + address))
	if err != nil {
		return err
	}
	ctx, cancel := context.WithTimeout(context.Background(), 20*time.Second)
	defer cancel()
	err = db.client.Connect(ctx)
	if err != nil {
		db.client = nil
		return err
	}

	if db.lastReadFromInprocess == nil {
		db.lastReadFromInprocess = make(map[string]int64, 100)
	}

	return db.Ping()
}

func (db *Mongodb) Close() {
	dbClientLock.Lock()
	defer dbClientLock.Unlock()
	if db.client != nil {
		ctx, cancel := context.WithTimeout(context.Background(), 2*time.Second)
		defer cancel()
		db.client.Disconnect(ctx)
		db.client = nil
	}
}

func (db *Mongodb) dropDatabase(dbname string) (err error) {
	if db.client == nil {
		return &DBError{utils.StatusServiceUnavailable, no_session_msg}
	}
	return db.client.Database(dbname).Drop(context.TODO())
}

func (db *Mongodb) insertRecord(dbname string, collection_name string, s interface{}) error {
	if db.client == nil {
		return &DBError{utils.StatusServiceUnavailable, no_session_msg}
	}

	c := db.client.Database(dbname).Collection(data_collection_name_prefix + collection_name)

	_, err := c.InsertOne(context.TODO(), s)
	return err
}

func (db *Mongodb) insertMeta(dbname string, s interface{}) error {
	if db.client == nil {
		return &DBError{utils.StatusServiceUnavailable, no_session_msg}
	}

	c := db.client.Database(dbname).Collection(meta_collection_name)

	_, err := c.InsertOne(context.TODO(), s)
	return err
}

func maxIndexQuery(request Request, returnIncompete bool) bson.M {
	var q bson.M
	if request.DatasetOp && !returnIncompete {
		if request.MinDatasetSize > 0 {
			q = bson.M{"$expr": bson.M{"$gte": []interface{}{bson.M{"$size": "$messages"}, request.MinDatasetSize}}}
		} else {
			q = bson.M{"$expr": bson.M{"$eq": []interface{}{"$size", bson.M{"$size": "$messages"}}}}
		}
		q = bson.M{"$or": []interface{}{bson.M{"name": finish_stream_keyword}, q}}
	} else {
		q = nil
	}
	return q
}

func (db *Mongodb) getMaxIndex(request Request, returnIncompete bool) (max_id int, err error) {
	c := db.client.Database(request.DbName()).Collection(data_collection_name_prefix + request.Stream)
	q := maxIndexQuery(request, returnIncompete)

	opts := options.FindOne().SetSort(bson.M{"_id": -1}).SetReturnKey(true)
	var result ID
	err = c.FindOne(context.TODO(), q, opts).Decode(&result)
	if err == mongo.ErrNoDocuments {
		return 0, nil

	}

	return result.ID, err
}
func duplicateError(err error) bool {
	command_error, ok := err.(mongo.CommandError)
	if !ok {
		write_exception_error, ok1 := err.(mongo.WriteException)
		if !ok1 {
			return false
		}
		return strings.Contains(write_exception_error.Error(), "duplicate key")
	}
	return command_error.Name == "DuplicateKey"
}

func (db *Mongodb) setCounter(request Request, ind int) (err error) {
	update := bson.M{"$set": bson.M{pointer_field_name: ind}}
	opts := options.Update().SetUpsert(true)
	c := db.client.Database(request.DbName()).Collection(pointer_collection_name)
	q := bson.M{"_id": request.GroupId + "_" + request.Stream}
	_, err = c.UpdateOne(context.TODO(), q, update, opts)
	return
}

func (db *Mongodb) errorWhenCannotSetField(request Request, max_ind int) error {
	if res, err := db.getRecordFromDb(request, max_ind, max_ind); err == nil {
		if err2 := checkStreamFinished(request, max_ind, max_ind, res); err2 != nil {
			return err2
		}
	}
	return &DBError{utils.StatusNoData, encodeAnswer(max_ind, max_ind, "")}
}

func (db *Mongodb) changeField(request Request, change fieldChangeRequest, res interface{}) (err error) {
	var update bson.M
	if change.op == field_op_inc {
		update = bson.M{"$inc": bson.M{change.fieldName: 1}}
	} else if change.op == field_op_set {
		update = bson.M{"$set": bson.M{change.fieldName: change.val}}
	}

	opts := options.FindOneAndUpdate().SetUpsert(true).SetReturnDocument(options.After)
	q := bson.M{"_id": request.GroupId + "_" + request.Stream, change.fieldName: bson.M{"$lt": change.max_ind}}
	c := db.client.Database(request.DbName()).Collection(change.collectionName)

	err = c.FindOneAndUpdate(context.TODO(), q, update, opts).Decode(res)
	if err != nil {
		// duplicateerror can happen because we set Upsert=true to allow insert pointer when it is absent. But then it will try to insert
		// pointer in case query found nothing, also when pointer exists and equal to max_ind. Here we have to return NoData
		if err == mongo.ErrNoDocuments || duplicateError(err) {
			// try again without upsert - if the first error was due to missing pointer
			opts = options.FindOneAndUpdate().SetUpsert(false).SetReturnDocument(options.After)
			if err2 := c.FindOneAndUpdate(context.TODO(), q, update, opts).Decode(res); err2 == nil {
				return nil
			}
			return db.errorWhenCannotSetField(request, change.max_ind)
		}
		return &DBError{utils.StatusTransactionInterrupted, err.Error()}
	}

	return nil
}

func encodeAnswer(id, id_max int, next_stream string) string {
	var r = struct {
		Op          string `json:"op"`
		Id          int    `json:"id"`
		Id_max      int    `json:"id_max"`
		Next_stream string `json:"next_stream"`
	}{"get_record_by_id", id, id_max, next_stream}
	answer, _ := json.Marshal(&r)
	return string(answer)
}

func recordContainsPartialData(request Request, rec map[string]interface{}) bool {
	if !request.DatasetOp {
		return false
	}

	name, ok_name := rec["name"].(string)
	if ok_name && name == finish_stream_keyword {
		return false
	}
	imgs, ok1 := rec["messages"].(primitive.A)
	expectedSize, ok2 := utils.InterfaceToInt64(rec["size"])
	if !ok1 || !ok2 {
		return false
	}
	nMessages := len(imgs)
	if (request.MinDatasetSize == 0 && int64(nMessages) != expectedSize) || (request.MinDatasetSize == 0 && nMessages < request.MinDatasetSize) {
		return true
	}
	return false
}

func (db *Mongodb) getRecordFromDb(request Request, id, id_max int) (res map[string]interface{}, err error) {
	q := bson.M{"_id": id}
	c := db.client.Database(request.DbName()).Collection(data_collection_name_prefix + request.Stream)
	err = c.FindOne(context.TODO(), q, options.FindOne()).Decode(&res)
	if err != nil {
		answer := encodeAnswer(id, id_max, "")
		request.Logger().WithFields(map[string]interface{}{"id": id, "cause": err.Error()}).Debug("error getting record")
		return res, &DBError{utils.StatusNoData, answer}
	}
	return res, err
}

func (db *Mongodb) getRecordByIDRaw(request Request, id, id_max int) ([]byte, error) {
	res, err := db.getRecordFromDb(request, id, id_max)
	if err != nil {
		return nil, err
	}

	if err := checkStreamFinished(request, id, id_max, res); err != nil {
		return nil, err
	}

	request.Logger().WithFields(map[string]interface{}{"id": id}).Debug("got record from db")

	record, err := utils.MapToJson(&res)
	if err != nil {
		return nil, err
	}
	if recordContainsPartialData(request, res) {
		return nil, &DBError{utils.StatusPartialData, string(record)}
	} else {
		return record, nil
	}
}

func (db *Mongodb) getRawRecordWithSort(dbname string, collection_name string, sortField string, sortOrder int) (map[string]interface{}, error) {
	var res map[string]interface{}
	c := db.client.Database(dbname).Collection(data_collection_name_prefix + collection_name)
	opts := options.FindOne().SetSort(bson.M{sortField: sortOrder})
	var q bson.M = nil
	err := c.FindOne(context.TODO(), q, opts).Decode(&res)

	if err != nil {
		if err == mongo.ErrNoDocuments {
			return map[string]interface{}{}, nil
		}
		return nil, err
	}
	return res, nil
}

func (db *Mongodb) getLastRawRecord(dbname string, collection_name string) (map[string]interface{}, error) {
	return db.getRawRecordWithSort(dbname, collection_name, "_id", -1)
}

func (db *Mongodb) getEarliestRawRecord(dbname string, collection_name string) (map[string]interface{}, error) {
	return db.getRawRecordWithSort(dbname, collection_name, "timestamp", 1)
}

func (db *Mongodb) getRecordByID(request Request) ([]byte, error) {
	id, err := strconv.Atoi(request.ExtraParam)
	if err != nil {
		return nil, &DBError{utils.StatusWrongInput, err.Error()}
	}

	max_ind, err := db.getMaxIndex(request, true)
	if err != nil {
		return nil, err
	}

	return db.getRecordByIDRaw(request, id, max_ind)
}

func (db *Mongodb) negAckRecord(request Request) ([]byte, error) {
	input := struct {
		Id     int
		Params struct {
			DelayMs int
		}
	}{}

	err := json.Unmarshal([]byte(request.ExtraParam), &input)
	if err != nil {
		return nil, &DBError{utils.StatusWrongInput, err.Error()}
	}

	err = db.InsertRecordToInprocess(request.DbName(), inprocess_collection_name_prefix+request.Stream+"_"+request.GroupId, input.Id, input.Params.DelayMs, 1, true)
	return []byte(""), err
}

func (db *Mongodb) ackRecord(request Request) ([]byte, error) {
	var record ID
	err := json.Unmarshal([]byte(request.ExtraParam), &record)
	if err != nil {
		return nil, &DBError{utils.StatusWrongInput, err.Error()}
	}
	c := db.client.Database(request.DbName()).Collection(acks_collection_name_prefix + request.Stream + "_" + request.GroupId)
	_, err = c.InsertOne(context.Background(), &record)
	if err != nil {
		if duplicateError(err) {
			return nil, &DBError{utils.StatusWrongInput, "already acknowledged"}
		}
		return nil, err
	}

	c = db.client.Database(request.DbName()).Collection(inprocess_collection_name_prefix + request.Stream + "_" + request.GroupId)
	_, err_del := c.DeleteOne(context.Background(), bson.M{"_id": record.ID})
	if err_del != nil {
		return nil, &DBError{utils.StatusWrongInput, err.Error()}
	}

	return []byte(""), err
}

func (db *Mongodb) checkDatabaseOperationPrerequisites(request Request) error {
	if db.client == nil {
		return &DBError{utils.StatusServiceUnavailable, no_session_msg}
	}

	if len(request.DbName()) <= 1 || len(request.Stream) == 0 {
		return &DBError{utils.StatusWrongInput, "beamtime_id ans stream must be set"}
	}

	return nil
}

func (db *Mongodb) getCurrentPointer(request Request) (LocationPointer, int, error) {
	max_ind, err := db.getMaxIndex(request, true)
	if err != nil {
		return LocationPointer{}, 0, err
	}

	if max_ind == 0 {
		return LocationPointer{}, 0, &DBError{utils.StatusNoData, encodeAnswer(0, 0, "")}
	}

	var curPointer LocationPointer
	err = db.changeField(request, fieldChangeRequest{
		collectionName: pointer_collection_name,
		fieldName:      pointer_field_name,
		op:             field_op_inc,
		max_ind:        max_ind}, &curPointer)
	if err != nil {
		return LocationPointer{}, 0, err
	}

	return curPointer, max_ind, nil
}

func (db *Mongodb) getUnProcessedId(dbname string, collection_name string, delayMs int, nResendAttempts int, rlog log.Logger) (int, error) {
	var res InProcessingRecord
	opts := options.FindOneAndUpdate().SetUpsert(false).SetReturnDocument(options.After)
	tNow := time.Now().UnixNano()
	var update bson.M
	if nResendAttempts == 0 {
		update = bson.M{"$set": bson.M{"delayMs": tNow + int64(delayMs*1e6), "maxResendAttempts": math.MaxInt32}, "$inc": bson.M{"resendAttempts": 1}}
	} else {
		update = bson.M{"$set": bson.M{"delayMs": tNow + int64(delayMs*1e6), "maxResendAttempts": nResendAttempts}, "$inc": bson.M{"resendAttempts": 1}}
	}

	q := bson.M{"delayMs": bson.M{"$lte": tNow}, "$expr": bson.M{"$lt": []string{"$resendAttempts", "$maxResendAttempts"}}}
	c := db.client.Database(dbname).Collection(collection_name)
	err := c.FindOneAndUpdate(context.TODO(), q, update, opts).Decode(&res)
	if err != nil {
		if err == mongo.ErrNoDocuments {
			return 0, nil
		}
		return 0, err
	}

	rlog.WithFields(map[string]interface{}{"id": res.ID}).Debug("got unprocessed message")
	return res.ID, nil
}

func (db *Mongodb) InsertRecordToInprocess(db_name string, collection_name string, id int, delayMs int, nResendAttempts int, replaceIfExist bool) error {
	record := InProcessingRecord{
		id, nResendAttempts, 0, time.Now().UnixNano() + int64(delayMs*1e6),
	}

	c := db.client.Database(db_name).Collection(collection_name)
	_, err := c.InsertOne(context.TODO(), &record)
	if duplicateError(err) {
		if !replaceIfExist {
			return nil
		}
		_, err := c.ReplaceOne(context.TODO(), bson.M{"_id": id}, &record)
		return err
	}
	return err
}

func (db *Mongodb) InsertToInprocessIfNeeded(db_name string, collection_name string, id int, extra_param string) error {
	if len(extra_param) == 0 {
		return nil
	}
	delayMs, nResendAttempts, err := extractsTwoIntsFromString(extra_param)
	if err != nil {
		return err
	}

	return db.InsertRecordToInprocess(db_name, collection_name, id, delayMs, nResendAttempts, false)

}

func (db *Mongodb) getNextAndMaxIndexesFromInprocessed(request Request, ignoreTimeout bool) (int, int, error) {
	var record_ind, max_ind, delayMs, nResendAttempts int
	var err error
	if len(request.ExtraParam) != 0 {
		delayMs, nResendAttempts, err = extractsTwoIntsFromString(request.ExtraParam)
		if err != nil {
			return 0, 0, err
		}
	} else {
		nResendAttempts = -1
	}
	tNow := time.Now().Unix()
	dbSessionLock.Lock()
	t := db.lastReadFromInprocess[request.Stream+"_"+request.GroupId]
	dbSessionLock.Unlock()
	if (t <= tNow-int64(db.settings.ReadFromInprocessPeriod)) || ignoreTimeout {
		record_ind, err = db.getUnProcessedId(request.DbName(), inprocess_collection_name_prefix+request.Stream+"_"+request.GroupId, delayMs, nResendAttempts,
			request.Logger())
		if err != nil {
			request.Logger().WithFields(map[string]interface{}{"cause": err.Error()}).Debug("error getting unprocessed message")
			return 0, 0, err
		}
	}
	if record_ind != 0 {
		max_ind, err = db.getMaxIndex(request, true)
		if err != nil {
			return 0, 0, err
		}
	} else {
		dbSessionLock.Lock()
		db.lastReadFromInprocess[request.Stream+"_"+request.GroupId] = time.Now().Unix()
		dbSessionLock.Unlock()
	}

	return record_ind, max_ind, nil

}

func (db *Mongodb) getNextAndMaxIndexesFromCurPointer(request Request) (int, int, error) {
	curPointer, max_ind, err := db.getCurrentPointer(request)
	if err != nil {
		request.Logger().WithFields(map[string]interface{}{"cause": err.Error()}).Debug("error getting next pointer")
		return 0, 0, err
	}
	request.Logger().WithFields(map[string]interface{}{"id": curPointer.Value}).Debug("got next pointer")
	return curPointer.Value, max_ind, nil
}

func (db *Mongodb) getNextAndMaxIndexes(request Request) (int, int, error) {
	nextInd, maxInd, err := db.getNextAndMaxIndexesFromInprocessed(request, false)
	if err != nil {
		return 0, 0, err
	}

	if nextInd == 0 {
		nextInd, maxInd, err = db.getNextAndMaxIndexesFromCurPointer(request)
		if err_db, ok := err.(*DBError); ok && err_db.Code == utils.StatusNoData {
			var err_inproc error
			nextInd, maxInd, err_inproc = db.getNextAndMaxIndexesFromInprocessed(request, true)
			if err_inproc != nil {
				return 0, 0, err_inproc
			}
			if nextInd == 0 {
				return 0, 0, err
			}
		}
	}
	return nextInd, maxInd, nil
}

func ExtractMessageRecord(data map[string]interface{}) (MessageRecord, bool) {
	var r MessageRecord
	err := utils.MapToStruct(data, &r)
	if err != nil {
		return r, false
	}
	r.FinishedStream = (r.Name == finish_stream_keyword)
	if r.FinishedStream {
		var next_stream string
		next_stream, ok := r.Meta["next_stream"].(string)
		if !ok {
			next_stream = no_next_stream_keyword
		}
		r.NextStream = next_stream
	}
	return r, true
}

func (db *Mongodb) tryGetRecordFromInprocessed(request Request, originalerror error) ([]byte, error) {
	var err_inproc error
	nextInd, maxInd, err_inproc := db.getNextAndMaxIndexesFromInprocessed(request, true)
	if err_inproc != nil {
		return nil, err_inproc
	}
	if nextInd != 0 {
		return db.getRecordByIDRaw(request, nextInd, maxInd)
	} else {
		return nil, originalerror
	}
}

func checkStreamFinished(request Request, id, id_max int, data map[string]interface{}) error {
	if id != id_max {
		return nil
	}
	r, ok := ExtractMessageRecord(data)
	if !ok || !r.FinishedStream {
		return nil
	}
	request.Logger().WithFields(map[string]interface{}{"nextStream": r.NextStream}).Debug("reached end of stream")

	answer := encodeAnswer(r.ID-1, r.ID-1, r.NextStream)
	return &DBError{utils.StatusNoData, answer}
}

func (db *Mongodb) getNextRecord(request Request) ([]byte, error) {
	nextInd, maxInd, err := db.getNextAndMaxIndexes(request)
	if err != nil {
		return nil, err
	}

	data, err := db.getRecordByIDRaw(request, nextInd, maxInd)
	if err != nil {
		data, err = db.tryGetRecordFromInprocessed(request, err)
	}

	if err == nil {
		err_update := db.InsertToInprocessIfNeeded(request.DbName(), inprocess_collection_name_prefix+request.Stream+"_"+request.GroupId, nextInd, request.ExtraParam)
		if err_update != nil {
			return nil, err_update
		}
	}
	return data, err
}

func (db *Mongodb) getLastRecord(request Request) ([]byte, error) {
	max_ind, err := db.getMaxIndex(request, false)
	if err != nil {
		return nil, err
	}
	return db.getRecordByIDRaw(request, max_ind, max_ind)
}

func (db *Mongodb) getLastRecordInGroup(request Request) ([]byte, error) {
	max_ind, err := db.getMaxIndex(request, false)
	if err != nil {
		return nil, err
	}

	var res map[string]interface{}
	err = db.changeField(request, fieldChangeRequest{
		collectionName: last_message_collection_name,
		fieldName:      last_message_field_name,
		op:             field_op_set,
		max_ind:        max_ind,
		val:            max_ind,
	}, &res)
	if err != nil {
		return nil, err
	}
	return db.getRecordByIDRaw(request, max_ind, max_ind)
}

func getSizeFilter(request Request) bson.M {
	filter := bson.M{}
	if request.ExtraParam == "false" { // do not return incomplete datasets
		filter = bson.M{"$expr": bson.M{"$eq": []interface{}{"$size", bson.M{"$size": "$messages"}}}}
	} else if request.ExtraParam == "true" {
		filter = bson.M{"$expr": bson.M{"$gt": []interface{}{bson.M{"$size": "$messages"}, 0}}}
	}
	filter = bson.M{"$and": []interface{}{bson.M{"name": bson.M{"$ne": finish_stream_keyword}}, filter}}
	return filter
}

func (db *Mongodb) getSize(request Request) ([]byte, error) {
	c := db.client.Database(request.DbName()).Collection(data_collection_name_prefix + request.Stream)

	filter := getSizeFilter(request)
	size, err := c.CountDocuments(context.TODO(), filter, options.Count())
	if err != nil {
		if ce, ok := err.(mongo.CommandError); ok && ce.Code == 17124 {
			return nil, &DBError{utils.StatusWrongInput, "no datasets found"}
		}
		return nil, err
	}

	var rec SizeRecord
	rec.Size = int(size)
	return json.Marshal(&rec)
}

func (db *Mongodb) resetCounter(request Request) ([]byte, error) {
	id, err := strconv.Atoi(request.ExtraParam)
	if err != nil {
		return nil, err
	}

	err = db.setCounter(request, id)
	if err != nil {
		return []byte(""), err
	}

	c := db.client.Database(request.DbName()).Collection(inprocess_collection_name_prefix + request.Stream + "_" + request.GroupId)
	_, err_del := c.DeleteMany(context.Background(), bson.M{"_id": bson.M{"$gte": id}})
	if err_del != nil {
		return nil, &DBError{utils.StatusWrongInput, err_del.Error()}
	}

	return []byte(""), nil
}

func getMetaId(request Request) (string, error) {
	switch request.ExtraParam {
	case "0":
		return "bt", nil
	case "1":
		return "st_" + request.Stream, nil
	default:
		return "", &DBError{utils.StatusWrongInput, "wrong meta type"}
	}
}

func (db *Mongodb) getMeta(request Request) ([]byte, error) {
	id, err := getMetaId(request)
	if err != nil {
		return nil, err
	}
	q := bson.M{"_id": id}
	var res map[string]interface{}
	c := db.client.Database(request.DbName()).Collection(meta_collection_name)
	err = c.FindOne(context.TODO(), q, options.FindOne()).Decode(&res)
	if err != nil {
		request.Logger().WithFields(map[string]interface{}{"id": id, "cause": err.Error()}).Debug("error getting meta")
		return nil, &DBError{utils.StatusNoData, err.Error()}
	}
	userMeta, ok := res["meta"]
	if !ok {
		request.Logger().WithFields(map[string]interface{}{"id": id, "cause": "cannot parse database response"}).Debug("error getting meta")
		return nil, errors.New("cannot get metadata")
	}
	request.Logger().WithFields(map[string]interface{}{"id": id}).Error("got metadata")
	return utils.MapToJson(&userMeta)
}

func (db *Mongodb) processQueryError(query, dbname string, err error, rlog log.Logger) ([]byte, error) {
	rlog.WithFields(map[string]interface{}{"query": query, "cause": err.Error()}).Debug("error processing query")
	return nil, &DBError{utils.StatusNoData, err.Error()}
}

func (db *Mongodb) queryMessages(request Request) ([]byte, error) {
	var res []map[string]interface{}
	q, sort, err := db.BSONFromSQL(request.DbName(), request.ExtraParam)
	if err != nil {
		request.Logger().WithFields(map[string]interface{}{"query": request.ExtraParam, "cause": err.Error()}).Debug("error parsing query")
		return nil, &DBError{utils.StatusWrongInput, err.Error()}
	}

	c := db.client.Database(request.DbName()).Collection(data_collection_name_prefix + request.Stream)
	opts := options.Find()

	if len(sort) > 0 {
		opts = opts.SetSort(sort)
	} else {
	}

	cursor, err := c.Find(context.TODO(), q, opts)
	if err != nil {
		return db.processQueryError(request.ExtraParam, request.DbName(), err, request.Logger())
	}
	err = cursor.All(context.TODO(), &res)
	if err != nil {
		return db.processQueryError(request.ExtraParam, request.DbName(), err, request.Logger())
	}

	request.Logger().WithFields(map[string]interface{}{"query": request.ExtraParam, "recordsFound": len(res)}).Debug("processed query")

	if res != nil {
		return utils.MapToJson(&res)
	} else {
		return []byte("[]"), nil
	}
}

func makeRange(min, max int) []int {
	a := make([]int, max-min+1)
	for i := range a {
		a[i] = min + i
	}
	return a
}

func extractsTwoIntsFromString(from_to string) (int, int, error) {
	s := strings.Split(from_to, "_")
	if len(s) != 2 {
		return 0, 0, errors.New("wrong format: " + from_to)
	}
	from, err := strconv.Atoi(s[0])
	if err != nil {
		return 0, 0, err
	}

	to, err := strconv.Atoi(s[1])
	if err != nil {
		return 0, 0, err
	}

	return from, to, nil

}

func (db *Mongodb) getNacksLimits(request Request) (int, int, error) {
	from, to, err := extractsTwoIntsFromString(request.ExtraParam)
	if err != nil {
		return 0, 0, err
	}

	if from == 0 {
		from = 1
	}

	if to == 0 {
		to, err = db.getMaxLimitWithoutEndOfStream(request, err)
		if err != nil {
			return 0, 0, err
		}
	}
	return from, to, nil
}

func (db *Mongodb) getMaxLimitWithoutEndOfStream(request Request, err error) (int, error) {
	maxInd, err := db.getMaxIndex(request, true)
	if err != nil {
		return 0, err
	}
	_, last_err := db.getRecordByIDRaw(request, maxInd, maxInd)
	if last_err != nil && maxInd > 0 {
		maxInd = maxInd - 1
	}
	return maxInd, nil
}

func (db *Mongodb) nacks(request Request) ([]byte, error) {
	from, to, err := db.getNacksLimits(request)
	if err != nil {
		return nil, err
	}

	res := Nacks{[]int{}}
	if to == 0 {
		return utils.MapToJson(&res)
	}

	res.Unacknowledged, err = db.getNacks(request, from, to)
	if err != nil {
		return nil, err
	}

	return utils.MapToJson(&res)
}

func (db *Mongodb) deleteCollection(request Request, name string) error {
	return db.client.Database(request.DbName()).Collection(name).Drop(context.Background())
}

func (db *Mongodb) collectionExist(request Request, name string) (bool, error) {
	result, err := db.client.Database(request.DbName()).ListCollectionNames(context.TODO(), bson.M{"name": name})
	if err != nil {
		return false, err
	}
	if len(result) == 1 {
		return true, nil
	}
	return false, nil
}

func (db *Mongodb) deleteDataCollection(errorOnNotexist bool, request Request) error {
	dataCol := data_collection_name_prefix + request.Stream
	if errorOnNotexist {
		exist, err := db.collectionExist(request, dataCol)
		if err != nil {
			return err
		}
		if !exist {
			return &DBError{utils.StatusWrongInput, "stream " + request.Stream + " does not exist"}
		}
	}
	return db.deleteCollection(request, dataCol)
}

func (db *Mongodb) deleteDocumentsInCollection(request Request, collection string, field string, pattern string) error {
	filter := bson.M{field: bson.D{{"$regex", primitive.Regex{Pattern: pattern, Options: "i"}}}}
	_, err := db.client.Database(request.DbName()).Collection(collection).DeleteMany(context.TODO(), filter)
	return err
}

func escapeQuery(query string) (res string) {
	chars := `\-[]{}()*+?.,^$|#`
	for _, char := range chars {
		query = strings.ReplaceAll(query, string(char), `\`+string(char))
	}
	return query
}

func (db *Mongodb) deleteCollectionsWithPrefix(request Request, prefix string) error {
	cols, err := db.client.Database(request.DbName()).ListCollectionNames(context.TODO(), bson.M{"name": bson.D{
		{"$regex", primitive.Regex{Pattern: "^" + escapeQuery(prefix), Options: "i"}}}})
	if err != nil {
		return err
	}

	for _, col := range cols {
		err := db.deleteCollection(request, col)
		if err != nil {
			return err
		}
	}

	return nil
}

func (db *Mongodb) deleteServiceMeta(request Request) error {
	err := db.deleteCollectionsWithPrefix(request, acks_collection_name_prefix+request.Stream)
	if err != nil {
		return err
	}
	err = db.deleteCollectionsWithPrefix(request, inprocess_collection_name_prefix+request.Stream)
	if err != nil {
		return err
	}
	return db.deleteDocumentsInCollection(request, pointer_collection_name, "_id", ".*_"+escapeQuery(request.Stream)+"$")
}

func (db *Mongodb) deleteStream(request Request) ([]byte, error) {
	params := struct {
		ErrorOnNotExist *bool
		DeleteMeta      *bool
	}{}

	err := json.Unmarshal([]byte(request.ExtraParam), &params)
	if err != nil {
		return nil, err
	}

	if params.DeleteMeta == nil || params.ErrorOnNotExist == nil {
		return nil, &DBError{utils.StatusWrongInput, "wrong params: " + request.ExtraParam}
	}
	if !*params.DeleteMeta {
		request.Logger().Debug("skipping delete stream meta")
		return nil, nil
	}

	err = db.deleteDataCollection(*params.ErrorOnNotExist, request)
	if err != nil {
		return nil, err
	}

	err = db.deleteServiceMeta(request)
	return nil, err
}

func (db *Mongodb) lastAck(request Request) ([]byte, error) {
	c := db.client.Database(request.DbName()).Collection(acks_collection_name_prefix + request.Stream + "_" + request.GroupId)
	opts := options.FindOne().SetSort(bson.M{"_id": -1}).SetReturnKey(true)
	result := LastAck{0}
	var q bson.M = nil
	err := c.FindOne(context.TODO(), q, opts).Decode(&result)
	if err == mongo.ErrNoDocuments {
		return utils.MapToJson(&result)
	}
	if err != nil {
		return nil, err
	}

	return utils.MapToJson(&result)
}

func (db *Mongodb) canAvoidDbRequest(min_index int, max_index int, c *mongo.Collection) ([]int, error, bool) {
	if min_index > max_index {
		return []int{}, errors.New("from index is greater than to index"), true
	}

	size, err := c.CountDocuments(context.TODO(), bson.M{}, options.Count())
	if err != nil {
		return []int{}, err, true
	}

	if size == 0 {
		return makeRange(min_index, max_index), nil, true
	}

	if min_index == 1 && int(size) == max_index {
		return []int{}, nil, true
	}

	return nil, nil, false
}

func getNacksQuery(max_index int, min_index int) []bson.D {
	matchStage := bson.D{{"$match", bson.D{{"_id", bson.D{{"$lt", max_index + 1}, {"$gt", min_index - 1}}}}}}
	groupStage := bson.D{
		{"$group", bson.D{
			{"_id", 0},
			{"numbers", bson.D{
				{"$push", "$_id"},
			}}},
		}}
	projectStage := bson.D{
		{"$project", bson.D{
			{"_id", 0},
			{"numbers", bson.D{
				{"$setDifference", bson.A{bson.D{{"$range", bson.A{min_index, max_index + 1}}}, "$numbers"}},
			}}},
		}}
	return mongo.Pipeline{matchStage, groupStage, projectStage}
}

func extractNacsFromCursor(err error, cursor *mongo.Cursor) ([]int, error) {
	resp := []struct {
		Numbers []int
	}{}
	err = cursor.All(context.Background(), &resp)
	if err != nil || len(resp) != 1 {
		return []int{}, err
	}
	return resp[0].Numbers, nil
}

func (db *Mongodb) getNacks(request Request, min_index, max_index int) ([]int, error) {
	c := db.client.Database(request.DbName()).Collection(acks_collection_name_prefix + request.Stream + "_" + request.GroupId)

	if res, err, ok := db.canAvoidDbRequest(min_index, max_index, c); ok {
		return res, err
	}

	query := getNacksQuery(max_index, min_index)
	cursor, err := c.Aggregate(context.Background(), query)

	return extractNacsFromCursor(err, cursor)
}

func (db *Mongodb) getStreams(request Request) ([]byte, error) {
	rec, err := streams.getStreams(db, request)
	if err != nil {
		return db.processQueryError("get streams", request.DbName(), err, request.Logger())
	}
	return json.Marshal(&rec)
}

func (db *Mongodb) ProcessRequest(request Request) (answer []byte, err error) {
	dbClientLock.RLock()
	defer dbClientLock.RUnlock()

	if err := db.checkDatabaseOperationPrerequisites(request); err != nil {
		return nil, err
	}

	if err := encodeRequest(&request); err != nil {
		return nil, err
	}

	switch request.Op {
	case "next":
		return db.getNextRecord(request)
	case "id":
		return db.getRecordByID(request)
	case "last":
		return db.getLastRecord(request)
	case "groupedlast":
		return db.getLastRecordInGroup(request)
	case "resetcounter":
		return db.resetCounter(request)
	case "size":
		return db.getSize(request)
	case "meta":
		return db.getMeta(request)
	case "querymessages":
		return db.queryMessages(request)
	case "streams":
		return db.getStreams(request)
	case "ackmessage":
		return db.ackRecord(request)
	case "negackmessage":
		return db.negAckRecord(request)
	case "nacks":
		return db.nacks(request)
	case "lastack":
		return db.lastAck(request)
	case "delete_stream":
		return db.deleteStream(request)
	}

	return nil, errors.New("Wrong db operation: " + request.Op)
}
