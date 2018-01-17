package database

type Agent interface {
	GetNextRecord(db_name string, collection_name string) (answer []byte, ok bool)
	Connect(string) error
	Close()
}
