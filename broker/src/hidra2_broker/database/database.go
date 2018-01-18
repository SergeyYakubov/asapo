package database

type Agent interface {
	GetNextRecord(db_name string) (answer []byte, code int)
	Connect(string) error
	Close()
}
