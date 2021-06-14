package database

import "asapo_common/utils"

type Request struct {
	DbName         string
	Stream         string
	GroupId        string
	Op             string
	DatasetOp      bool
	MinDatasetSize int
	ExtraParam     string
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
	UpdateStreamCachePeriodMs int
}

type DBError struct {
	Code    int
	Message string
}

func (err *DBError) Error() string {
	return err.Message
}

func GetStatusCodeFromError(err error) int {
	err_db, ok := err.(*DBError)
	if ok {
		return err_db.Code
	} else {
		return utils.StatusServiceUnavailable
	}
}

