package database

type Agent interface {
	ProcessRequest(db_name string, group_id string, op string, extra string) ([]byte, error)
	Ping() error
	Connect(string) error
	Close()
}

type DBError struct {
	Code    int
	Message string
}

func (err *DBError) Error() string {
	return err.Message
}
