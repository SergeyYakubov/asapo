package database

type Request struct {
	DbName string
	DbCollectionName string
	GroupId string
	Op string
	DatasetOp bool
	MinDatasetSize int
	ExtraParam string
}

type Agent interface {
	ProcessRequest(request Request) ([]byte, error)
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
