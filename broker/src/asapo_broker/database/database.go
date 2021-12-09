package database

import (
	"asapo_common/logger"
	"asapo_common/utils"
)

type Request struct {
	Beamtime       string
	DataSource     string
	Stream         string
	GroupId        string
	Op             string
	DatasetOp      bool
	MinDatasetSize int
	ExtraParam     string
}

func (request *Request) Logger() logger.Logger {
	return logger.WithFields(map[string]interface{}{
		"beamtime":   request.Beamtime,
		"dataSource": decodeString(request.DataSource),
		"stream":     decodeString(request.Stream),
		"groupId":    decodeString(request.GroupId),
		"operation":  request.Op,
	})
}

func (request *Request) DbName() string {
	return request.Beamtime + "_" + request.DataSource
}

type Agent interface {
	ProcessRequest(request Request) ([]byte, error)
	Ping() error
	Connect(string) error
	Close()
	SetSettings(settings DBSettings)
}

type DBSettings struct {
	ReadFromInprocessPeriod   int
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
