package database

type Agent interface {
	GetRecordFromDb(db_name string, group_id string, op string, id int) ([]byte, error)
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
