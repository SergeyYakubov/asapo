package database

type Agent interface {
	ProcessRequest(db_name string, data_collection_name string, group_id string, op string, extra string) ([]byte, error)
	Ping() error
	Connect(string) error
	Close()
	SetSettings(settings DBSettings)
}

type DBSettings struct {
	ReadFromInprocessPeriod int
	UpdateSubstreamCachePeriodMs int
}

type DBError struct {
	Code    int
	Message string
}

func (err *DBError) Error() string {
	return err.Message
}
