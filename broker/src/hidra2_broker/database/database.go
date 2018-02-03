package database

type Agent interface {
	GetNextRecord(db_name string) ([]byte, error)
	Connect(string) error
	Close()
	Copy() Agent
}

type DBError struct {
	Code    int
	Message string
}

func (err *DBError) Error() string {
	return err.Message
}
