package main

import (
	"context"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
	"go.mongodb.org/mongo-driver/mongo/writeconcern"
	"go.mongodb.org/mongo-driver/x/bsonx"
	"log"
	"os"
	"strconv"
	"time"
)

var client *mongo.Client

var address = "localhost:27017"

func connectDb() error {
	opts := options.Client().SetConnectTimeout(20 * time.Second).
		ApplyURI("mongodb://" + address).SetWriteConcern(writeconcern.New(writeconcern.J(false)))
	var err error
	client, err = mongo.NewClient(opts)
	if err != nil {
		return err
	}
	ctx, cancel := context.WithTimeout(context.Background(), 20*time.Second)
	defer cancel()
	err = client.Connect(ctx)
	if err != nil {
		client = nil
		return err
	}
	return nil
}

type MessageRecord struct {
	ID             int                    `bson:"id" json:"id"`
	Timestamp      int                    `bson:"timestamp" json:"timestamp"`
	Name           string                 `bson:"name" json:"name"`
	Meta           map[string]interface{} `json:"meta"`
	NextStream     string
	FinishedStream bool
}

func getRecord(id int) *MessageRecord {
	var rec MessageRecord
	rec.ID = id
	rec.Name = "rec_" + strconv.Itoa(id)
	rec.NextStream = "ns"
	rec.FinishedStream = true
	return &rec
}


func main() {
	if err := connectDb(); err != nil {
		log.Fatal(err)
	}
	ctx := context.Background()

	dbName := "test"
	client.Database(dbName).Drop(ctx)

	nRecords, _ := strconv.Atoi(os.Args[1])
	nCollections, _ := strconv.Atoi(os.Args[2])

	keysDoc := bsonx.Doc{}
	keysDoc = keysDoc.Append("name", bsonx.Int32(1)).
		Append("id", bsonx.Int32(1))

	mod := mongo.IndexModel{
		Keys: keysDoc, Options: nil,
	}

	_, err := client.Database(dbName).Collection("data").Indexes().CreateOne(ctx, mod)
	if err != nil {
		log.Fatal(err)
	}

	start := time.Now()
	for col := 0; col < nCollections; col++ {
		collection := "col" + strconv.Itoa(col+1)
		for i := 0; i < nRecords/nCollections; i++ {
			rec := getRecord(i+1)
			rec.Name = collection
			_, err := client.Database(dbName).Collection("data").InsertOne(ctx, rec)
			if err != nil {
				log.Fatal(err)
			}
		}
	}
	elapsed := time.Since(start)
	log.Printf("write %d records/sec", int(float64(nRecords)/elapsed.Seconds()))

}
